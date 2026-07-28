"""Deterministic composition analysis and camera fitting for generated scenes."""

from __future__ import annotations

from dataclasses import asdict, dataclass
import math
from statistics import median
from typing import Iterable, Sequence

try:
    from .scene_schema import Group, Material, Scene, SceneObject, Transform
except ImportError:
    from scene_schema import Group, Material, Scene, SceneObject, Transform


Vector3 = tuple[float, float, float]
Matrix4 = tuple[
    tuple[float, float, float, float],
    tuple[float, float, float, float],
    tuple[float, float, float, float],
    tuple[float, float, float, float],
]


@dataclass(frozen=True)
class AuditThresholds:
    """Thresholds chosen for readable benchmark compositions."""

    safe_frame_margin: float = 0.10
    minimum_object_area: float = 0.004
    minimum_scene_fill: float = 0.18
    maximum_scene_fill: float = 0.92
    occlusion_coverage: float = 0.80
    excessive_emissive_strength: float = 6.0


@dataclass(frozen=True)
class Bounds:
    minimum: Vector3
    maximum: Vector3

    @property
    def center(self) -> Vector3:
        return tuple(
            (self.minimum[index] + self.maximum[index]) / 2.0 for index in range(3)
        )  # type: ignore[return-value]

    @property
    def extents(self) -> Vector3:
        return tuple(
            self.maximum[index] - self.minimum[index] for index in range(3)
        )  # type: ignore[return-value]

    @property
    def diagonal(self) -> float:
        return math.sqrt(sum(component * component for component in self.extents))

    def corners(self) -> tuple[Vector3, ...]:
        return tuple(
            (x, y, z)
            for x in (self.minimum[0], self.maximum[0])
            for y in (self.minimum[1], self.maximum[1])
            for z in (self.minimum[2], self.maximum[2])
        )


@dataclass(frozen=True)
class SceneLeaf:
    path: str
    object_type: str
    bounds: Bounds
    material: Material
    backdrop: bool = False


@dataclass(frozen=True)
class ObjectProjection:
    path: str
    object_type: str
    ndc_bounds: tuple[float, float, float, float] | None
    screen_area: float
    center_depth: float
    backdrop: bool


@dataclass(frozen=True)
class AuditIssue:
    code: str
    severity: str
    message: str
    object_path: str | None = None


@dataclass(frozen=True)
class SceneAuditReport:
    inventory: dict[str, int]
    projections: tuple[ObjectProjection, ...]
    issues: tuple[AuditIssue, ...]
    scene_fill: float

    @property
    def error_count(self) -> int:
        return sum(issue.severity == "error" for issue in self.issues)

    @property
    def warning_count(self) -> int:
        return sum(issue.severity == "warning" for issue in self.issues)

    @property
    def summary(self) -> str:
        return (
            f"{sum(self.inventory.values())} finite objects, "
            f"{self.error_count} errors, {self.warning_count} warnings, "
            f"{self.scene_fill:.0%} frame coverage"
        )

    def to_dict(self) -> dict[str, object]:
        return asdict(self)


@dataclass(frozen=True)
class SceneRepairResult:
    material_adjustments: int = 0
    emission_adjustments: int = 0
    objects_repositioned: int = 0
    camera_adjusted: bool = False

    @property
    def changed(self) -> bool:
        return any(
            (
                self.material_adjustments,
                self.emission_adjustments,
                self.objects_repositioned,
                self.camera_adjusted,
            )
        )


def _identity() -> Matrix4:
    return (
        (1.0, 0.0, 0.0, 0.0),
        (0.0, 1.0, 0.0, 0.0),
        (0.0, 0.0, 1.0, 0.0),
        (0.0, 0.0, 0.0, 1.0),
    )


def _matrix_multiply(left: Matrix4, right: Matrix4) -> Matrix4:
    return tuple(
        tuple(
            sum(left[row][index] * right[index][column] for index in range(4))
            for column in range(4)
        )
        for row in range(4)
    )  # type: ignore[return-value]


