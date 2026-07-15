import json
import unittest

import httpx
from openai import BadRequestError, OpenAI

from tools.generate_scene import apply_quality_preset, build_request_options
from tools.scene_schema import Scene


class GenerateSceneRequestTests(unittest.TestCase):
    def make_preview_scene(self) -> Scene:
        return Scene.model_validate(
            {
                "image": {
                    "width": 200,
                    "height": 100,
                    "file": "quality.ppm",
                    "multithreaded": True,
                },
                "camera": {
                    "hsize": 200,
                    "vsize": 100,
                    "fov": 1.0472,
                    "from": [0, 2, -5],
                    "to": [0, 1, 0],
                    "up": [0, 1, 0],
                },
                "lights": [
                    {
                        "type": "point",
                        "position": [-5, 8, -5],
                        "color": [1, 1, 1],
                    }
                ],
                "objects": [{"type": "sphere"}],
            }
        )

    def capture_request(
        self,
        *,
        strict_schema: bool,
        model: str = "gpt-5.4-nano",
        reasoning_effort: str = "auto",
    ) -> dict:
        captured: dict = {}

        def capture_request(request: httpx.Request) -> httpx.Response:
            captured.update(json.loads(request.content))
            return httpx.Response(
                400,
                json={"error": {"message": "captured", "type": "test"}},
                request=request,
            )

        client = OpenAI(
            api_key="test",
            http_client=httpx.Client(transport=httpx.MockTransport(capture_request)),
            max_retries=0,
        )

        options = build_request_options(
            model,
            "three planets",
            strict_schema=strict_schema,
            reasoning_effort=reasoning_effort,
        )
        with self.assertRaises(BadRequestError):
            if strict_schema:
                client.responses.parse(**options)
            else:
                client.responses.create(**options)

        return captured

    def test_fast_mode_requests_json_and_nested_verbosity(self) -> None:
        captured = self.capture_request(strict_schema=False)

        self.assertNotIn("verbosity", captured)
        self.assertEqual(captured["reasoning"]["effort"], "none")
        self.assertEqual(captured["text"]["verbosity"], "low")
        self.assertEqual(captured["text"]["format"]["type"], "json_object")

    def test_mini_uses_quality_biased_defaults(self) -> None:
        captured = self.capture_request(
            strict_schema=False,
            model="gpt-5.4-mini",
        )

        self.assertEqual(captured["reasoning"]["effort"], "low")
        self.assertEqual(captured["text"]["verbosity"], "medium")

    def test_reasoning_effort_can_be_overridden(self) -> None:
        captured = self.capture_request(
            strict_schema=False,
            model="gpt-5.4-mini",
            reasoning_effort="medium",
        )

        self.assertEqual(captured["reasoning"]["effort"], "medium")

    def test_high_quality_prompt_upscales_image_and_camera(self) -> None:
        scene = self.make_preview_scene()

        applied = apply_quality_preset(scene, "make a high quality render")

        self.assertEqual(applied, "high")
        self.assertEqual((scene.image.width, scene.image.height), (800, 400))
        self.assertEqual((scene.camera.hsize, scene.camera.vsize), (800, 400))

    def test_explicit_resolution_wins_over_quality_words(self) -> None:
        scene = self.make_preview_scene()

        applied = apply_quality_preset(
            scene, "make a high quality render at 1200x600"
        )

        self.assertEqual(applied, "custom")
        self.assertEqual((scene.image.width, scene.image.height), (1200, 600))
        self.assertEqual((scene.camera.hsize, scene.camera.vsize), (1200, 600))

    def test_explicit_quality_option_overrides_generated_resolution(self) -> None:
        scene = self.make_preview_scene()

        applied = apply_quality_preset(scene, "a quick draft", "ultra")

        self.assertEqual(applied, "ultra")
        self.assertEqual((scene.image.width, scene.image.height), (1600, 800))

    def test_strict_mode_keeps_schema_and_nested_verbosity(self) -> None:
        captured = self.capture_request(strict_schema=True)

        self.assertNotIn("verbosity", captured)
        self.assertEqual(captured["text"]["verbosity"], "low")
        self.assertEqual(captured["text"]["format"]["type"], "json_schema")


if __name__ == "__main__":
    unittest.main()
