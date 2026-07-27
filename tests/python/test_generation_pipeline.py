from __future__ import annotations

import json
from pathlib import Path
import shutil
from tempfile import TemporaryDirectory
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from PIL import Image

from tools.generation_service import (
    GenerationEvent,
    GenerationRequest,
    GenerationService,
)
from tools.scene_schema import Scene


SCENE = {
    "image": {
        "width": 40,
        "height": 20,
        "file": "scene.ppm",
        "multithreaded": True,
    },
    "camera": {
        "hsize": 40,
        "vsize": 20,
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
            "transform": {"translate": [0, 1, 0]},
            "material": {"color": [0.8, 0.2, 0.2]},
        }
    ],
}


class GenerationPipelineTests(unittest.TestCase):
    def test_local_render_pipeline_creates_and_reports_png(self) -> None:
        repository_root = Path(__file__).resolve().parents[2]
        executable = repository_root / "raytracer.exe"
        self.assertTrue(executable.is_file(), "Build raytracer.exe before running tests")

        with TemporaryDirectory() as temporary_directory:
            project_root = Path(temporary_directory)
            shutil.copy2(executable, project_root / "raytracer.exe")
            events: list[GenerationEvent] = []

            def fake_generate(_prompt: str, output: Path, **_options: object):
                scene = Scene.model_validate(SCENE)
                output.parent.mkdir(parents=True, exist_ok=True)
                output.write_text(
                    json.dumps(scene.model_dump(by_alias=True, exclude_none=True)),
                    encoding="utf-8",
                )
                return SimpleNamespace(scene=scene)

            service = GenerationService(project_root)
            request = GenerationRequest(prompt="A red sphere", api_key="sk-test")
            with patch(
                "tools.generation_service.generate_scene_file", side_effect=fake_generate
            ):
                result = service.run(request, events.append)

            self.assertTrue(result.png_path.is_file())
            with Image.open(result.png_path) as image:
                self.assertEqual(image.format, "PNG")
                self.assertEqual(image.size, (40, 20))
            render_percentages = [
                event.percent
                for event in events
                if event.stage == "render" and event.percent is not None
            ]
            self.assertIn(100, render_percentages)
            self.assertEqual(events[-1].stage, "complete")


if __name__ == "__main__":
    unittest.main()
