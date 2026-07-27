"""Secure API-key persistence for the desktop interface."""

from __future__ import annotations

import os
from typing import Any


SERVICE_NAME = "RayTracerAIPromptSceneGenerator"
API_KEY_ACCOUNT = "openai-api-key"


class SettingsStoreError(RuntimeError):
    """Raised when the operating-system credential store is unavailable."""


def _keyring() -> Any:
    try:
        import keyring
    except ImportError as exc:
        raise SettingsStoreError(
            "Secure key storage is unavailable. Install the project requirements first."
        ) from exc
    return keyring


def get_saved_api_key() -> str | None:
    try:
        value = _keyring().get_password(SERVICE_NAME, API_KEY_ACCOUNT)
    except Exception as exc:
        raise SettingsStoreError(f"Could not read the saved API key: {exc}") from exc
    return value.strip() if value and value.strip() else None


def get_api_key() -> tuple[str | None, str | None]:
    """Return the configured API key and its source name."""

    environment_key = os.environ.get("OPENAI_API_KEY", "").strip()
    if environment_key:
        return environment_key, "OPENAI_API_KEY environment variable"
    saved_key = get_saved_api_key()
    if saved_key:
        return saved_key, "secure credential store"
    return None, None


def save_api_key(api_key: str) -> None:
    value = api_key.strip()
    if not value:
        raise ValueError("Enter an API key before saving.")
    try:
        _keyring().set_password(SERVICE_NAME, API_KEY_ACCOUNT, value)
    except Exception as exc:
        raise SettingsStoreError(f"Could not save the API key securely: {exc}") from exc


def delete_saved_api_key() -> None:
    keyring = _keyring()
    try:
        if keyring.get_password(SERVICE_NAME, API_KEY_ACCOUNT) is not None:
            keyring.delete_password(SERVICE_NAME, API_KEY_ACCOUNT)
    except Exception as exc:
        raise SettingsStoreError(f"Could not remove the saved API key: {exc}") from exc
