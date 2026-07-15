"""Generate a validated ray-tracer scene JSON file with the OpenAI API."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
from time import perf_counter
from typing import Any

from pydantic import ValidationError

try:
    from .scene_prompt import SYSTEM_PROMPT
    from .scene_schema import Scene
except ImportError:
    from scene_prompt import SYSTEM_PROMPT
    from scene_schema import Scene


def build_request_options(
    model: str, description: str, *, strict_schema: bool = False
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
        options["reasoning"] = {"effort": "none"}
        options.setdefault("text", {})["verbosity"] = "low"
    return options


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
        default=os.environ.get("OPENAI_SCENE_MODEL", "gpt-5.4-nano"),
        help="OpenAI model (default: gpt-5.4-nano)",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=60.0,
        help="maximum seconds to wait for the API request (default: 60)",
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

    if output.suffix.lower() != ".json":
        print("error: --output must end in .json", file=sys.stderr)
        return 2
    if output.exists() and not args.force:
        print(f"error: {output} already exists; pass --force to replace it", file=sys.stderr)
        return 2
    if args.timeout <= 0:
        print("error: --timeout must be greater than zero", file=sys.stderr)
        return 2
    if not os.environ.get("OPENAI_API_KEY"):
        print(
            "error: OPENAI_API_KEY is not set. Set it in your environment; "
            "do not put API keys in source files.",
            file=sys.stderr,
        )
        return 2

    started_at = perf_counter()
    mode = "strict schema" if args.strict_schema else "fast JSON"
    print(f"Generating scene with {args.model} ({mode} mode)...", flush=True)

    try:
        from openai import OpenAI

        client = OpenAI(timeout=args.timeout, max_retries=0)
        request_options = build_request_options(
            args.model,
            " ".join(args.description),
            strict_schema=args.strict_schema,
        )
        if args.strict_schema:
            response = client.responses.parse(**request_options)
            scene = response.output_parsed
            if scene is None:
                raise RuntimeError("the model returned no parsed scene")
        else:
            response = client.responses.create(**request_options)
            scene = Scene.model_validate_json(response.output_text)
        # Validate once more before touching the filesystem.
        scene = Scene.model_validate(scene.model_dump(by_alias=True))
    except (ValidationError, RuntimeError, ValueError) as exc:
        elapsed = perf_counter() - started_at
        print(f"error: scene generation failed after {elapsed:.1f}s: {exc}", file=sys.stderr)
        return 1
    except Exception as exc:
        elapsed = perf_counter() - started_at
        print(f"error: OpenAI request failed after {elapsed:.1f}s: {exc}", file=sys.stderr)
        return 1

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        scene.model_dump_json(by_alias=True, exclude_none=True, indent=2) + "\n",
        encoding="utf-8",
    )
    elapsed = perf_counter() - started_at
    print(f"Created validated scene in {elapsed:.1f}s: {output}")
    print(f"Render with: ./raytracer.exe --scene {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