def _transform_point(matrix: Matrix4, point: Sequence[float]) -> Vector3:
    value = (float(point[0]), float(point[1]), float(point[2]), 1.0)
    result = tuple(
        sum(matrix[row][column] * value[column] for column in range(4))
        for row in range(4)
    )
    return result[0], result[1], result[2]


def _transform_matrix(transform: Transform) -> Matrix4:
    sx, sy, sz = transform.scale
    rx, ry, rz = transform.rotate
    tx, ty, tz = transform.translate
    scale: Matrix4 = (
        (sx, 0.0, 0.0, 0.0),
        (0.0, sy, 0.0, 0.0),
        (0.0, 0.0, sz, 0.0),
        (0.0, 0.0, 0.0, 1.0),
    )
    rotate_x: Matrix4 = (
        (1.0, 0.0, 0.0, 0.0),
        (0.0, math.cos(rx), -math.sin(rx), 0.0),
        (0.0, math.sin(rx), math.cos(rx), 0.0),
        (0.0, 0.0, 0.0, 1.0),
    )
    rotate_y: Matrix4 = (
        (math.cos(ry), 0.0, math.sin(ry), 0.0),
        (0.0, 1.0, 0.0, 0.0),
        (-math.sin(ry), 0.0, math.cos(ry), 0.0),
        (0.0, 0.0, 0.0, 1.0),
    )
    rotate_z: Matrix4 = (
        (math.cos(rz), -math.sin(rz), 0.0, 0.0),
        (math.sin(rz), math.cos(rz), 0.0, 0.0),
        (0.0, 0.0, 1.0, 0.0),
        (0.0, 0.0, 0.0, 1.0),
    )
    translate: Matrix4 = (
        (1.0, 0.0, 0.0, tx),
        (0.0, 1.0, 0.0, ty),
        (0.0, 0.0, 1.0, tz),
        (0.0, 0.0, 0.0, 1.0),
    )
    return _matrix_multiply(
        translate,
        _matrix_multiply(
            rotate_z,
            _matrix_multiply(rotate_y, _matrix_multiply(rotate_x, scale)),
        ),
    )


def _bounds_from_points(points: Iterable[Sequence[float]]) -> Bounds:
    values = list(points)
    if not values:
        raise ValueError("cannot calculate bounds without points")
    return Bounds(
        tuple(min(point[index] for point in values) for index in range(3)),  # type: ignore[arg-type]
        tuple(max(point[index] for point in values) for index in range(3)),  # type: ignore[arg-type]
    )


def _local_bounds(obj: SceneObject) -> Bounds | None:
    if obj.type == "plane" or obj.type == "group":
        return None
    if obj.type in ("sphere", "cube"):
        return Bounds((-1.0, -1.0, -1.0), (1.0, 1.0, 1.0))
    if obj.type == "cylinder":
        return Bounds((-1.0, obj.minimum, -1.0), (1.0, obj.maximum, 1.0))
    if obj.type == "triangle":
        return _bounds_from_points((obj.p1, obj.p2, obj.p3))
    raise ValueError(f"unsupported object type for bounds: {obj.type}")


def _walk_objects(
    objects: Sequence[SceneObject],
    parent_matrix: Matrix4,
    parent_path: str,
) -> list[SceneLeaf]:
    leaves: list[SceneLeaf] = []
    for index, obj in enumerate(objects):
        path = f"{parent_path}/{index}:{obj.type}"
        world_matrix = _matrix_multiply(parent_matrix, _transform_matrix(obj.transform))
        if isinstance(obj, Group):
            leaves.extend(_walk_objects(obj.children, world_matrix, path))
            continue
        local = _local_bounds(obj)
        if local is None:
            continue
        world = _bounds_from_points(
            _transform_point(world_matrix, corner) for corner in local.corners()
        )
        leaves.append(SceneLeaf(path, obj.type, world, obj.material))
    return leaves


