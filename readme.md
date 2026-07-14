AI Scene Generator
------------------
The Python generator turns a natural-language description into scene JSON that the
C++ ray tracer can render. It uses OpenAI Structured Outputs and validates every
response against the renderer's actual limits before writing a file.

### 1. Build the ray tracer

The canonical build uses CMake. From the project root with the current MinGW
toolchain:

```powershell
cmake --preset mingw-debug
cmake --build --preset mingw-debug
ctest --preset mingw-debug
```

Verify the renderer with the included example:

```powershell
.\build\cmake-mingw-debug\src\raytracer.exe --scene scenes/example_scene.json
```

The original Makefile remains available during the migration:

```powershell
mingw32-make
.\raytracer.exe --scene scenes/example_scene.json
```

The current module boundaries and dependency rules are documented in
[`src/ARCHITECTURE.md`](src/ARCHITECTURE.md).

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
resolution, and multithreading. For example:

```powershell
python tools/generate_scene.py `
  "A blue glass planet between two small red moons, viewed from slightly above. Use 300 by 150 resolution." `
  -o scenes/glass_planet.json
```

Generator options:

- `-o` or `--output` is required and must point to a `.json` file.
- `--force` allows an existing JSON file to be replaced.
- `--model MODEL_NAME` overrides the default model for one request.
- `OPENAI_SCENE_MODEL` changes the default model for the current shell.
- The default model is `gpt-5.4-mini`.

Example using all relevant options:

```powershell
python tools/generate_scene.py "A simple solar system" `
  -o scenes/planets.json `
  --model gpt-5.4-mini `
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
`src/loaders/SceneLoader.cpp`:

- Exactly one point light.
- Between 1 and 100 sphere objects.
- Sphere scaling and translation; rotation is not currently loaded from JSON.
- RGB color components from `0` to `1`.
- `ambient`, `diffuse`, `specular`, `reflective`, and `transparency` from `0` to `1`.
- `shininess` from `10` to `200`, inclusive. Other values terminate the C++ renderer.
- `refractiveIndex` from `1` to `3`.
- A field of view greater than `0` and less than pi radians.
- Image dimensions from `1` to `4096`; `image.width/height` must match
  `camera.hsize/vsize`.
- A simple `.ppm` output filename without directory components.
- Non-zero scale values and valid camera vectors.

Floors and walls are approximated using heavily scaled spheres because planes are
not yet supported by the JSON loader.

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

The schema and boundary checks live in `tools/scene_schema.py`. The renderer-specific
AI instructions live in `tools/scene_prompt.py`. Update both whenever the C++ scene
loader gains a feature or changes a numeric boundary.


OpenGL and NVIDIA CUDA Roadmap
------------------------------

The long-term goal is an application that can load one scene and use three render
paths:

- The existing CPU ray tracer, retained as the correctness reference and fallback.
- An OpenGL rasterized preview for immediate scene inspection and camera movement.
- An NVIDIA CUDA ray tracer that can progressively render into the OpenGL window.

OpenGL and CUDA have different responsibilities. OpenGL first provides a fast,
approximate view of the scene; it is not expected to exactly reproduce recursive
reflection, refraction, patterns, or ray-traced shadows. CUDA will later provide the
accurate ray-traced viewport. All three paths must consume the same scene model so
that the project does not develop separate, incompatible scene representations.

### Target architecture

```text
Scene JSON
    |
    v
Shared owned Scene model
    |---> CPU ray tracer ------> ImageBuffer ------> PPM/image output
    |---> OpenGL preview --------------------------> Interactive window
    `---> CUDA ray tracer -----> OpenGL texture ---> Interactive window
