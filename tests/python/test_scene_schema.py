import json
import unittest
from pathlib import Path

from pydantic import ValidationError

from tools.scene_schema import Scene


ROOT = Path(__file__).resolve().parents[2]


class SceneSchemaTests(unittest.TestCase):
    def test_schema_uses_structured_outputs_supported_composition(self) -> None:
        schema = Scene.model_json_schema()
        encoded = json.dumps(schema)
        self.assertNotIn('"oneOf"', encoded)
        self.assertIn('"anyOf"', encoded)
        self.assertEqual(schema["type"], "object")

    def test_extended_fixture_validates(self) -> None:
        data = json.loads(
            (ROOT / "tests" / "fixtures" / "extended_scene.json").read_text()
        )
        scene = Scene.model_validate(data)
        self.assertEqual(len(scene.lights), 2)
        self.assertTrue(scene.image.bloom)
        self.assertEqual([obj.type for obj in scene.objects], [
            "plane", "cube", "cylinder", "triangle", "group"
        ])
        emissive = scene.objects[4].children[0].material
        self.assertEqual(emissive.emissiveColor, [0.2, 0.6, 1.0])
        self.assertEqual(emissive.emissiveStrength, 4.0)

    def test_emission_uses_hdr_strength_not_over_range_albedo(self) -> None:
        data = json.loads(
            (ROOT / "tests" / "fixtures" / "extended_scene.json").read_text()
        )
        sphere = data["objects"][4]["children"][0]
        sphere["material"]["emissiveStrength"] = 12.0
        scene = Scene.model_validate(data)
        self.assertEqual(
            scene.objects[4].children[0].material.emissiveStrength, 12.0
        )

        sphere["material"]["color"] = [2.0, 0.5, 0.5]
        with self.assertRaises(ValidationError):
            Scene.model_validate(data)

    def test_rejects_more_than_four_lights(self) -> None:
        data = json.loads(
            (ROOT / "tests" / "fixtures" / "extended_scene.json").read_text()
        )
        data["lights"] = data["lights"] * 3
        with self.assertRaises(ValidationError):
            Scene.model_validate(data)

    def test_rejects_degenerate_triangle(self) -> None:
        data = json.loads(
            (ROOT / "tests" / "fixtures" / "extended_scene.json").read_text()
        )
        triangle = data["objects"][3]
        triangle["p1"] = [0, 0, 0]
        triangle["p2"] = [1, 1, 1]
        triangle["p3"] = [2, 2, 2]
        with self.assertRaises(ValidationError):
            Scene.model_validate(data)

    def test_rejects_invalid_cylinder_limits(self) -> None:
        data = json.loads(
            (ROOT / "tests" / "fixtures" / "extended_scene.json").read_text()
        )
        cylinder = data["objects"][2]
        cylinder["minimum"] = 2
        cylinder["maximum"] = 2
        with self.assertRaises(ValidationError):
            Scene.model_validate(data)


if __name__ == "__main__":
    unittest.main()
