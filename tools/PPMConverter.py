## Converts PPM files to PNG files using the Pillow library.
## RUN by `python tools/PPMConverter.py <input.ppm> <output.png>
## python tools/PPMConverter.py scene.ppm ./Renders/Test.png
## Output file goes to ../Renders

import sys
from PIL import Image

if len(sys.argv) != 3:
    print("Usage: python tools/PPMConverter.py <input.ppm> <output.png>")
    sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]

image = Image.open(input_file)
image.save(output_file, "PNG")

print(f"Converted {input_file} to {output_file}")