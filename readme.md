Ray Tracer AI Prompt Scene Generator
<img width="1600" height="800" alt="image" src="https://github.com/user-attachments/assets/42437b10-24d4-4001-89ee-978d9f4e693f" />

------------------
The Python generator turns a natural-language description into scene JSON that the
C++ ray tracer can render. It uses the OpenAI Responses API and validates every
response against the renderer's actual limits before writing a file. Fast JSON mode
is the default; optional Structured Outputs mode performs server-side schema checks.

### 1. Build the ray tracer

From the project root:

```powershell
mingw32-make
```

Verify the renderer with the included example:

```powershell
.\raytracer.exe --scene scenes/example_scene.json
```

### 2. Set up Python

Create and activate a virtual environment, then install the dependencies:

```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install -r requirements.txt
```

Run the activation command again whenever you open a new PowerShell window.

### 3. Configure the OpenAI API key

Create an API key in your OpenAI Platform account, then set it in the current
PowerShell session:

```powershell
$env:OPENAI_API_KEY = "your-api-key"
```

Do not paste a real key into Python, JSON, `.env`, or any committed file. This
environment variable lasts only for the current PowerShell session.

Optionally select the default model for the session:

```powershell
$env:OPENAI_SCENE_MODEL = "gpt-5.4-mini"
```

### 4. Generate a scene

```powershell
python tools/generate_scene.py `
  "three glossy planets floating above a red ground" `
  --output scenes/planets.json
```

Descriptions can include composition, colors, materials, camera position, output
resolution/quality, glow, bloom, and multithreading. For example:

```powershell
python tools/generate_scene.py `
  "A glowing blue planet between two small red moons, viewed from slightly above. Use bloom and 300 by 150 resolution." `
  -o scenes/glowing_planet.json
```

Emissive materials use `emissiveColor` plus `emissiveStrength`. Ask for glow or
bloom in the description and the generator will also enable the image bloom fields.
Emission makes a surface self-lit but does not cast light onto nearby objects, so
the generator may use the single point light to suggest that illumination.

### Generated object types

AI-generated scenes can use spheres, infinite planes, cubes, finite cylinders,
triangles, and recursive groups. Object and group transforms support `scale`,
`rotate` (XYZ radians), and `translate`, in that order. For example:

```powershell
python tools/generate_scene.py `
  "A patterned floor with a glass sphere, a rotated gold cube, two closed marble columns, and a grouped triangular sculpture" `
  -o scenes/gallery.json
```

Use planes for floors and walls. Cylinder `minimum` and `maximum` values bound its
local Y axis, while `closed` adds end caps. Triangle points are local coordinates.
Groups contain one or more child objects and share a parent transform; groups do not
have their own material. Keep infinite planes at the top level because group bounds
are intended for finite objects.

### Generated material patterns

The AI generator and JSON loader support `stripe`, `checkers`, `gradient`, `ring`,
and recursive `perturbed` patterns. Ask for them directly in the description:

```powershell
python tools/generate_scene.py `
  "A ball with a red-to-blue gradient beside a ball with perturbed green and gold rings" `
  -o scenes/patterned_balls.json
```

Two-color patterns contain `colorA` and `colorB`. A perturbed pattern wraps another
pattern in `base` and can tune `distortionScale` and `noiseFrequency`. Pattern
transforms independently support `scale`, `rotate`, and `translate`. Generated JSON
uses the correct spelling `perturbed`; the loader also accepts the legacy spelling
`pertubed`.

### Quality presets

Choose a deterministic output size with `--quality`:

```powershell
python tools/generate_scene.py "A neon solar system with bloom" `
  -o scenes/neon_system.json `
  --quality high
```

The presets set the longest image edge while preserving the generated aspect ratio:

- `preview`: 200 pixels
- `standard`: 400 pixels
- `high`: 800 pixels
- `ultra`: 1600 pixels
- `auto` (default): honor an explicit `WIDTHxHEIGHT` or `WIDTH by HEIGHT` in the
  description, infer `high`/`ultra` from phrases such as "final render" or
  "maximum quality", otherwise keep the model's validated dimensions.

An explicit `--quality` option takes priority over a resolution written in the
description. Image and camera dimensions are always updated together.

Generator options:

- `-o` or `--output` is required and must point to a `.json` file.
- `--force` allows an existing JSON file to be replaced.
- `--model MODEL_NAME` overrides the default model for one request.
- `--quality {auto,preview,standard,high,ultra}` controls output resolution.
- `--reasoning-effort {auto,none,low,medium,high,xhigh}` controls GPT-5.4 reasoning.
- `--timeout SECONDS` changes the 60-second API timeout.
- `--strict-schema` enables server-side Structured Outputs instead of fast JSON mode.
- `OPENAI_SCENE_MODEL` changes the default model for the current shell.
- `OPENAI_SCENE_REASONING` changes the default reasoning effort for the current shell.
- The default model is `gpt-5.4-mini`.

