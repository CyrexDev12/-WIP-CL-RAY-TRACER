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
implementation for now. The fixed-size math migration is complete, and
`scene/Scene` now supplies the move-only ownership and enumeration boundary while
it temporarily owns legacy polymorphic `Shape` and `Light` implementations. Keeping
that bridge preserves the current CPU output as stable IDs and backend snapshots
are introduced.

## Owned scene boundary

`clrt::scene::Scene` owns cameras by value and polymorphic lights and shapes through
`std::unique_ptr`. Its collections are read-only to consumers but fully enumerable.
Each `SceneObject` exposes the owned shape together with its transform and material;
those properties remain stored on `Shape` during migration so there is no duplicate
state to drift out of sync.

The active JSON/`World` CPU path still predates this boundary, but its resource
ownership is now explicit: `World` uniquely owns every shape and its light, while
`Lighting` only borrows that light for shading. The JSON loader constructs a
temporary RAII-owned world and commits it only after the entire document validates.
Moving that data directly into `Scene` remains a later integration step.

## Stable scene identities

`ObjectId`, `MaterialId`, `MeshAssetId`, and `MeshInstanceId` are distinct,
trivially-copyable 32-bit types. Their maximum value is reserved as an invalid
sentinel. `World` and `Scene` assign object and material IDs deterministically in
root insertion order and depth-first preorder for group descendants. IDs survive
container growth and moves because they are values rather than addresses.

`Intersection` and `Computations` contain object/material IDs only. CPU code uses
the `ObjectResolver` interface to look up a shape when it needs virtual geometry or
material behavior. The resolver may maintain internal pointers to owned CPU objects,
but intersection records and data copied to OpenGL or CUDA must never contain those
host pointers.

`MeshAsset` is the immutable shared representation for imported geometry. It owns a
unified vertex buffer, triangle index buffer, calculated bounds, named material
slots and index-aligned slot ranges, plus original/normalized source metadata.
Construction rejects invalid IDs, non-finite or non-normalized vertices, incomplete
triangles, out-of-range indices, and invalid material ranges. A later asset cache
will assign `MeshAssetId`; instances will refer to it using `MeshInstanceId` and
will not duplicate these buffers.

`MeshInstance` is the separate per-scene placement record. It contains only its
`MeshInstanceId`, referenced `MeshAssetId`, transform with cached inverse and
inverse-transpose, and optional material-slot-to-`MaterialId` overrides. It never
contains vertex or index buffers. `Scene` stores immutable mesh assets and mesh
instances in separate enumerable collections; multiple instances can therefore
share the same asset allocation while using different transforms and overrides.
Instance creation rejects unknown assets, invalid material slots, duplicate slot
overrides, invalid IDs, and non-invertible transforms.

## Renderer scene snapshots

The authoring scene retains its group hierarchy, but renderers receive a flat
`SceneSnapshot`. The shared `buildSceneSnapshot` pass walks analytic roots in
insertion order and group children in preorder. It composes column-vector
transforms as `parentWorld * local`, omits non-renderable group nodes, and emits one
`FlattenedObject` per analytic leaf. Mesh instances are emitted separately in
instance-ID order; an instance may name a parent group by `ObjectId` and receives
that group's accumulated world transform.

Each flattened record contains stable IDs, final world transform, cached inverse,
cached inverse-transpose, and a negative-determinant orientation flag. Flattened
mesh records additionally retain their asset ID and material overrides, but never
copy asset geometry. This makes transform order and normal/winding behavior
identical for CPU, OpenGL, and CUDA consumers.

Snapshots are immutable derived data. The initial policy is explicit whole-snapshot
rebuilding after any group or local transform change. Rebuilding recalculates only
compact instance/object records; it does not reload OBJ files or recreate vertex and
index buffers. Dirty-subtree caching may replace the whole rebuild later without
changing the renderer-facing snapshot format.

## Generated JSON scene contract

The JSON loader, `tools/scene_schema.py`, and `tools/scene_prompt.py` form one feature
boundary and must change together. They currently support spheres, planes, cubes,
finite cylinders, triangles, recursive groups, transformed basic/perturbed
patterns, and one to four point lights. Object and pattern transforms compose as
translation, Z/Y/X rotation, then scale, which applies scale first, then X/Y/Z
rotation, then translation to column vectors. Loader construction is recursive and
transactional: a malformed child, pattern, transform, or material rejects the whole
load without modifying the caller's existing scene state.

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
