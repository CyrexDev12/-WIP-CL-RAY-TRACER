"""Generate a validated ray-tracer scene JSON file with the OpenAI API."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import os
from pathlib import Path
import re
import sys
from time import perf_counter
from typing import Any, Callable

from pydantic import ValidationError

try:
    from .scene_prompt import SYSTEM_PROMPT
    from .scene_schema import Scene
except ImportError:
    from scene_prompt import SYSTEM_PROMPT
    from scene_schema import Scene


QUALITY_LONG_EDGES = {
    "preview": 200,
    "standard": 400,
    "high": 800,
    "ultra": 1600,
}

DEFAULT_MODEL = "gpt-5.4-mini"
DEFAULT_API_TIMEOUT = 180.0


@dataclass(frozen=True)
class SceneGenerationResult:
    """Result returned by the reusable scene-generation API."""

    scene: Scene
    output: Path
    elapsed_seconds: float
    applied_quality: str | None

EXPLICIT_RESOLUTION = re.compile(
    r"\b(\d{2,4})\s*(?:x|by)\s*(\d{2,4})\b", re.I
)
ULTRA_QUALITY = re.compile(
    r"\b(?:ultra(?:[- ]high)?[- ]quality|maximum[- ]quality|ultra[- ]resolution)\b",
    re.I,
)
HIGH_QUALITY = re.compile(
    r"\b(?:high[- ]quality|high[- ]resolution|final[- ]quality|"
    r"production[- ]quality|final[- ]render)\b",
    re.I,
)


def apply_quality_preset(
    scene: Scene, description: str, requested_quality: str = "auto"
) -> str | None:
    quality = requested_quality
    if quality == "auto":
        explicit_resolution = EXPLICIT_RESOLUTION.search(description)
        if explicit_resolution:
            width, height = map(int, explicit_resolution.groups())
            if width > 4096 or height > 4096:
                raise ValueError("explicit resolution must not exceed 4096x4096")
            scene.image.width = width
            scene.image.height = height
            scene.camera.hsize = width
            scene.camera.vsize = height
            return "custom"
        if ULTRA_QUALITY.search(description):
            quality = "ultra"
        elif HIGH_QUALITY.search(description):
            quality = "high"
        else:
            return None

    target_long_edge = QUALITY_LONG_EDGES[quality]
    width = scene.image.width
    height = scene.image.height
    current_long_edge = max(width, height)

    if requested_quality == "auto" and current_long_edge >= target_long_edge:
        return quality

    if width >= height:
        new_width = target_long_edge
        new_height = max(1, round(target_long_edge * height / width))
    else:
        new_height = target_long_edge
        new_width = max(1, round(target_long_edge * width / height))

    scene.image.width = new_width
    scene.image.height = new_height
    scene.camera.hsize = new_width
    scene.camera.vsize = new_height
    return quality


def build_request_options(
    model: str,
    description: str,
    *,
    strict_schema: bool = False,
    reasoning_effort: str = "auto",
) -> dict[str, Any]:
    options: dict[str, Any] = {
        "model": model,
        "input": [
            {"role": "system", "content": SYSTEM_PROMPT},
            {"role": "user", "content": description},
        ],
    }
    if strict_schema:
        options["text_format"] = Scene
    else:
        options["text"] = {"format": {"type": "json_object"}}

    if model.startswith("gpt-5.4-"):
        if reasoning_effort == "auto":
            reasoning_effort = "low" if "mini" in model else "none"
        options["reasoning"] = {"effort": reasoning_effort}
        options.setdefault("text", {})["verbosity"] = (
            "medium" if "mini" in model else "low"
        )
    return options


def generate_scene_file(
    description: str,
    output: Path,
    *,
    api_key: str | None = None,
    model: str = DEFAULT_MODEL,
    reasoning_effort: str = "auto",
    quality: str = "auto",
    timeout: float = DEFAULT_API_TIMEOUT,
    strict_schema: bool = False,
    force: bool = False,
    multithreaded: bool | None = None,
    image_file: str | None = None,
    progress_callback: Callable[[str], None] | None = None,
) -> SceneGenerationResult:
    """Generate, validate, and save a scene for CLI or GUI callers."""

    description = description.strip()
    output = Path(output)
    if not description:
        raise ValueError("the scene description must not be empty")
    if output.suffix.lower() != ".json":
        raise ValueError("output must end in .json")
    if output.exists() and not force:
        raise FileExistsError(f"{output} already exists; pass force=True to replace it")
    if timeout <= 0:
        raise ValueError("timeout must be greater than zero")
    if quality not in ("auto", *QUALITY_LONG_EDGES):
        raise ValueError(f"unsupported quality preset: {quality}")
    if reasoning_effort not in ("auto", "none", "low", "medium", "high", "xhigh"):
        raise ValueError(f"unsupported reasoning effort: {reasoning_effort}")

    resolved_api_key = api_key or os.environ.get("OPENAI_API_KEY")
    if not resolved_api_key:
        raise ValueError("OPENAI_API_KEY is not set")

    notify = progress_callback or (lambda _message: None)
    started_at = perf_counter()
    mode = "strict schema" if strict_schema else "fast JSON"
    notify(f"Generating scene with {model} ({mode} mode)...")

    from openai import OpenAI

    client = OpenAI(api_key=resolved_api_key, timeout=timeout, max_retries=0)
    request_options = build_request_options(
        model,
        description,
        strict_schema=strict_schema,
        reasoning_effort=reasoning_effort,
    )
    if strict_schema:
        response = client.responses.parse(**request_options)
        scene = response.output_parsed
        if scene is None:
            raise RuntimeError("the model returned no parsed scene")
    else:
        response = client.responses.create(**request_options)
        scene = Scene.model_validate_json(response.output_text)

    notify("Validating generated scene...")
    applied_quality = apply_quality_preset(scene, description, quality)
    if multithreaded is not None:
        scene.image.multithreaded = multithreaded
    if image_file is not None:
        scene.image.file = image_file

    # Validate once more after deterministic UI/CLI overrides.
    scene = Scene.model_validate(scene.model_dump(by_alias=True))
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        scene.model_dump_json(by_alias=True, exclude_none=True, indent=2) + "\n",
        encoding="utf-8",
    )
    elapsed = perf_counter() - started_at
    notify(f"Created validated scene in {elapsed:.1f}s")
    return SceneGenerationResult(scene, output, elapsed, applied_quality)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Turn a natural-language description into a ray-tracer scene."
    )
    parser.add_argument("description", nargs="+", help="Scene to create")
    parser.add_argument(
        "-o", "--output", type=Path, required=True, help="Output .json path"
    )
    parser.add_argument(
        "--model",
        default=os.environ.get("OPENAI_SCENE_MODEL", DEFAULT_MODEL),
        help="OpenAI model (default: gpt-5.4-mini)",
    )
    parser.add_argument(
        "--reasoning-effort",
        choices=("auto", "none", "low", "medium", "high", "xhigh"),
        default=os.environ.get("OPENAI_SCENE_REASONING", "auto"),
        help="reasoning effort for GPT-5.4 models (default: auto)",
    )
    parser.add_argument(
        "--quality",
        choices=("auto", "preview", "standard", "high", "ultra"),
        default="auto",
        help="output resolution preset inferred from the prompt by default",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=DEFAULT_API_TIMEOUT,
        help="maximum seconds to wait for the API request (default: 180)",
    )
    parser.add_argument(
        "--strict-schema",
        action="store_true",
        help="use slower server-side Structured Outputs validation",
    )
    parser.add_argument("--force", action="store_true", help="Overwrite output")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    output: Path = args.output

    started_at = perf_counter()
    try:
        description = " ".join(args.description)
        result = generate_scene_file(
            description,
            output,
            model=args.model,
            reasoning_effort=args.reasoning_effort,
            quality=args.quality,
            timeout=args.timeout,
            strict_schema=args.strict_schema,
            force=args.force,
            progress_callback=lambda message: print(message, flush=True),
        )
    except (ValidationError, RuntimeError, ValueError, FileExistsError) as exc:
        elapsed = perf_counter() - started_at
        print(f"error: scene generation failed after {elapsed:.1f}s: {exc}", file=sys.stderr)
        return 1
    except Exception as exc:
        elapsed = perf_counter() - started_at
        print(f"error: OpenAI request failed after {elapsed:.1f}s: {exc}", file=sys.stderr)
        return 1

    if result.applied_quality is not None:
        print(
            f"Applied {result.applied_quality} quality resolution: "
            f"{result.scene.image.width}x{result.scene.image.height}"
        )
    print(f"Render with: ./raytracer.exe --scene {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