def collect_scene_leaves(scene: Scene) -> tuple[SceneLeaf, ...]:
    """Return finite renderable leaves with recursive world-space bounds."""

    leaves = _walk_objects(scene.objects, _identity(), "objects")
    if not leaves:
        return ()
    diagonals = [leaf.bounds.diagonal for leaf in leaves if leaf.bounds.diagonal > 0]
    typical = median(diagonals) if diagonals else 0.0
    marked: list[SceneLeaf] = []
    for leaf in leaves:
        extents = leaf.bounds.extents
        largest = max(extents)
        smallest = min(extents)
        is_thin = largest > 0 and smallest / largest <= 0.08
        is_dominant = typical > 0 and leaf.bounds.diagonal >= typical * 6.0
        backdrop = leaf.object_type == "cube" and is_thin and is_dominant
        marked.append(
            SceneLeaf(leaf.path, leaf.object_type, leaf.bounds, leaf.material, backdrop)
        )
    return tuple(marked)


def _walk_renderables(objects: Sequence[SceneObject]) -> Iterable[SceneObject]:
    for obj in objects:
        if isinstance(obj, Group):
            yield from _walk_renderables(obj.children)
        elif obj.type != "plane":
            yield obj


def _subtract(left: Sequence[float], right: Sequence[float]) -> Vector3:
    return tuple(left[index] - right[index] for index in range(3))  # type: ignore[return-value]


def _add(left: Sequence[float], right: Sequence[float]) -> Vector3:
    return tuple(left[index] + right[index] for index in range(3))  # type: ignore[return-value]


def _scale(vector: Sequence[float], value: float) -> Vector3:
    return tuple(component * value for component in vector)  # type: ignore[return-value]


def _dot(left: Sequence[float], right: Sequence[float]) -> float:
    return sum(left[index] * right[index] for index in range(3))


def _cross(left: Sequence[float], right: Sequence[float]) -> Vector3:
    return (
        left[1] * right[2] - left[2] * right[1],
        left[2] * right[0] - left[0] * right[2],
        left[0] * right[1] - left[1] * right[0],
    )


def _normalize(vector: Sequence[float]) -> Vector3:
    magnitude = math.sqrt(_dot(vector, vector))
    if magnitude <= 1e-9:
        raise ValueError("cannot normalize a zero-length vector")
    return tuple(component / magnitude for component in vector)  # type: ignore[return-value]


def _camera_basis(scene: Scene) -> tuple[Vector3, Vector3, Vector3]:
    forward = _normalize(_subtract(scene.camera.to, scene.camera.from_))
    try:
        horizontal = _normalize(_cross(forward, scene.camera.up))
    except ValueError:
        fallback = (0.0, 0.0, 1.0) if abs(forward[1]) > 0.9 else (0.0, 1.0, 0.0)
        horizontal = _normalize(_cross(forward, fallback))
    vertical = _normalize(_cross(horizontal, forward))
    return forward, horizontal, vertical


def _view_half_extents(scene: Scene) -> tuple[float, float]:
    half_view = math.tan(scene.camera.fov / 2.0)
    aspect = scene.camera.hsize / scene.camera.vsize
    if aspect >= 1.0:
        return half_view, half_view / aspect
    return half_view * aspect, half_view


def _project_leaf(scene: Scene, leaf: SceneLeaf) -> ObjectProjection:
    forward, horizontal, vertical = _camera_basis(scene)
    half_width, half_height = _view_half_extents(scene)
    projected: list[tuple[float, float]] = []
    depths: list[float] = []
    for corner in leaf.bounds.corners():
        relative = _subtract(corner, scene.camera.from_)
        depth = _dot(relative, forward)
        depths.append(depth)
        if depth > 1e-6:
            projected.append(
                (
                    _dot(relative, horizontal) / (depth * half_width),
                    _dot(relative, vertical) / (depth * half_height),
                )
            )
    center_depth = _dot(_subtract(leaf.bounds.center, scene.camera.from_), forward)
    if not projected:
        return ObjectProjection(
            leaf.path, leaf.object_type, None, 0.0, center_depth, leaf.backdrop
        )
    left = min(point[0] for point in projected)
    right = max(point[0] for point in projected)
    bottom = min(point[1] for point in projected)
    top = max(point[1] for point in projected)
    visible_width = max(0.0, min(1.0, right) - max(-1.0, left))
    visible_height = max(0.0, min(1.0, top) - max(-1.0, bottom))
    return ObjectProjection(
        leaf.path,
        leaf.object_type,
        (left, right, bottom, top),
        visible_width * visible_height / 4.0,
        center_depth,
        leaf.backdrop,
    )


