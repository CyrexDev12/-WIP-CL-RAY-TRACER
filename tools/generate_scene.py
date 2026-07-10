"""Generate a validated ray-tracer scene JSON file with the OpenAI API."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

from pydantic import ValidationError

from scene_prompt import SYSTEM_PROMPT
from scene_schema import Scene


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
        default=os.environ.get("OPENAI_SCENE_MODEL", "gpt-5.4-mini"),
        help="OpenAI model (default: gpt-5.4-mini)",
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
    if not os.environ.get("OPENAI_API_KEY"):
        print(
            "error: OPENAI_API_KEY is not set. Set it in your environment; "
            "do not put API keys in source files.",
            file=sys.stderr,
        )
        return 2

    try:
        from openai import OpenAI

        client = OpenAI()
        response = client.responses.parse(
            model=args.model,
            input=[
                {"role": "system", "content": SYSTEM_PROMPT},
                {"role": "user", "content": " ".join(args.description)},
            ],
            text_format=Scene,
        )
        scene = response.output_parsed
        if scene is None:
            raise RuntimeError("the model returned no parsed scene")
        # Validate once more before touching the filesystem.
        scene = Scene.model_validate(scene.model_dump(by_alias=True))
    except (ValidationError, RuntimeError, ValueError) as exc:
        print(f"error: scene generation failed: {exc}", file=sys.stderr)
        return 1
    except Exception as exc:
        print(f"error: OpenAI request failed: {exc}", file=sys.stderr)
        return 1

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(scene.model_dump_json(by_alias=True, indent=2) + "\n", encoding="utf-8")
    print(f"Created validated scene: {output}")
    print(f"Render with: ./raytracer.exe --scene {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
