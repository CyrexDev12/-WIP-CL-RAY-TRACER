"""Run the controlled six-prompt assessment and retain a complete evidence package."""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import shutil
import sys
from time import perf_counter

try:
    from .generate_scene import DEFAULT_MODEL, SOFTWARE_VERSION
    from .generation_service import GenerationEvent, GenerationRequest, GenerationService
    from .settings_store import get_api_key
except ImportError:
    from generate_scene import DEFAULT_MODEL, SOFTWARE_VERSION
    from generation_service import GenerationEvent, GenerationRequest, GenerationService
    from settings_store import get_api_key


BENCHMARK_PROMPTS: tuple[tuple[str, str, str], ...] = (
    (
        "One",
        "Glass, Chrome, and Reflections",
        "Create a dramatic studio still life on an infinite black-and-white checkered floor. Place a large transparent glass sphere in the center with high transparency and a realistic refractive index. Put a highly reflective dark chrome cube to its left and a polished gold closed cylinder to its right. Add two smaller colored spheres behind them. Use a low camera angle, strong perspective, one bright white point light above and to the left, deep shadows, crisp highlights, and a dark background. Keep the composition clean and cinematic.",
    ),
    (
        "Two",
        "Neon Planetary System",
        "Create a cinematic miniature solar system floating over a dark reflective plane. Place one large glowing orange sun slightly off center with strong emissive color and bloom. Surround it with five planets of different sizes and colors, including a blue glass planet, a striped gas giant, and a small reflective red moon. Arrange the planets at different depths rather than in a straight line. Use a wide three-quarter camera angle, a subtle point light near the sun, a nearly black environment, vivid rim highlights, and strong but controlled bloom. Remember that emission is visually self-lit but does not illuminate nearby objects, so the requested point light helps sell the effect.",
    ),
    (
        "Three",
        "Pattern Material Gallery",
        "Design a sophisticated material-gallery scene with five spheres displayed on individual matte pedestals above a neutral plane. Give the spheres these materials: black-and-white stripes, blue-to-gold gradient, red-and-cream rings, green-and-black checkers, and a perturbed purple-and-cyan pattern. Apply different scales and rotations to the patterns so their transformations are obvious. Use a centered gallery camera, one soft white point light above and to the left, balanced shadows, a charcoal background, and enough spacing that every sphere is clearly visible.",
    ),
    (
        "Four",
        "Geometric Sci-Fi Temple",
        "Create a symmetrical science-fiction temple built from grouped geometric shapes. Use two rows of tall closed cylinders as columns, stacked and rotated cubes for the central doorway, and large triangles forming a glowing emblem above it. Place everything on a slightly reflective dark plane. Use dark stone materials with subtle gold accents and a cyan emissive emblem with bloom. Frame the temple from a low, centered camera position looking slightly upward. Use one warm point light near the entrance and dramatic contrast while keeping the architecture readable.",
    ),
    (
        "Five",
        "Abstract Recursive Sculpture",
        "Create an abstract floating sculpture made from three nested groups. Each group should contain a central reflective sphere, two rotated cubes, and three thin triangular fins. Scale and rotate each group differently so the full structure twists upward like a spiral. Use silver, deep blue, and copper materials. Suspend it above a matte plane with a soft circular-looking shadow. Use a three-quarter camera angle, a single bright point light from above, a dark navy background, and subtle blue emissive accents with restrained bloom.",
    ),
    (
        "Six",
        "Surreal Checkerboard Landscape",
        "Create a surreal landscape on an infinite red-and-black checkered plane. Place three enormous glossy spheres receding toward the horizon, a transparent glass cube floating above the foreground, and several thin closed cylinders leaning at different angles like abstract towers. Add one small glowing cyan sphere as the focal point. Use an extremely low camera close to the ground, strong depth and scale, one pale point light high behind the camera, long shadows, reflective accents, and gentle bloom around the cyan sphere.",
    ),
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run the controlled render benchmark.")
    parser.add_argument("--iteration", type=int, default=2)
    parser.add_argument("--model", default=DEFAULT_MODEL)
    parser.add_argument("--timeout", type=float, default=180.0)
    parser.add_argument("--force", action="store_true")
    parser.add_argument(
        "--retry-failed",
        action="store_true",
        help="rerun only failed records in an existing iteration manifest",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.iteration < 1:
        print("error: --iteration must be positive", file=sys.stderr)
        return 2
    if args.timeout <= 0:
        print("error: --timeout must be positive", file=sys.stderr)
        return 2
    if args.force and args.retry_failed:
        print("error: --force and --retry-failed cannot be combined", file=sys.stderr)
        return 2
    try:
        api_key, key_source = get_api_key()
    except Exception as exc:
        print(f"error: could not access the API key: {exc}", file=sys.stderr)
        return 2
    if not api_key:
        print("error: configure an OpenAI API key in Settings before running the benchmark", file=sys.stderr)
        return 2

    root = Path(__file__).resolve().parent.parent
    evidence_dir = root / "TestDocs" / "Test Renders" / f"Iter{args.iteration}"
    manifest_path = evidence_dir / "manifest.json"
    planned = [evidence_dir / f"{file_stem}.png" for file_stem, _, _ in BENCHMARK_PROMPTS]
    if not args.retry_failed and not args.force and any(path.exists() for path in planned):
        print(
            f"error: benchmark evidence already exists in {evidence_dir}; pass --force to replace it",
            file=sys.stderr,
        )
        return 2
    evidence_dir.mkdir(parents=True, exist_ok=True)

    if args.retry_failed:
        if not manifest_path.is_file():
            print(f"error: no manifest exists at {manifest_path}", file=sys.stderr)
            return 2
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        settings = manifest.get("settings", {})
        expected = {
            "quality": "high",
            "long_edge_pixels": 800,
            "reasoning_effort": "medium",
            "multithreaded": True,
            "model": args.model,
            "timeout_seconds": args.timeout,
        }
        if any(settings.get(key) != value for key, value in expected.items()):
            print("error: retry settings do not match the existing manifest", file=sys.stderr)
            return 2
    else:
        manifest = {
            "iteration": args.iteration,
            "software_version": SOFTWARE_VERSION,
            "started_at_utc": datetime.now(timezone.utc).isoformat(),
            "settings": {
                "quality": "high",
                "long_edge_pixels": 800,
                "reasoning_effort": "medium",
                "multithreaded": True,
                "model": args.model,
                "timeout_seconds": args.timeout,
                "api_key_source": key_source,
            },
            "renders": [],
        }
    records: list[dict[str, object]] = manifest["renders"]  # type: ignore[assignment]
    records_by_name = {str(record.get("name")): record for record in records}
    selected_prompts = [
        item
        for item in BENCHMARK_PROMPTS
        if not args.retry_failed
        or records_by_name.get(item[1], {}).get("status") == "failed"
    ]
    if args.retry_failed and not selected_prompts:
        print("No failed benchmark records to retry.")
        return 0
    service = GenerationService(root)
    benchmark_started = perf_counter()

    for index, (file_stem, name, prompt) in enumerate(selected_prompts, 1):
        print(f"[{index}/{len(selected_prompts)}] {name}", flush=True)

        def report(event: GenerationEvent) -> None:
            print(f"  {event.stage}: {event.message}", flush=True)

        if args.retry_failed:
            record = records_by_name[name]
            retry_history = record.setdefault("retry_history", [])
            if isinstance(retry_history, list):
                retry_history.append(
                    {
                        "retried_at_utc": datetime.now(timezone.utc).isoformat(),
                        "previous_error": record.get("error"),
                    }
                )
            record.clear()
            record.update(
                {
                    "name": name,
                    "prompt": prompt,
                    "status": "running",
                    "retry_history": retry_history,
                }
            )
        else:
            record = {"name": name, "prompt": prompt, "status": "running"}
            records.append(record)
        manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
        try:
            result = service.run(
                GenerationRequest(
                    prompt=prompt,
                    api_key=api_key,
                    quality="high",
                    reasoning_effort="medium",
                    multithreaded=True,
                    model=args.model,
                    timeout=args.timeout,
                ),
                report,
            )
            png_path = evidence_dir / f"{file_stem}.png"
            scene_path = evidence_dir / f"{file_stem}.scene.json"
            audit_path = evidence_dir / f"{file_stem}.audit.json"
            shutil.copy2(result.png_path, png_path)
            shutil.copy2(result.scene_path, scene_path)
            if result.audit_path is not None and result.audit_path.is_file():
                shutil.copy2(result.audit_path, audit_path)
            record.update(
                {
                    "status": "complete",
                    "elapsed_seconds": result.elapsed_seconds,
                    "png": str(png_path.relative_to(root)),
                    "scene": str(scene_path.relative_to(root)),
                    "audit": str(audit_path.relative_to(root)) if audit_path.is_file() else None,
                }
            )
        except Exception as exc:
            record.update({"status": "failed", "error": str(exc)})
            print(f"  failed: {exc}", file=sys.stderr, flush=True)
        manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    completed_at = datetime.now(timezone.utc).isoformat()
    elapsed = perf_counter() - benchmark_started
    manifest["completed_at_utc"] = completed_at
    if args.retry_failed:
        manifest["last_retry_completed_at_utc"] = completed_at
        manifest["last_retry_elapsed_seconds"] = elapsed
    else:
        manifest["elapsed_seconds"] = elapsed
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    failures = sum(record.get("status") != "complete" for record in records)
    print(f"Benchmark evidence: {evidence_dir}")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