def _rectangle_overlap(
    first: tuple[float, float, float, float],
    second: tuple[float, float, float, float],
) -> float:
    width = max(0.0, min(first[1], second[1]) - max(first[0], second[0]))
    height = max(0.0, min(first[3], second[3]) - max(first[2], second[2]))
    return width * height


def _material_issues(
    scene: Scene, leaf: SceneLeaf, thresholds: AuditThresholds
) -> list[AuditIssue]:
    issues: list[AuditIssue] = []
    material = leaf.material
    colors = [material.color]
    if material.pattern is not None:
        pattern = material.pattern
        while getattr(pattern, "type", None) == "perturbed":
            pattern = pattern.base
        if hasattr(pattern, "colorA"):
            colors.extend((pattern.colorA, pattern.colorB))
    peak_color = max(max(color) for color in colors)
    if material.ambient < 0.08 and peak_color < 0.25 and material.emissiveStrength < 0.1:
        issues.append(
            AuditIssue(
                "dark_material",
                "warning",
                "Dark, low-ambient material may lose its silhouette and surface detail.",
                leaf.path,
            )
        )
    if (
        scene.image.bloom
        and material.emissiveStrength > thresholds.excessive_emissive_strength
    ):
        issues.append(
            AuditIssue(
                "excessive_emission",
                "warning",
                "High emissive strength combined with bloom may clip highlights.",
                leaf.path,
            )
        )
    return issues


def audit_scene(
    scene: Scene, thresholds: AuditThresholds = AuditThresholds()
) -> SceneAuditReport:
    """Analyze structure, framing, visibility risks, and material readability."""

    leaves = collect_scene_leaves(scene)
    inventory: dict[str, int] = {}
    for leaf in leaves:
        inventory[leaf.object_type] = inventory.get(leaf.object_type, 0) + 1
    projections = tuple(_project_leaf(scene, leaf) for leaf in leaves)
    issues: list[AuditIssue] = []
    safe_limit = 1.0 - thresholds.safe_frame_margin
    content = [projection for projection in projections if not projection.backdrop]

    for leaf, projection in zip(leaves, projections):
        issues.extend(_material_issues(scene, leaf, thresholds))
        if projection.backdrop:
            continue
        if projection.ndc_bounds is None or projection.center_depth <= 0:
            issues.append(
                AuditIssue(
                    "behind_camera",
                    "error",
                    "Object is entirely behind the camera.",
                    projection.path,
                )
            )
            continue
        left, right, bottom, top = projection.ndc_bounds
        if right <= -1.0 or left >= 1.0 or top <= -1.0 or bottom >= 1.0:
            issues.append(
                AuditIssue(
                    "outside_frame",
                    "error",
                    "Object is entirely outside the camera frame.",
                    projection.path,
                )
            )
        elif (
            left < -safe_limit
            or right > safe_limit
            or bottom < -safe_limit
            or top > safe_limit
        ):
            issues.append(
                AuditIssue(
                    "unsafe_frame",
                    "warning",
                    "Object crosses the configured safe-frame margin.",
                    projection.path,
                )
            )
        if projection.screen_area < thresholds.minimum_object_area:
            issues.append(
                AuditIssue(
                    "too_small",
                    "warning",
                    "Object occupies too little of the frame to read reliably.",
                    projection.path,
                )
            )

    visible_rectangles = [
        projection.ndc_bounds
        for projection in content
        if projection.ndc_bounds is not None and projection.screen_area > 0
    ]
    scene_fill = 0.0
    if visible_rectangles:
        left = max(-1.0, min(rectangle[0] for rectangle in visible_rectangles))
        right = min(1.0, max(rectangle[1] for rectangle in visible_rectangles))
        bottom = max(-1.0, min(rectangle[2] for rectangle in visible_rectangles))
        top = min(1.0, max(rectangle[3] for rectangle in visible_rectangles))
        scene_fill = max(0.0, right - left) * max(0.0, top - bottom) / 4.0
    if content and scene_fill < thresholds.minimum_scene_fill:
        issues.append(
            AuditIssue(
                "scene_too_small",
                "warning",
                "Finite scene content occupies too little of the frame.",
            )
        )
    elif scene_fill > thresholds.maximum_scene_fill:
        issues.append(
            AuditIssue(
                "scene_crowded",
                "warning",
                "Scene content fills nearly the entire frame and may appear cramped.",
            )
        )

    for index, nearer in enumerate(content):
        if nearer.ndc_bounds is None or nearer.screen_area <= 0:
            continue
        for farther in content[index + 1 :]:
            if farther.ndc_bounds is None or farther.screen_area <= 0:
                continue
            front, back = (
                (nearer, farther)
                if nearer.center_depth <= farther.center_depth
                else (farther, nearer)
            )
            overlap = _rectangle_overlap(front.ndc_bounds, back.ndc_bounds)
            back_rectangle_area = max(
                1e-9,
                (back.ndc_bounds[1] - back.ndc_bounds[0])
                * (back.ndc_bounds[3] - back.ndc_bounds[2]),
            )
            if overlap / back_rectangle_area >= thresholds.occlusion_coverage:
                issues.append(
                    AuditIssue(
                        "possible_occlusion",
                        "warning",
                        f"Object may be substantially hidden behind {front.path}.",
                        back.path,
                    )
                )

    return SceneAuditReport(inventory, projections, tuple(issues), scene_fill)


