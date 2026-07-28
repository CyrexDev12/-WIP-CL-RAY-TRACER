"""Validated scene model matching src/SceneLoader.cpp."""

from __future__ import annotations

from typing import Annotated, Literal, Union

from pydantic import BaseModel, ConfigDict, Field, model_validator


Vector3 = Annotated[list[float], Field(min_length=3, max_length=3)]
Color3 = Annotated[
    list[Annotated[float, Field(ge=0.0, le=1.0)]],
    Field(min_length=3, max_length=3),
]


class StrictModel(BaseModel):
    model_config = ConfigDict(extra="forbid")


class ImageSettings(StrictModel):
    width: int = Field(ge=1, le=4096)
    height: int = Field(ge=1, le=4096)
    file: str = Field(pattern=r"^[^/\\]+\.ppm$")
    multithreaded: bool = True
    bloom: bool = False
    bloomIntensity: float = Field(default=0.35, ge=0.0, le=2.0)
    bloomThreshold: float = Field(default=1.0, ge=0.0, le=10.0)
    bloomRadius: int = Field(default=6, ge=1, le=32)
    toneMapping: bool = True
    exposure: float = Field(default=1.0, ge=0.1, le=5.0)
    gamma: float = Field(default=2.2, ge=1.0, le=3.0)


class CameraSettings(StrictModel):
    hsize: int = Field(ge=1, le=4096)
    vsize: int = Field(ge=1, le=4096)
    fov: float = Field(gt=0.0, lt=3.141592653589793)
    from_: Vector3 = Field(alias="from")
    to: Vector3
    up: Vector3

    @model_validator(mode="after")
    def validate_view(self) -> "CameraSettings":
        if self.from_ == self.to:
            raise ValueError("camera 'from' and 'to' must differ")
        if all(component == 0 for component in self.up):
            raise ValueError("camera 'up' must not be the zero vector")
        return self


class PointLight(StrictModel):
    type: Literal["point"] = "point"
    position: Vector3
    color: Color3


class Transform(StrictModel):
    scale: Vector3 = Field(default_factory=lambda: [1.0, 1.0, 1.0])
    rotate: Vector3 = Field(default_factory=lambda: [0.0, 0.0, 0.0])
    translate: Vector3 = Field(default_factory=lambda: [0.0, 0.0, 0.0])

    @model_validator(mode="after")
    def validate_scale(self) -> "Transform":
        if any(component == 0 for component in self.scale):
            raise ValueError("scale components must be non-zero")
        return self


class TwoColorPattern(StrictModel):
    colorA: Color3
    colorB: Color3
    mapping: Literal["object", "spherical"] = "object"
    transform: Transform = Field(default_factory=Transform)


class StripePattern(TwoColorPattern):
    type: Literal["stripe"] = "stripe"


class CheckersPattern(TwoColorPattern):
    type: Literal["checkers"] = "checkers"


class GradientPattern(TwoColorPattern):
    type: Literal["gradient"] = "gradient"


class RingPattern(TwoColorPattern):
    type: Literal["ring"] = "ring"


class PerturbedPattern(StrictModel):
    type: Literal["perturbed"] = "perturbed"
    base: "Pattern"
    distortionScale: float = Field(default=0.2, ge=0.0, le=2.0)
    noiseFrequency: float = Field(default=2.0, gt=0.0, le=100.0)
    mapping: Literal["object", "spherical"] = "object"
    transform: Transform = Field(default_factory=Transform)


Pattern = Union[
    StripePattern,
    CheckersPattern,
    GradientPattern,
    RingPattern,
    PerturbedPattern,
]


class Material(StrictModel):
    color: Color3 = Field(default_factory=lambda: [1.0, 1.0, 1.0])
    ambient: float = Field(default=0.1, ge=0.0, le=1.0)
    diffuse: float = Field(default=0.9, ge=0.0, le=1.0)
    specular: float = Field(default=0.9, ge=0.0, le=1.0)
    shininess: float = Field(default=200.0, ge=10.0, le=200.0)
    reflective: float = Field(default=0.0, ge=0.0, le=1.0)
    transparency: float = Field(default=0.0, ge=0.0, le=1.0)
    refractiveIndex: float = Field(default=1.0, ge=1.0, le=3.0)
    emissiveColor: Color3 = Field(default_factory=lambda: [0.0, 0.0, 0.0])
    emissiveStrength: float = Field(default=0.0, ge=0.0, le=20.0)
    pattern: Pattern | None = None


class Renderable(StrictModel):
    transform: Transform = Field(default_factory=Transform)
    material: Material = Field(default_factory=Material)


class Sphere(Renderable):
    type: Literal["sphere"] = "sphere"


class Plane(Renderable):
    type: Literal["plane"] = "plane"


class Cube(Renderable):
    type: Literal["cube"] = "cube"


class Cylinder(Renderable):
    type: Literal["cylinder"] = "cylinder"
    minimum: float = -1.0
    maximum: float = 1.0
    closed: bool = False

    @model_validator(mode="after")
    def validate_limits(self) -> "Cylinder":
        if self.minimum >= self.maximum:
            raise ValueError("cylinder minimum must be less than maximum")
        return self


class Triangle(Renderable):
    type: Literal["triangle"] = "triangle"
    p1: Vector3
    p2: Vector3
    p3: Vector3

    @model_validator(mode="after")
    def validate_points(self) -> "Triangle":
        edge1 = [self.p2[i] - self.p1[i] for i in range(3)]
        edge2 = [self.p3[i] - self.p1[i] for i in range(3)]
        cross = [
            edge1[1] * edge2[2] - edge1[2] * edge2[1],
            edge1[2] * edge2[0] - edge1[0] * edge2[2],
            edge1[0] * edge2[1] - edge1[1] * edge2[0],
        ]
        if all(component == 0 for component in cross):
            raise ValueError("triangle points must not be collinear")
        return self


class Group(StrictModel):
    type: Literal["group"] = "group"
    transform: Transform = Field(default_factory=Transform)
    children: list["SceneObject"] = Field(min_length=1, max_length=100)


SceneObject = Union[Sphere, Plane, Cube, Cylinder, Triangle, Group]


class Scene(StrictModel):
    image: ImageSettings
    camera: CameraSettings
    lights: list[PointLight] = Field(min_length=1, max_length=1)
    objects: list[SceneObject] = Field(min_length=1, max_length=100)

    @model_validator(mode="after")
    def synchronize_dimensions(self) -> "Scene":
        if (self.image.width, self.image.height) != (
            self.camera.hsize,
            self.camera.vsize,
        ):
            raise ValueError("image dimensions must match camera hsize/vsize")
        return self


PerturbedPattern.model_rebuild()
Group.model_rebuild()
