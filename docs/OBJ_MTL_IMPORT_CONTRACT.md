# Initial OBJ/MTL import contract

Status: **locked for the first OBJ milestone**.

This document is the normative contract for the shared OBJ asset loader. The CPU,
OpenGL, and CUDA backends must consume the same imported `MeshAsset`; a backend must
not reinterpret the rules below. Expanding this subset requires updating this
document and adding deterministic fixtures before changing the loader.

## Supported OBJ syntax

The first milestone recognizes these records:

| Record | Initial behavior |
|---|---|
| `v x y z` | Required three-dimensional position. Homogeneous `w` and vertex colors are unsupported. |
| `vt u [v]` | One- or two-dimensional texture coordinate. Missing `v` is `0`. A third `w` component is unsupported. UVs are retained but not sampled initially. |
| `vn x y z` | Explicit non-zero normal. It is normalized during import. |
| `f ...` | Face with at least three corners using `v`, `v/vt`, `v//vn`, or `v/vt/vn`. All corners in one face must use the same form. |
| `o name` | Object label retained as source metadata. An empty name clears it. |
| `g [name ...]` | Group membership retained as source metadata. An empty record clears it. |
| `s off`, `s 0`, `s N` | Disable smoothing or select positive decimal smoothing-group `N`. |
| `usemtl name` | Select a material for subsequent faces. The name is the trimmed remainder of the line. |
| `mtllib file ...` | Reference one or more whitespace-delimited material-library paths. |

Blank lines and comments beginning with `#` are ignored; `#` also starts an inline
comment. Spaces and tabs separate fields. Logical-line continuation, quoted or
escaped filenames, free-form curves/surfaces, parameter-space vertices, and all
other OBJ records are outside the initial subset. Unsupported records produce a
line-numbered warning and are otherwise ignored. Malformed instances of a supported
record are errors, not warnings. All parsed numbers must be finite.

OBJ position, texture-coordinate, and normal indices remain independent until
canonicalization. A positive index is one-based. A negative index is relative to
the end of the corresponding list as it exists at that `f` record. Index zero,
an out-of-range index, and a missing required component are line-numbered errors.
Each unique resolved `(position, texture coordinate, normal)` combination becomes
one `MeshVertex` and the mesh has one unified index buffer. A missing UV uses a
distinct no-UV sentinel rather than `(0, 0)`.

## Supported MTL syntax

The initial material subset is:

| Record | Meaning |
|---|---|
| `newmtl name` | Begin a named material. |
| `Ka r g b` | Ambient color. |
| `Kd r g b` | Diffuse color. |
| `Ks r g b` | Specular color. |

Color values are retained without implicit color-space conversion or clamping.
Repeating a supported property within one material is an error. Duplicate
`newmtl` names across the libraries used by one OBJ are also errors. Statements
before the first `newmtl` are errors. Missing properties use the shared renderer's
documented default mesh-material values.

Texture maps (including `map_Kd`), bump/normal maps, reflection maps, spectral/XYZ
colors, `Ns`, `d`, `Tr`, `illum`, and vendor extensions are not loaded initially.
They produce a warning so an asset is not silently presented as fully supported,
then the remainder of that line is ignored. Other unknown MTL records follow the
same warning policy.

Faces with no `usemtl`, an unresolved `usemtl`, or a material whose library could
not be read use the renderer's named default mesh material. Missing/unreadable MTL
files and unresolved material names produce source-located warnings, but do not
discard otherwise valid geometry. A missing/unreadable OBJ is fatal.

## Coordinates, winding, and transforms

- Import preserves OBJ `x`, `y`, and `z` values exactly. It performs no axis swap,
  handedness conversion, origin shift, unit conversion, or automatic scaling.
- Import preserves UV values exactly; in particular, it does not flip `v`.
- Face corner order is preserved. In the project's right-handed coordinates, the
  geometric normal of triangle `(p0, p1, p2)` is
  `normalize(cross(p1 - p0, p2 - p0))`. A front face is counter-clockwise when
  viewed from the side toward which that normal points.