```

The shared engine code should not depend on OpenGL or CUDA. Renderer-specific code
may translate the shared scene into backend-friendly data, such as preview meshes
for OpenGL or flat device buffers for CUDA.

### Two-contributor implementation strategy

The work is split into two lanes so contributors can make progress in parallel after
the shared interfaces are agreed upon:

- **Contributor A - Core and CPU lane:** fixed-size math, scene ownership, CPU
  renderer extraction, JSON loading, reference tests, and CPU/CUDA comparisons.
- **Contributor B - Graphics and GPU lane:** CMake/toolchain setup, GLFW/OpenGL
  viewer, preview meshes and shaders, CUDA kernels, and CUDA/OpenGL interoperability.
- **Shared integration points:** scene structures, camera conventions, material
  layout, renderer interfaces, CLI behavior, and acceptance of reference images.

The contributor labels describe work lanes, not permanent ownership. Add a name or
GitHub handle beside a checklist item when claiming it. Avoid changing a shared
interface on one lane without notifying the other contributor first.

Recommended collaboration rules:

- Keep pull requests focused on one checklist group or one backend boundary.
- Land shared types and interfaces before code that consumes them.
- Do not mix broad math/scene refactors with OpenGL or CUDA implementation in the
  same pull request.
- Keep the CPU backend working at every integration checkpoint.
- Add or update tests whenever a shared data structure or rendering convention
  changes.
- Document matrix layout, handedness, camera forward direction, angle units, and
  color range before connecting the OpenGL and CUDA backends.
- Use small deterministic scenes for correctness comparisons and separate larger
  scenes for performance measurements.

REFACTOR PLAN (NEW ADDITIONS)

  NEW 3D Model Integration 

  We will be implementing obj wavewront files for the OpenGL addition. 

  This will allow us to create immersive environemts, rather than using CSG. 

### Phase 0 - Baseline and decisions

Both contributors should complete this phase together. Its output is a stable
reference against which the refactor and GPU renderer can be checked.

- [x] **Shared:** Record build instructions and supported development platforms.
      Owner: Codex.
- [x] **Shared:** Select and document the OpenGL dependencies. Initial recommendation:
      GLFW for the window/input layer and GLAD for OpenGL function loading.
      Owner: unassigned. (NOTE: FIISHED, located in docs/dependencies)
- [x] **Shared:** Adopt CMake as the canonical build system while temporarily keeping
      the current Makefile available during migration. Owner: Codex.
- [x] **Shared:** Document the math conventions: row/column-major storage,
      multiplication order, coordinate handedness, camera forward axis, and degrees
      versus radians. Owner: Codex.
- [x] **Core/CPU:** Save small CPU reference renders covering a sphere, plane,
      triangle, shadow, reflection, transparency/refraction, Schlick reflectance,
      pattern, group, and bounding box. Owner: unassigned.
- [ ] **Core/CPU:** Add repeatable timing output that excludes JSON loading and file
      writing. Owner: unassigned. (IMPLEMENT LATER; PUSHBACK TO LATER PHASE)
- TODO:: NEXT [ ] **Graphics/GPU:** Verify the chosen NVIDIA GPU, driver, Visual Studio/MSVC, and
      CUDA toolkit by compiling and running a minimal CUDA program. Owner: unassigned.

Acceptance criteria:

- The current example scenes still render through the CPU path.
- Reference images and baseline timings are recorded reproducibly.
- Both contributors agree on the shared coordinate and matrix conventions.

### Phase 1 - Shared engine foundation

CUDA kernels cannot directly use the current virtual shape hierarchy,
`std::vector<double>` tuples, or `vector<vector<double>>` matrices efficiently. This
phase creates fixed-layout engine types without changing rendered results.

- [x] **Core/CPU:** Add fixed-size `Vec3`, `Point3`, `Color`, `Mat4`, and `Ray` types
      with contiguous storage and no per-operation heap allocation. Owner: Codex.
- [x] **Core/CPU:** Port tuple/vector operations and tests to the fixed-size types.
      Legacy tuple overloads remain only as explicit compatibility adapters while
      older callers are retired. Owner: Codex.
- [x] **Core/CPU:** Port matrix transforms, inversion, and camera ray generation.
      Cache the inverse camera transform instead of calculating it for every pixel.
      Owner: Codex.
- [x] **Core/CPU:** Port shape transforms, intersections, normals, group bounds, hit
      computations, lighting vectors, patterns, shadows, reflection, and refraction
      to fixed-size math. Cache inverse and inverse-transpose transforms on shapes
      and patterns. Owner: Codex.
- [ ] **Shared:** Define an owned, enumerable `Scene` containing cameras, lights,
      objects, transforms, and materials. Owner: unassigned.
- [ ] **Core/CPU:** Replace ambiguous raw ownership in `World` and the JSON loader
      with RAII ownership. Owner: unassigned. (IMPORTANT)
- [ ] **Shared:** Give renderable objects and materials stable IDs so intersections
      and backend data do not depend on host pointers. Owner: unassigned.
- [ ] **Shared:** Decide how groups are flattened into final world transforms before
      a scene is handed to a rendering backend. Owner: unassigned.
- [ ] **Core/CPU:** Extend the JSON loader and its Python schema/prompt together when
      adding planes, cubes, cylinders, triangles, groups, patterns, or additional
      lights. Owner: unassigned.

Acceptance criteria:

- Existing CPU reference scenes remain visually equivalent within an agreed numeric
  tolerance.
- The shared scene owns its data and can be enumerated without exposing `World`
  internals.
- No ray or vector operation allocates dynamic memory in the render hot path.

### Phase 2 - Renderer boundaries and build system

- [ ] **Shared:** Introduce a renderer-facing `Scene` snapshot and `CameraState`.
      Owner: unassigned.
- [ ] **Core/CPU:** Move the existing render loop behind a `CpuRayTracer` interface.
      Owner: unassigned.
- [ ] **Shared:** Replace PPM-specific render output with a general floating-point
      `ImageBuffer`; keep PPM encoding as a separate output step. Owner: unassigned.
- [ ] **Graphics/GPU:** Create CMake targets for shared core code, CPU rendering,
      OpenGL, CUDA, the application, and tests. Owner: unassigned.
- [ ] **Graphics/GPU:** Add dependency handling for GLFW and GLAD. Owner: unassigned.
- [ ] **Shared:** Add CLI backend selection, with planned forms such as
      `--backend cpu`, `--view`, `--view-after-render`, and later `--backend cuda`.
      Owner: unassigned.
- [ ] **Shared:** Add clear runtime errors when OpenGL or CUDA support is unavailable.
      Owner: unassigned.

Acceptance criteria:

- The CPU renderer can be selected explicitly and produces the reference output.
- A non-graphical build remains possible when OpenGL/CUDA targets are disabled.
- Shared engine headers do not include OpenGL or CUDA headers.

### Phase 3 - OpenGL interactive preview

- [ ] **Graphics/GPU:** Create a GLFW window and modern OpenGL context with a clean
      startup/shutdown path. Owner: unassigned.
- [ ] **Graphics/GPU:** Add shader compilation/linking with actionable error output.
      Owner: unassigned.
- [ ] **Graphics/GPU:** Add resize-aware projection and framebuffer handling.
      Owner: unassigned.
- [ ] **Graphics/GPU:** Add WASD movement, mouse look, configurable movement speed,
      and a key to reset to the scene camera. Owner: unassigned.
- [ ] **Graphics/GPU:** Generate reusable preview meshes for spheres, planes, cubes,
      cylinders, and triangles. Owner: unassigned.
- [ ] **Graphics/GPU:** Upload object transforms and material properties from the
      shared scene. Owner: unassigned.
- [ ] **Graphics/GPU:** Implement approximate direct lighting and emissive materials
      in GLSL. Owner: unassigned.
- [ ] **Graphics/GPU:** Implement `--view` without requiring a completed ray trace.
      Owner: unassigned.
- [ ] **Graphics/GPU:** Implement `--view-after-render` and optionally display the
      completed CPU image as an overlay or comparison panel. Owner: unassigned.
- [ ] **Core/CPU:** Add shared camera tests proving that CPU rays and the OpenGL camera
      agree on position, orientation, field of view, and aspect ratio. Owner:
      unassigned.

Acceptance criteria:

- A loaded scene opens in a responsive 3D window and every supported JSON object is
  represented in the expected location, scale, and orientation.
- The user can navigate freely and reset to the authored render camera.
- Closing the viewer releases its graphics resources cleanly.

### Phase 4 - Minimal CUDA ray tracer

The CUDA renderer should use a flat, data-oriented copy of the shared scene. Do not
attempt to copy C++ virtual objects, `shared_ptr`, or host pointers to device memory.

- [ ] **Graphics/GPU:** Add CUDA as a first-class CMake language and compile `.cu`
      sources with the supported MSVC host compiler. Owner: unassigned.
- [ ] **Graphics/GPU:** Implement CUDA error-checking and RAII wrappers for device
      buffers. Owner: unassigned.
- [ ] **Shared:** Define packed GPU scene records for camera, materials, lights, and
      each initially supported primitive. Owner: unassigned.
- [ ] **Core/CPU:** Implement and test the host-to-device scene flattener. Owner:
      unassigned.
- [ ] **Graphics/GPU:** Launch one CUDA thread per pixel and produce a test gradient.
      Owner: unassigned.
- [ ] **Graphics/GPU:** Generate camera rays and render sphere/plane silhouettes and
      normals. Owner: unassigned.
- [ ] **Graphics/GPU:** Add closest-hit selection, materials, Phong lighting, and
      shadows. Owner: unassigned.
- [ ] **Core/CPU:** Build automated CPU/CUDA pixel comparisons with tolerances and
      diagnostic difference images. Owner: unassigned.
- [ ] **Graphics/GPU:** Add triangles, cubes, and cylinders after sphere/plane output
      matches the CPU reference. Owner: unassigned.

Acceptance criteria:

- CUDA renders the agreed basic scenes within the documented CPU/GPU tolerance.
- Backend selection falls back or fails clearly when no compatible NVIDIA device is
  available.
- Timings separate scene upload, kernel execution, and output transfer.

### Phase 5 - CUDA feature parity

- [ ] **Graphics/GPU:** Add bounded reflection using an iterative ray stack or queue.
      Owner: unassigned.
- [ ] **Graphics/GPU:** Add refraction, total internal reflection, and Schlick
      reflectance. Owner: unassigned.
- [ ] **Graphics/GPU:** Add patterns and emissive material behavior. Owner:
      unassigned.
- [ ] **Graphics/GPU:** Support the remaining shared scene light and primitive types.
      Owner: unassigned.
- [ ] **Core/CPU:** Expand CPU/CUDA reference comparisons for every added feature.
      Owner: unassigned.
- [ ] **Shared:** Define supported recursion depth and behavior when device limits are
      reached. Owner: unassigned.

Acceptance criteria:

- Every declared CUDA-supported feature has a CPU comparison scene and test.
- Unsupported scene features produce explicit errors rather than silently rendering
  incorrectly.

### Phase 6 - CUDA/OpenGL interactive ray tracing

- [ ] **Graphics/GPU:** Allocate an OpenGL texture or pixel buffer suitable for CUDA
      interoperability. Owner: unassigned.
- [ ] **Graphics/GPU:** Register the resource with CUDA once, then map, render, unmap,
      and display it safely each frame. Owner: unassigned.
- [ ] **Graphics/GPU:** Reset accumulation when the camera or scene changes. Owner:
      unassigned.
- [ ] **Graphics/GPU:** Add progressive accumulation while the camera is stationary.
      Owner: unassigned.
- [ ] **Graphics/GPU:** Allow switching between raster preview and CUDA ray-traced
      display without reloading the scene. Owner: unassigned.
- [ ] **Shared:** Add resolution scale, sample count, recursion depth, and render-mode
      controls. Owner: unassigned.
- [ ] **Core/CPU:** Verify that saving an interactive CUDA frame uses the same color
      conversion rules as CPU output. Owner: unassigned.

Acceptance criteria:

- Camera movement produces a responsive low-sample image and stopping movement
  progressively improves it.
- CUDA never writes a graphics resource while OpenGL is using it.
- Resizing, switching modes, reloading scenes, and closing the application do not
  leak or invalidate GPU resources.

### Phase 7 - Acceleration and polish

- [ ] **Core/CPU:** Build a CPU-side bounding volume hierarchy from flattened scene
      bounds. Owner: unassigned.
- [ ] **Graphics/GPU:** Upload flat BVH nodes and traverse them in CUDA. Owner:
      unassigned.
- [ ] **Shared:** Add benchmark scenes and publish CPU single-threaded, CPU
      multithreaded, and CUDA timing methodology. Owner: unassigned.
- [ ] **Graphics/GPU:** Profile memory access, branch divergence, occupancy, and
      transfer costs before applying optimizations. Owner: unassigned.
- [ ] **Graphics/GPU:** Add optional OpenGL instancing and preview shadow maps if they
      materially improve real scenes. Owner: unassigned.
- [ ] **Shared:** Update setup, controls, troubleshooting, screenshots, and feature
      compatibility documentation. Owner: unassigned.

Acceptance criteria:

- Performance changes include before/after measurements and preserve correctness.
- The README accurately identifies which features are supported by CPU, OpenGL
  preview, and CUDA.

### Definition of done for each checklist item

An item is complete only when its implementation builds in the intended
configuration, relevant tests pass, user-visible behavior is documented, and the
other contributor can reproduce it from a clean build. Check the item and replace
`Owner: unassigned` with the contributor's name or handle in the same pull request.

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
This project supports loading scenes described in JSON. The loader understands a compact schema that covers the common elements needed to build a scene: image output settings, a camera, lights, and objects (spheres supported currently).

Top-level keys
- `image` (optional): `{ "width": int, "height": int, "file": string }` — output image settings.
- `camera` (optional): `{ "hsize": int, "vsize": int, "fov": float, "from": [x,y,z], "to": [x,y,z], "up": [x,y,z] }` — camera and view transform.
- `lights` (optional): an array of lights. Supported light object example:
    - `{ "type": "point", "position": [x,y,z], "color": [r,g,b] }`
- `objects` (optional): an array of scene objects. Currently supported:
    - Sphere:
        - `type`: "sphere"
        - `transform`: optional object with `scale` and/or `translate` arrays: `{ "scale": [sx,sy,sz], "translate": [tx,ty,tz] }`
        - `material`: optional object with properties like `color` (`[r,g,b]`), `ambient`, `diffuse`, `specular`, `shininess`, `reflective`, `transparency`, `refractiveIndex`.

    - `image.multithreaded` (optional): boolean to request a multithreaded render. Example: `{ "image": { "file": "out.ppm", "multithreaded": true } }`.

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

If you install via system packages, CMake should find `<nlohmann/json.hpp>`
automatically. With a non-standard installation, set
`CLRT_NLOHMANN_JSON_INCLUDE_DIR` to the directory containing the `nlohmann` folder.

Quick Build
-----------
On Windows using the MinGW CMake preset:

```powershell
cmake --preset mingw-debug
cmake --build --preset mingw-debug
ctest --preset mingw-debug
```

Visual Studio 2022 users can substitute the `msvc-debug` preset after installing
the C++ desktop build tools. The legacy `mingw32-make` path remains available while
the build migration is in progress.

If configuration fails because `nlohmann/json.hpp` is missing, install the library
or provide `CLRT_NLOHMANN_JSON_INCLUDE_DIR` during CMake configuration.

Where to look in the code
- Loader implementation: [src/loaders/SceneLoader.cpp](src/loaders/SceneLoader.cpp#L1)
- Loader header: [src/loaders/SceneLoader.h](src/loaders/SceneLoader.h#L1)
- Example scene: [scenes/example_scene.json](scenes/example_scene.json#L1)
- CLI integration: [src/app/main.cpp](src/app/main.cpp#L1)

Next steps
----------
- Add more object types (planes, cubes, triangles) and patterns to the JSON schema.
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