Example using all relevant options:

```powershell
python tools/generate_scene.py "A simple solar system" `
  -o scenes/planets.json `
  --model gpt-5.4-mini `
  --quality ultra `
  --reasoning-effort low `
  --timeout 120 `
  --strict-schema `
  --force
```

Generating a scene makes an OpenAI API request and may incur API usage charges.

### 5. Inspect and render the generated JSON

Review the generated file before rendering, then run:

```powershell
.\raytracer.exe --scene scenes/planets.json
```

The rendered PPM is written to the filename in `image.file`. That filename is
relative to the directory from which `raytracer.exe` is run.

### Current generated-scene limits

The generator deliberately supports only features implemented by
`src/SceneLoader.cpp`:

- Exactly one point light.
- Between 1 and 100 top-level objects: sphere, plane, cube, finite cylinder,
  triangle, or recursive group.
- Object, group, and pattern scaling, XYZ rotation in radians, and translation.
- Finite cylinder bounds and optional closed end caps; non-collinear triangle points.
- RGB color components from `0` to `1`.
- Optional `emissiveColor` components from `0` to `1` and `emissiveStrength` from
  `0` to `20`. Emission is self-lighting and does not illuminate other objects.
- Optional bloom post-processing with intensity from `0` to `2`, threshold from
  `0` to `10`, and radius from `1` to `32`.
- Material patterns: `stripe`, `checkers`, `gradient`, `ring`, and recursively
  nested `perturbed`, each with independent transforms.
- `ambient`, `diffuse`, `specular`, `reflective`, and `transparency` from `0` to `1`.
- `shininess` from `10` to `200`, inclusive. Other values terminate the C++ renderer.
- `refractiveIndex` from `1` to `3`.
- A field of view greater than `0` and less than pi radians.
- Image dimensions from `1` to `4096`; `image.width/height` must match
  `camera.hsize/vsize`.
- A simple `.ppm` output filename without directory components.
- Non-zero scale values and valid camera vectors.
- Infinite planes should remain top-level rather than inside finite-bounds groups.

### Troubleshooting

`OPENAI_API_KEY is not set`
: Set `$env:OPENAI_API_KEY` in the same PowerShell window used to run Python.

`ModuleNotFoundError: openai` or `ModuleNotFoundError: pydantic`
: Activate `.venv`, then run `python -m pip install -r requirements.txt`.

`already exists; pass --force to replace it`
: Choose another output filename or add `--force` if replacement is intentional.

Scene validation failure
: The model attempted to use a value or structure outside the supported renderer
  schema. No JSON file is written. Refine the description or try again.

`Must be a value between 10-200!`
: A hand-edited or older scene contains an invalid `material.shininess`. Change it to
  a value from `10` through `200`. Newly generated scenes enforce this automatically.

API request timeout
: Increase the limit with `--timeout 120` (or another positive number), especially
  for strict schema mode or higher reasoning effort.

The schema and boundary checks live in `tools/scene_schema.py`. The renderer-specific
AI instructions live in `tools/scene_prompt.py`. Update both whenever the C++ scene
loader gains a feature or changes a numeric boundary.


This project is currently all entirely based off on the CPU. We plan to use CUDA using NVIDA technology. 


1. Finish CPU ray tracer.
Triangle, CSG

Add JSON file Intepreting 

JSON PARSER USING
nlohmann/json 

2. Add CPU multithreading.
    - Compare Times for more complex renders (with multi threading vs without)
3. Build CUDA mini-renderer with spheres and planes.
4. Compare render times.
5. Add reflection/refraction/Schlick to the GPU version.

-------------------------------------------------------------------------------------------------------------

- Tuple: A point like (-4, 4, 3)
(x,y,z,w)
For a point in 3D space, you typically use w = 1 → (x, y, z, 1)
For a direction/vector, you use w = 0 → (x, y, z, 0)

W = 1 -> Point 
W = 0 -> Vector 



Matrix Implementation 
Using 2D vector dynamic arrays

Matrix Class includes:
vector<vector<double>>
rows
cols

and all of the needed operations 

Matrix Transformations 

Translation, Scaling, Rotation, Shearing can all be represented as matricies 


Identifying Hits 
- When rendering a scene we will need to identify which one of the intersections is actually visible from the ray's origin. 
(Some can be behind the ray, and others may be hidden or behind by other objects)
- We call the visible intersection the 'hit' 
(The hit will never be behind the ray's origin) (Since it is effectivley behind the camera), so we can ignore all intersections with negative t values. The hit will always be the intersection with the lowest nonnegative t value. 


Moving the sphere 
When we increase the distance between the sphere and the ray, we can translate the ray away from the sphere, and it is indifferent from
translating the sphere away from the ray. 

Scaling 
- If we want to make our sphere bigger it is just the same as shrinking the distance between the ray and the sphere. 
- It is an inverse relationship. 
- We scale the ray by the inverse of how you were wanting to scale the sphere. 

Rotation 
- If we want to rotate the sphere, you rotate the sphere by the inverse of the rotation you wanted to apply to the sphere. 

To sum up transformations, whatever transformation we want to apply to the sphere, we apply the inverse to the ray. 


Lighting and Shading 

- We implement a model to simulate the reflection of light from a surface, which allows us to draw the sphere and make it look '3D' 

HOW TO: 
- Implement a source of light 
- Implement a shading algorithm to approximate how birhgtly that light illuminates the surfaces it shines on 

(Most ray tracers favor approximations over physically accurate simulations)

We define 4 different vectors 
If p is where your ray intersects an object, these four vectors are defined as: 
- E is the eye vector, pointing from p to the origin of the ray.
- L is the light vector, poiting from p to the position of the light source.
- N is the surface normal, a vector that is perpendicular to the surface at P. 
- R is the reflection vector, poiting in the direction that incoming light would bounce, or reflect. 

Phong Reflection Model 
Simulates interaction between three different types of lighting. 

- Ambient Reflection: Background lighting. Or light reflected from other objects in its environment. The Phong model treats this as a constant coloring all points on the surface equally. 
- Diffuse Reflection: Light reflected from a matte surface. It depends only on the angle between the light source and the surface normal. 
- Specular Reflection: Reflection of the light source itself and results in what is called a specular highlight. The bright spot on a curved surface. It depends only on the angle between the reflection vector and the eye vector and is controlled by a parameter that we cann shininess. The higher the shininess the smaller and tighter the specular highlight. 


The lighting Function (Located within the object class, e.g. sphere)
Expects 5 arguments material, point being illuminated, the light source, eye and normal vectors from the phong reflection model. 


Camera 

Map a 3-Dimensional scene onto a two dimensional canvas. 
Cameras Canvas will always be one unit away from the camera 



To Do: 

- Switch from using vector<double> to using a tuple class, so it is dealt with on the stack istead of heap to make the program faster. 

JSON Scenes
-----------
This project supports loading scenes from JSON files. The loader uses the header-only library `nlohmann/json`.

Install (examples):

Windows (vcpkg):
```
vcpkg install nlohmann-json
```

Ubuntu:
```
sudo apt install nlohmann-json3-dev
```

Usage:
- Run the executable with `--scene <path/to/scene.json>`
- An example scene lives at `scenes/example_scene.json`.

JSON Scene Format (Quick Reference)
----------------------------------
This project supports loading scenes described in JSON. The loader understands image output settings, a camera, one point light, materials, patterns, and recursive scene objects.

Top-level keys
- `image` (optional): `{ "width": int, "height": int, "file": string, "multithreaded": bool, "bloom": bool, "bloomIntensity": float, "bloomThreshold": float, "bloomRadius": int }` — output image settings.
- `camera` (optional): `{ "hsize": int, "vsize": int, "fov": float, "from": [x,y,z], "to": [x,y,z], "up": [x,y,z] }` — camera and view transform.
- `lights` (optional): an array of lights. Supported light object example:
    - `{ "type": "point", "position": [x,y,z], "color": [r,g,b] }`
- `objects` (required): one or more scene objects:
    - Sphere: `{ "type": "sphere", "transform": {...}, "material": {...} }`.
    - Plane: `{ "type": "plane", "transform": {...}, "material": {...} }`; local plane is infinite at `y=0`.
    - Cube: `{ "type": "cube", "transform": {...}, "material": {...} }`; local bounds are `-1` through `1`.
    - Cylinder: `{ "type": "cylinder", "minimum": -1, "maximum": 1, "closed": false, "transform": {...}, "material": {...} }`.
    - Triangle: `{ "type": "triangle", "p1": [x,y,z], "p2": [x,y,z], "p3": [x,y,z], "transform": {...}, "material": {...} }`.
    - Group: `{ "type": "group", "transform": {...}, "children": [ ...objects... ] }`; groups have no material.
    - Transform: `{ "scale": [sx,sy,sz], "rotate": [rx,ry,rz], "translate": [tx,ty,tz] }`; rotations use radians.
    - Material properties include `color`, `ambient`, `diffuse`, `specular`, `shininess`, `reflective`, `transparency`, `refractiveIndex`, `emissiveColor`, `emissiveStrength`, and `pattern`.
    - Two-color pattern: `{ "type": "gradient", "colorA": [r,g,b], "colorB": [r,g,b], "transform": {...} }`; types are `stripe`, `checkers`, `gradient`, or `ring`.
    - Perturbed pattern: `{ "type": "perturbed", "base": { ...another pattern... }, "distortionScale": number, "noiseFrequency": number, "transform": {...} }`.

    - `image.multithreaded` (optional): boolean to request a multithreaded render. Example: `{ "image": { "file": "out.ppm", "multithreaded": true } }`.
    - `image.bloom` (optional): enables a halo around HDR emissive pixels. Tune it with `bloomIntensity`, `bloomThreshold`, and `bloomRadius`.

Notes about tuples
- This codebase uses 4D tuples internally `(x, y, z, w)`. The JSON format uses 3-element arrays for positions and vectors; the loader converts them to the expected 4D tuples internally (points get `w=1`, direction vectors get `w=0`).

Example
-------
See `scenes/example_scene.json` for a working example. Run it like:

```powershell
./raytracer.exe --scene scenes/example_scene.json
```

Build & Dependency Notes
------------------------
- The loader uses the header-only library `nlohmann/json`. Install it with your package manager or via vcpkg.

Examples:

Windows (vcpkg):
```powershell
vcpkg install nlohmann-json
```

Ubuntu / Debian:
```bash
sudo apt install nlohmann-json3-dev
```

If you install via system packages, your compiler should find the header `<nlohmann/json.hpp>` automatically. If you use vcpkg, either integrate vcpkg into your build environment or adjust `CXXFLAGS` in the Makefile to include the vcpkg include path.

Quick Build
-----------
On Windows (MinGW) using the provided Makefile:

```powershell
mingw32-make
```

On Unix-like systems (if `make` is available):

```bash
make
```

If your build fails due to missing `nlohmann/json.hpp`, install the library or add its include directory to `CXXFLAGS` in the `Makefile` (the Makefile already uses `-Isrc`).

Where to look in the code
- Loader implementation: [src/SceneLoader.cpp](src/SceneLoader.cpp#L1)
- Loader header: [src/SceneLoader.h](src/SceneLoader.h#L1)
- Example scene: [scenes/example_scene.json](scenes/example_scene.json#L1)
- CLI integration: [src/main.cpp](src/main.cpp#L1)

Next steps
----------
- Add imported mesh and CSG objects to the JSON schema when loaders are implemented.
- Add automated validation tests for JSON scenes and unit tests for the loader.





Shadows

Ray tracer computes shadows by casting a ray, called a shadow ray. From each point of intersection toward the light source. If something intersects that shadow
ray between the point and the light source, then the point is considered to be a shadow. 


Implementing New Objects 

Base class Shape* is an abstract class that handles generic operations like transformations and material assignment. 

Plane
- Normal is always straight up at (0, 1, 0)
- Local Intersect: If the ray is parallel to the plane (ray.direction.y is close to 0), it misses completeley. Otherwise, t = -rayorigin.y  ray.direction.y 

Patterns 
A pattern is a function that accepts a point in space and returns a color. 


To create better Render
- Update resolution usually (800, 400) is high resolution, (200, 100) is low resolution for the camera 


Pattern Design choice using Shared_Ptr vs Raw Pointers
- using std::shared_ptr instead of raw pointers for the pattern system fundementally shifts responsibility of memory management 
from the developers to the compiler. 

Patterns are unique because they are structural data, they don't live in one single palce, and their layouts are often shared or deeply nested. 

A ray tracer shoots millions of ray, and calls LocalPatternAt() on every hit. If a ray hits an object with a deleted pattern, it will access garbage memory, resulting in a immediate seg fault. 

Shared_ptr uses reference counting; 

- When we create a pattern the ref count is 1, 
- When we pass it to a shape the count becomes 2
- When the scene function eneds the local variable dies, dropping the count back to 1. 
- Because the count is not zero, the pattern stays alive in memory for the shape to safely use during rendering. 


relection 

We add a reflection attribute to material. 

When the reflective is 0, the surface is completely NONREFLECTIVE, whereas setting it to 1 produces a perfect mirror. Numbers in between will represent partial reflections. 

The prepare_computations function will pre-compute the reflectV vector. 

To do this we will create a plane and position a ray above it, slanting downward at a 45 degree angle. Position the intersection on the plane, and have prepare_computations() compute the reflection vector. 

ReflectedColor()
Create a new ray originating at the hits location and pointing in the diretion of reflectv. Find the color of the new ray via color_at(). 
Then multiply the result by the reflective value. If reflective is set to something between 0-1, it will give you partial reflection. 

Implement int remaining to color_at() reflectedColor() and shadeHit() to limit recursion calls 

Groups are abstract shapes with no surface of their own. Taking their form instead from the shapes they contain.
