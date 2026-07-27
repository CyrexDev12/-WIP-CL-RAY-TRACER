"""Convert renderer PPM output to PNG."""

from pathlib import Path
import sys


def convert_ppm_to_png(input_file: Path | str, output_file: Path | str) -> Path:
    """Convert one PPM file to PNG and return the resulting path."""

    from PIL import Image

    source = Path(input_file)
    destination = Path(output_file)
    if not source.is_file():
        raise FileNotFoundError(f"PPM input does not exist: {source}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    with Image.open(source) as image:
        image.save(destination, "PNG")
    return destination


def main() -> int:
    if len(sys.argv) != 3:
        print("Usage: python tools/PPMConverter.py <input.ppm> <output.png>")
        return 2

    try:
        output = convert_ppm_to_png(sys.argv[1], sys.argv[2])
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    print(f"Converted {sys.argv[1]} to {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
