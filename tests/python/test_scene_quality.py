from __future__ import annotations

import unittest

from tools.scene_quality import (
    audit_scene,
    auto_frame_scene,
    collect_scene_leaves,
    repair_scene,
)
from tools.scene_schema import Scene


def make_scene(objects: list[dict], *, bloom: bool = False) -> Scene:
    return Scene.model_validate(
        {
            "image": {
                "width": 800,
                "height": 400,
                "file": "quality-test.ppm",
                "multithreaded": True,
                "bloom": bloom,
            },
            "camera": {
                "hsize": 800,
                "vsize": 400,
                "fov": 1.0471975512,
                "from": [0, 1, -8],
                "to": [0, 1, 0],
                "up": [0, 1, 0],
            },
            "lights": [
                {
                    "type": "point",
                    "position": [-10, 10, -10],
                    "color": [1, 1, 1],
                }
            ],
            "objects": objects,
        }
    )


class SceneQualityTests(unittest.TestCase):
    def test_image_defaults_enable_tone_mapping(self) -> None:
        scene = make_scene([{"type": "sphere"}])

        self.assertTrue(scene.image.toneMapping)
        self.assertEqual(scene.image.exposure, 1.0)
        self.assertEqual(scene.image.gamma, 2.2)

    def test_schema_accepts_spherical_pattern_mapping(self) -> None:
        scene = make_scene(
            [
                {
                    "type": "sphere",
                    "material": {
                        "pattern": {
                            "type": "checkers",
                            "mapping": "spherical",
                            "colorA": [0, 0, 0],
                            "colorB": [1, 1, 1],
                            "transform": {"scale": [0.125, 0.25, 1]},
                        }
                    },
                }
            ]
        )

        self.assertEqual(scene.objects[0].material.pattern.mapping, "spherical")

    def test_recursive_group_bounds_include_parent_transform(self) -> None:
        scene = make_scene(
            [
                {
                    "type": "group",
                    "transform": {"scale": [2, 2, 2], "translate": [10, 0, 0]},
                    "children": [
                        {
                            "type": "sphere",
                            "transform": {"translate": [1, 0, 0]},
                        }
                    ],
                }
            ]
        )

        leaves = collect_scene_leaves(scene)

        self.assertEqual(len(leaves), 1)
        self.assertEqual(leaves[0].bounds.minimum, (10.0, -2.0, -2.0))
        self.assertEqual(leaves[0].bounds.maximum, (14.0, 2.0, 2.0))

    def test_audit_finds_object_outside_frame(self) -> None:
        scene = make_scene(
            [{"type": "sphere", "transform": {"translate": [20, 1, 0]}}]
        )

        report = audit_scene(scene)

        self.assertIn("outside_frame", {issue.code for issue in report.issues})

    def test_auto_frame_places_subject_inside_safe_margin(self) -> None:
        scene = make_scene(
            [{"type": "sphere", "transform": {"translate": [20, 4, 2]}}]
        )

        self.assertTrue(auto_frame_scene(scene))
        report = audit_scene(scene)
        framing_codes = {
            issue.code
            for issue in report.issues
            if issue.code in {"behind_camera", "outside_frame", "unsafe_frame"}
        }

        self.assertEqual(framing_codes, set())
        self.assertEqual(scene.camera.to, [20.0, 4.0, 2.0])

    def test_large_thin_cube_is_treated_as_backdrop(self) -> None:
        scene = make_scene(
            [
                {"type": "sphere", "transform": {"translate": [-2, 1, 0]}},
                {"type": "sphere", "transform": {"translate": [0, 1, 0]}},
                {"type": "sphere", "transform": {"translate": [2, 1, 0]}},
                {
                    "type": "cube",
                    "transform": {
                        "scale": [100, 50, 1],
                        "translate": [0, 20, 60],
                    },
                },
            ]
        )

        leaves = collect_scene_leaves(scene)

        self.assertFalse(any(leaf.backdrop for leaf in leaves[:3]))
        self.assertTrue(leaves[3].backdrop)
        auto_frame_scene(scene)
        self.assertLess(abs(scene.camera.to[0]), 0.01)
        self.assertLess(scene.camera.to[1], 3.0)

    def test_audit_warns_about_dark_material_and_excessive_emission(self) -> None:
        scene = make_scene(
            [
                {
                    "type": "sphere",
                    "material": {
                        "color": [0.05, 0.05, 0.05],
                        "ambient": 0.02,
                    },
                },
                {
                    "type": "sphere",
                    "transform": {"translate": [3, 1, 0]},
                    "material": {
                        "emissiveColor": [1, 0.3, 0],
                        "emissiveStrength": 12,
                    },
                },
            ],
            bloom=True,
        )

        codes = {issue.code for issue in audit_scene(scene).issues}

        self.assertIn("dark_material", codes)
        self.assertIn("excessive_emission", codes)

    def test_repair_improves_dark_material_and_caps_emission(self) -> None:
        scene = make_scene(
            [
                {
                    "type": "cube",
                    "material": {"color": [0.03, 0.03, 0.03], "ambient": 0.01},
                },
                {
                    "type": "sphere",
                    "transform": {"translate": [3, 1, 0]},
                    "material": {
                        "emissiveColor": [1, 0.3, 0],
                        "emissiveStrength": 15,
                    },
                },
            ],
            bloom=True,
        )

        result = repair_scene(scene, adjust_layout=False)

        self.assertEqual(result.material_adjustments, 1)
        self.assertEqual(result.emission_adjustments, 1)
        self.assertGreaterEqual(scene.objects[0].material.ambient, 0.16)
        self.assertEqual(scene.objects[1].material.emissiveStrength, 6.0)
        self.assertEqual(scene.image.exposure, 0.9)

    def test_repair_separates_occluded_top_level_objects(self) -> None:
        scene = make_scene(
            [
                {"type": "sphere", "transform": {"translate": [0, 1, 0]}},
                {"type": "sphere", "transform": {"translate": [0, 1, 2]}},
            ]
        )
        before = sum(
            issue.code == "possible_occlusion" for issue in audit_scene(scene).issues
        )

        result = repair_scene(scene)
        after = sum(
            issue.code == "possible_occlusion" for issue in audit_scene(scene).issues
        )

        self.assertGreater(before, 0)
        self.assertEqual(result.objects_repositioned, 1)
        self.assertLess(after, before)


if __name__ == "__main__":
    unittest.main()
