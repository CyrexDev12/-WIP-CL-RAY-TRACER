"""Validated scene model matching src/SceneLoader.cpp."""

from __future__ import annotations

from typing import Annotated, Literal

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
    translate: Vector3 = Field(default_factory=lambda: [0.0, 0.0, 0.0])

    @model_validator(mode="after")
    def validate_scale(self) -> "Transform":
        if any(component == 0 for component in self.scale):
            raise ValueError("scale components must be non-zero")
        return self


class Material(StrictModel):
    color: Color3
    ambient: float = Field(default=0.1, ge=0.0, le=1.0)
    diffuse: float = Field(default=0.9, ge=0.0, le=1.0)
    specular: float = Field(default=0.9, ge=0.0, le=1.0)
    shininess: float = Field(default=200.0, ge=10.0, le=200.0)
    reflective: float = Field(default=0.0, ge=0.0, le=1.0)
    transparency: float = Field(default=0.0, ge=0.0, le=1.0)
    refractiveIndex: float = Field(default=1.0, ge=1.0, le=3.0)


class Sphere(StrictModel):
    type: Literal["sphere"] = "sphere"
    transform: Transform = Field(default_factory=Transform)
    material: Material


class Scene(StrictModel):
    image: ImageSettings
    camera: CameraSettings
    lights: list[PointLight] = Field(min_length=1, max_length=1)
    objects: list[Sphere] = Field(min_length=1, max_length=100)

    @model_validator(mode="after")
    def synchronize_dimensions(self) -> "Scene":
        if (self.image.width, self.image.height) != (
            self.camera.hsize,
            self.camera.vsize,
        ):
            raise ValueError("image dimensions must match camera hsize/vsize")
        return self
