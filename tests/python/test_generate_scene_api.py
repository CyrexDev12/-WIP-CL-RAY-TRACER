from __future__ import annotations

import json
from pathlib import Path
import sys
from tempfile import TemporaryDirectory
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from tools.generate_scene import (
    DEFAULT_API_TIMEOUT,
    generate_scene_file,
    normalize_generated_scene_data,
)


SCENE = {
    "image": {
        "width": 200,
        "height": 100,
        "file": "generated.ppm",
        "multithreaded": True,
    },
    "camera": {
        "hsize": 200,
        "vsize": 100,
        "fov": 1.047,
        "from": [0, 1.5, -5],
        "to": [0, 1, 0],
        "up": [0, 1, 0],
    },
    "lights": [
        {"type": "point", "position": [-10, 10, -10], "color": [1, 1, 1]}
    ],
    "objects": [
        {
            "type": "sphere",
            "material": {"color": [0.8, 0.2, 0.2]},
        }
    ],
}


class FakeResponses:
    def create(self, **_options: object) -> SimpleNamespace:
        return SimpleNamespace(output_text=json.dumps(SCENE))


class FakeOpenAI:
    last_options: dict[str, object] | None = None

    def __init__(self, **options: object) -> None:
        FakeOpenAI.last_options = options
        self.responses = FakeResponses()


class GenerateSceneApiTests(unittest.TestCase):
    def test_normalizes_direct_pattern_transforms_and_legacy_type(self) -> None:
        normalized = normalize_generated_scene_data(
            {
                "type": "pertubed",
                "scale": [2, 2, 2],
                "rotate": [0, 1, 0],
                "base": {
                    "type": "checkers",
                    "translate": [1, 0, 0],
                    "colorA": [0, 0, 0],
                    "colorB": [1, 1, 1],
                },
            }
        )

        self.assertEqual(normalized["type"], "perturbed")
        self.assertEqual(normalized["transform"]["scale"], [2, 2, 2])
        self.assertEqual(normalized["transform"]["rotate"], [0, 1, 0])
        self.assertEqual(
            normalized["base"]["transform"]["translate"], [1, 0, 0]
        )

    def test_generates_scene_with_deterministic_ui_overrides(self) -> None:
        messages: list[str] = []
        fake_module = SimpleNamespace(OpenAI=FakeOpenAI)
        with TemporaryDirectory() as temporary_directory:
            output = Path(temporary_directory) / "scene.json"
            with patch.dict(sys.modules, {"openai": fake_module}):
                result = generate_scene_file(
                    "A red sphere",
                    output,
                    api_key="sk-test",
                    quality="high",
                    reasoning_effort="low",
                    multithreaded=False,
                    image_file="scene.ppm",
                    progress_callback=messages.append,
                )

            self.assertTrue(output.is_file())
            self.assertEqual(result.scene.image.width, 800)
            self.assertEqual(result.scene.image.height, 400)
            self.assertFalse(result.scene.image.multithreaded)
            self.assertEqual(result.scene.image.file, "scene.ppm")
            self.assertTrue(result.audit_path.is_file())
            audit_data = json.loads(result.audit_path.read_text(encoding="utf-8"))
            self.assertIn("final", audit_data)
            self.assertIn("inventory", audit_data["final"])
            self.assertEqual(audit_data["prompt"], "A red sphere")
            self.assertEqual(audit_data["software_version"], "1.1-dev")
            self.assertEqual(
                audit_data["generation_settings"]["reasoning_effort"], "low"
            )
            assert FakeOpenAI.last_options is not None
            self.assertEqual(FakeOpenAI.last_options["api_key"], "sk-test")
            self.assertEqual(FakeOpenAI.last_options["timeout"], DEFAULT_API_TIMEOUT)
            self.assertEqual(DEFAULT_API_TIMEOUT, 180.0)
            self.assertGreaterEqual(len(messages), 3)

    def test_rejects_empty_description_before_calling_openai(self) -> None:
        with TemporaryDirectory() as temporary_directory:
            with self.assertRaisesRegex(ValueError, "must not be empty"):
                generate_scene_file(
                    " ",
                    Path(temporary_directory) / "scene.json",
                    api_key="sk-test",
                )


if __name__ == "__main__":
    unittest.main()
