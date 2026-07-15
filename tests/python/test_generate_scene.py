import json
import unittest

import httpx
from openai import BadRequestError, OpenAI

from tools.generate_scene import build_request_options


class GenerateSceneRequestTests(unittest.TestCase):
    def capture_request(self, *, strict_schema: bool) -> dict:
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
            "gpt-5.4-nano",
            "three planets",
            strict_schema=strict_schema,
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
        self.assertEqual(captured["text"]["verbosity"], "low")
        self.assertEqual(captured["text"]["format"]["type"], "json_object")

    def test_strict_mode_keeps_schema_and_nested_verbosity(self) -> None:
        captured = self.capture_request(strict_schema=True)

        self.assertNotIn("verbosity", captured)
        self.assertEqual(captured["text"]["verbosity"], "low")
        self.assertEqual(captured["text"]["format"]["type"], "json_schema")


if __name__ == "__main__":
    unittest.main()