- Explicit `vn` values are normalized but are not flipped to agree with a geometric
  normal. A zero-length explicit normal is an error.
- The existing analytic `Triangle` class uses the historical
  `cross(edge2, edge1)` convention. That legacy behavior does not define mesh
  winding and must not be copied into the OBJ loader.
- Orientation, scale, and units are changed only by a `MeshInstance` scene
  transform. Normals use the inverse transpose of that transform. A transform with
  a negative determinant reverses world-space winding; each backend must account
  for that when doing face culling and must not mutate the shared asset indices.

Initial CPU ray intersection is two-sided. OpenGL preview culling must either be
disabled or honor the winding rule (including negative-determinant instances).

## Polygon triangulation

Triangles are emitted in source-face order. A face with corners
`(c0, c1, ..., cn-1)` is triangulated as the deterministic fan
`(c0, c1, c2)`, `(c0, c2, c3)`, ..., `(c0, cn-2, cn-1)`. This preserves source
winding and material, object, group, and smoothing metadata on every emitted
triangle.

The initial subset accepts triangles and simple, convex, planar polygons. Before
triangulation, the loader rejects faces that have fewer than three distinct
positions, are self-intersecting, are non-convex, are non-planar beyond a tolerance
of `1e-9 * max(1, face bounding-box diagonal)` in OBJ units, or would emit a
zero-area triangle at that same scale. These are line-numbered errors. Concave-
polygon ear clipping and polygon repair are later features; the loader must not
guess or silently drop bad corners.

## Missing-normal policy

Every corner of a face must either provide `vn` or omit it; a partially specified
face is an error. Assets may otherwise mix faces with and without explicit normals.
Explicit normals always win for the faces that provide them.

For a face without `vn`:

1. Compute each emitted triangle's unnormalized geometric cross product using the
   winding rule above. Degenerate results have already been rejected.
2. With smoothing off (`s off` or `s 0`, also the initial state), assign the
   normalized geometric face normal and split vertices at that source-face
   boundary. All triangles from one planar polygon receive the same polygon normal.
3. With smoothing group `N`, sum the unnormalized triangle normals for all incident
   corners that share the same source position index and smoothing-group number,
   then normalize the sum. The accumulation key deliberately ignores UV seams,
   object/group labels, and material slots. Different smoothing-group numbers never
   share a generated normal.

The accumulation order is source face order followed by emitted triangle order so
results are deterministic. If accumulated vectors cancel to zero, import fails
instead of selecting an arbitrary normal.

## Asset paths and cache identity

- A mesh path in scene JSON is resolved relative to the directory containing the
  scene file, never relative to the process working directory.
- An `mtllib` path is resolved relative to the directory of the normalized absolute
  OBJ reference. If that reference names a symbolic link, its containing directory
  is used rather than the link target's directory.
- Relative `.` and `..` components are allowed. Absolute paths are accepted for
  local authoring but make a scene non-portable and produce a warning. JSON should
  use forward slashes for portable paths.
- Paths are interpreted as filesystem paths only. The initial loader does not fetch
  URLs, expand environment variables or `~`, or search fallback directories.
- The cache key is the normalized absolute OBJ path after resolving `.` and `..`.
  Symbolic links are not dereferenced while forming the key, so two differently
  located links cannot accidentally reuse an asset with a different relative MTL
  dependency. Equality follows the host filesystem's case-sensitivity rules. MTL
  paths are not part of instance identity because they are dependencies of the
  cached OBJ asset.
- The loader retains the scene-relative spelling, normalized resolved path, and
  source locations as metadata for diagnostics. Repeated references with the same
  cache identity share one immutable `MeshAsset`; transforms and material overrides
  remain on separate `MeshInstance` objects.

Path resolution must report the scene, OBJ, or MTL filename and line number that
introduced a failing reference whenever that information exists.
