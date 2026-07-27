from __future__ import annotations

import os
import unittest
from unittest.mock import patch

from tools.app import format_duration, format_elapsed
from tools.generation_service import (
    GenerationRequest,
    estimate_remaining,
    parse_progress_line,
)
from tools import settings_store


class ProgressTests(unittest.TestCase):
    def test_parses_renderer_progress_event(self) -> None:
        self.assertEqual(
            parse_progress_line('{"event":"render_progress","percent":35}'), 35
        )

    def test_ignores_unrelated_or_invalid_renderer_output(self) -> None:
        self.assertIsNone(parse_progress_line("[DEBUG] Rendering..."))
        self.assertIsNone(parse_progress_line('{"event":"other","percent":20}'))
        self.assertIsNone(
            parse_progress_line('{"event":"render_progress","percent":true}')
        )

    def test_progress_is_clamped(self) -> None:
        self.assertEqual(
            parse_progress_line('{"event":"render_progress","percent":150}'), 100
        )

    def test_eta_waits_for_stable_progress(self) -> None:
        self.assertIsNone(estimate_remaining(4.0, 5))
        self.assertEqual(estimate_remaining(10.0, 25), 30.0)
        self.assertIsNone(estimate_remaining(10.0, 100))

    def test_time_formatting(self) -> None:
        self.assertEqual(format_elapsed(65), "1m 05s")
        self.assertEqual(format_duration(45), "About 45 seconds remaining")
        self.assertEqual(format_duration(125), "About 2m 05s remaining")

    def test_api_key_is_hidden_from_request_representation(self) -> None:
        request = GenerationRequest(prompt="A sphere", api_key="sk-secret")
        self.assertNotIn("sk-secret", repr(request))


class FakeKeyring:
    def __init__(self) -> None:
        self.value: str | None = None

    def get_password(self, service: str, account: str) -> str | None:
        self._assert_names(service, account)
        return self.value

    def set_password(self, service: str, account: str, value: str) -> None:
        self._assert_names(service, account)
        self.value = value

    def delete_password(self, service: str, account: str) -> None:
        self._assert_names(service, account)
        self.value = None

    def _assert_names(self, service: str, account: str) -> None:
        assert service == settings_store.SERVICE_NAME
        assert account == settings_store.API_KEY_ACCOUNT


class SettingsStoreTests(unittest.TestCase):
    def setUp(self) -> None:
        self.keyring = FakeKeyring()
        self.keyring_patch = patch.object(
            settings_store, "_keyring", return_value=self.keyring
        )
        self.keyring_patch.start()
        self.addCleanup(self.keyring_patch.stop)

    def test_saves_reads_and_deletes_key(self) -> None:
        settings_store.save_api_key("  sk-test  ")
        self.assertEqual(settings_store.get_saved_api_key(), "sk-test")
        settings_store.delete_saved_api_key()
        self.assertIsNone(settings_store.get_saved_api_key())

    def test_environment_key_takes_priority(self) -> None:
        self.keyring.value = "saved-key"
        with patch.dict(os.environ, {"OPENAI_API_KEY": "environment-key"}):
            key, source = settings_store.get_api_key()
        self.assertEqual(key, "environment-key")
        self.assertIn("environment", source or "")

    def test_rejects_empty_key(self) -> None:
        with self.assertRaises(ValueError):
            settings_store.save_api_key("  ")


if __name__ == "__main__":
    unittest.main()