def auto_frame_scene(scene: Scene, margin: float = 0.10) -> bool:
    """Fit finite subject bounds while preserving the generated viewing direction."""

    leaves = [leaf for leaf in collect_scene_leaves(scene) if not leaf.backdrop]
    if not leaves:
        return False
    subject_bounds = _bounds_from_points(
        corner for leaf in leaves for corner in leaf.bounds.corners()
    )
    center = subject_bounds.center
    forward, horizontal, vertical = _camera_basis(scene)
    half_width, half_height = _view_half_extents(scene)
    usable = max(0.1, 1.0 - margin)
    required_distance = 0.5
    for corner in subject_bounds.corners():
        relative = _subtract(corner, center)
        forward_offset = _dot(relative, forward)
        horizontal_offset = abs(_dot(relative, horizontal))
        vertical_offset = abs(_dot(relative, vertical))
        required_distance = max(
            required_distance,
            horizontal_offset / (half_width * usable) - forward_offset,
            vertical_offset / (half_height * usable) - forward_offset,
            0.05 - forward_offset,
        )
    required_distance *= 1.03
    new_from = _add(center, _scale(forward, -required_distance))
    changed = any(
        abs(scene.camera.from_[index] - new_from[index]) > 1e-6
        or abs(scene.camera.to[index] - center[index]) > 1e-6
        for index in range(3)
    )
    scene.camera.from_ = list(new_from)
    scene.camera.to = list(center)
    return changed


def repair_material_readability(
    scene: Scene, thresholds: AuditThresholds = AuditThresholds()
) -> tuple[int, int]:
    """Apply bounded corrections for materials the audit predicts will be unreadable."""

    material_adjustments = 0
    emission_adjustments = 0
    for obj in _walk_renderables(scene.objects):
        material = obj.material
        colors = [material.color]
        if material.pattern is not None:
            pattern = material.pattern
            while getattr(pattern, "type", None) == "perturbed":
                pattern = pattern.base
            if hasattr(pattern, "colorA"):
                colors.extend((pattern.colorA, pattern.colorB))
        peak_color = max(max(color) for color in colors)
        minimum_ambient = 0.0
        if peak_color < 0.25 and material.emissiveStrength < 0.1:
            minimum_ambient = 0.16
        elif material.transparency >= 0.5:
            minimum_ambient = 0.08
        elif material.reflective >= 0.75 and peak_color < 0.4:
            minimum_ambient = 0.12
        if material.ambient < minimum_ambient:
            material.ambient = minimum_ambient
            material_adjustments += 1
        if material.transparency >= 0.5 and material.specular < 0.6:
            material.specular = 0.6
            material_adjustments += 1
        if (
            scene.image.bloom
            and material.emissiveStrength > thresholds.excessive_emissive_strength
        ):
            material.emissiveStrength = thresholds.excessive_emissive_strength
            emission_adjustments += 1
    if emission_adjustments:
        scene.image.exposure = min(scene.image.exposure, 0.9)
    return material_adjustments, emission_adjustments


