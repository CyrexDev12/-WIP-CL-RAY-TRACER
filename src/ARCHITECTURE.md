# Source architecture

The source tree is being migrated incrementally. The current boundaries are:

```text
src/
|-- app/                 Command-line entry point and application orchestration
|-- core/
|   `-- math/            Fixed-size shared math types for the new architecture
|-- loaders/             Input adapters such as the JSON scene loader
|-- renderers/
|   `-- cpu/             Existing CPU ray-tracing backend
|-- geometry/            Legacy geometry and intersection implementation
|-- math/                Legacy tuple and matrix implementation
`-- scene/               Legacy camera, materials, lighting, world, and image data
```

The intended dependency direction is:

```text
app -> loaders  -> core scene model
app -> renderer -> core scene model
```

The `CLRT::Math` target is the first part of the new core. `CLRT::Core` still names
the legacy engine library during migration and now depends on `CLRT::Math`.

## Math and coordinate conventions

- Scalars in the shared CPU scene and reference renderer are `double`.
- GPU records will explicitly convert shared values to packed `float` data.
- Matrices use contiguous row-major storage.
- Points and vectors are treated as column vectors, so `matrix * value` applies a
  transform.
- Matrix composition applies the rightmost transform first. For example,
  `translation * scaling * point` scales the point and then translates it.
- The world is right-handed and an untransformed camera looks down negative Z.
- Positive rotations follow the right-hand rule.
- Angles are expressed in radians.
- `Point3` and `Vec3` are distinct types; point-minus-point produces a vector.
- Colors are linear RGB values during rendering. Clamping and output transfer happen
  only at the output boundary.
- Matrices uploaded directly to OpenGL must account for OpenGL's column-major upload
  convention, either by transposing during upload or setting the upload transpose
  behavior explicitly.

Rules for new code:

1. `app` may coordinate modules but must not implement rendering algorithms.
2. Loaders create shared scene data; renderers do not parse files.
3. Renderers do not depend on each other.
4. Shared engine code does not include OpenGL or CUDA headers.
5. OpenGL and CUDA representations are derived from shared scene data.
6. Tests live under `tests`, never inside the application executable.

The existing `math`, `geometry`, and `scene` folders form the legacy `CLRT::Core`
implementation for now. They will migrate onto `core/math` before the owned scene
model is introduced. Keeping both representations temporarily preserves the current
CPU output as a baseline.

## Transitional math boundary

`math/LegacyMathAdapters` is the explicit bridge between heap-backed four-component
tuples/legacy matrices and `Point3`, `Vec3`, and `Mat4`. It may be used while porting
an existing subsystem, but new render backends and new shared scene types must use
`core/math` directly.

The active CPU rendering path now uses fixed-size math:

- `geometry/Ray` stores a fixed `Point3` origin and `Vec3` direction while preserving
  temporary constructors and transforms for legacy callers.
- `Camera` stores `Mat4` directly and caches its inverse whenever its transform is
  changed.
- `ray_for_pixel` uses only fixed-size math and does not allocate or invert a matrix
  per pixel.
- Shapes store `Mat4` transforms and cache both inverse and inverse-transpose
  matrices. Primitive intersections, normals, group bounds, and hit computations
  use `Point3` and `Vec3` directly.
- Lights store `Point3` positions. Shading, shadows, reflection, refraction, and
  pattern evaluation stay in fixed-size math through the active render path.
- The JSON loader creates fixed points, vectors, camera transforms, shape transforms,
  and lights without routing through legacy tuples or matrices.

Legacy tuple constructors and overloads remain at subsystem edges so the optional
legacy test executable continues to compile. They are compatibility APIs, not the
math representation for new engine or renderer code.