def _top_level_index(path: str | None) -> int | None:
    if path is None or path.count("/") != 1 or not path.startswith("objects/"):
        return None
    try:
        return int(path.split("/", 1)[1].split(":", 1)[0])
    except (ValueError, IndexError):
        return None


def repair_top_level_occlusions(scene: Scene, max_passes: int = 3) -> int:
    """Separate occluded free-standing objects while leaving recursive groups intact."""

    moved: set[int] = set()
    for pass_index in range(max_passes):
        report = audit_scene(scene)
        projections = {projection.path: projection for projection in report.projections}
        leaves = {leaf.path: leaf for leaf in collect_scene_leaves(scene)}
        candidates: list[tuple[str, str]] = []
        for issue in report.issues:
            if issue.code != "possible_occlusion" or issue.object_path is None:
                continue
            marker = "behind "
            if marker not in issue.message:
                continue
            front_path = issue.message.split(marker, 1)[1].rstrip(".")
            candidates.append((issue.object_path, front_path))
        if not candidates:
            break
        moved_this_pass = False
        adjusted_this_pass: set[int] = set()
        forward, horizontal, _vertical = _camera_basis(scene)
        del forward
        for back_path, front_path in candidates:
            back_index = _top_level_index(back_path)
            front_index = _top_level_index(front_path)
            if back_index is None or front_index is None or back_index == front_index:
                continue
            if back_index in adjusted_this_pass:
                continue
            obj = scene.objects[back_index]
            if obj.type not in {"sphere", "cube", "cylinder", "triangle"}:
                continue
            back_projection = projections.get(back_path)
            back_leaf = leaves.get(back_path)
            front_leaf = leaves.get(front_path)
            if (
                back_projection is None
                or back_projection.ndc_bounds is None
                or back_leaf is None
                or front_leaf is None
            ):
                continue
            center_x = (
                back_projection.ndc_bounds[0] + back_projection.ndc_bounds[1]
            ) / 2.0
            direction = 1.0 if center_x >= 0.05 else -1.0
            if abs(center_x) < 0.05:
                direction = 1.0 if (back_index + pass_index) % 2 == 0 else -1.0
            step = max(0.5, 0.35 * (back_leaf.bounds.diagonal + front_leaf.bounds.diagonal))
            obj.transform.translate = [
                obj.transform.translate[index] + horizontal[index] * direction * step
                for index in range(3)
            ]
            moved.add(back_index)
            adjusted_this_pass.add(back_index)
            moved_this_pass = True
        if not moved_this_pass:
            break
        auto_frame_scene(scene)
    return len(moved)


def repair_scene(
    scene: Scene,
    *,
    adjust_layout: bool = True,
    thresholds: AuditThresholds = AuditThresholds(),
) -> SceneRepairResult:
    """Apply conservative deterministic repairs and return an auditable summary."""

    material_adjustments, emission_adjustments = repair_material_readability(
        scene, thresholds
    )
    objects_repositioned = (
        repair_top_level_occlusions(scene) if adjust_layout else 0
    )
    report = audit_scene(scene, thresholds)
    camera_adjusted = False
    if adjust_layout and any(
        issue.code
        in {
            "behind_camera",
            "outside_frame",
            "unsafe_frame",
            "too_small",
            "scene_too_small",
            "scene_crowded",
        }
        for issue in report.issues
    ):
        camera_adjusted = auto_frame_scene(scene)
    return SceneRepairResult(
        material_adjustments,
        emission_adjustments,
        objects_repositioned,
        camera_adjusted,
    )
