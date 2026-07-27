"""Background-friendly orchestration for scene generation and rendering."""

from __future__ import annotations

from dataclasses import dataclass, field
from datetime import datetime
import json
from pathlib import Path
import subprocess
import threading
from time import perf_counter
from typing import Callable
from uuid import uuid4

try:
    from .generate_scene import DEFAULT_MODEL, generate_scene_file
    from .PPMConverter import convert_ppm_to_png
except ImportError:
    from generate_scene import DEFAULT_MODEL, generate_scene_file
    from PPMConverter import convert_ppm_to_png


@dataclass(frozen=True)
class GenerationRequest:
    prompt: str
    api_key: str = field(repr=False)
    quality: str = "auto"
    reasoning_effort: str = "auto"
    multithreaded: bool = True
    model: str = DEFAULT_MODEL


@dataclass(frozen=True)
class GenerationEvent:
    stage: str
    message: str
    percent: int | None = None
    elapsed_seconds: float = 0.0
    eta_seconds: float | None = None


@dataclass(frozen=True)
class GenerationResult:
    png_path: Path
    scene_path: Path
    elapsed_seconds: float


class GenerationServiceError(RuntimeError):
    pass


class GenerationCancelled(GenerationServiceError):
    pass


def parse_progress_line(line: str) -> int | None:
    """Return a validated percentage from a renderer event line."""

    try:
        event = json.loads(line)
    except (json.JSONDecodeError, TypeError):
        return None
    if not isinstance(event, dict) or event.get("event") != "render_progress":
        return None
    percent = event.get("percent")
    if isinstance(percent, bool) or not isinstance(percent, int):
        return None
    return max(0, min(100, percent))


def estimate_remaining(elapsed_seconds: float, percent: int) -> float | None:
    if percent < 10 or percent >= 100 or elapsed_seconds < 0:
        return None
    return max(0.0, elapsed_seconds * (100 - percent) / percent)


class GenerationService:
    """Runs one generation job and exposes cancellation for its renderer process."""

    def __init__(self, project_root: Path | None = None) -> None:
        self.project_root = (
            Path(project_root).resolve()
            if project_root is not None
            else Path(__file__).resolve().parent.parent
        )
        self._cancelled = threading.Event()
        self._process_lock = threading.Lock()
        self._process: subprocess.Popen[str] | None = None

    def cancel(self) -> None:
        self._cancelled.set()
        with self._process_lock:
            process = self._process
        if process is not None and process.poll() is None:
            process.terminate()

    def run(
        self,
        request: GenerationRequest,
        event_callback: Callable[[GenerationEvent], None],
    ) -> GenerationResult:
        self._cancelled.clear()
        started_at = perf_counter()

        def emit(
            stage: str,
            message: str,
            percent: int | None = None,
            eta_seconds: float | None = None,
        ) -> None:
            event_callback(
                GenerationEvent(
                    stage=stage,
                    message=message,
                    percent=percent,
                    elapsed_seconds=perf_counter() - started_at,
                    eta_seconds=eta_seconds,
                )
            )

        executable = self.project_root / "raytracer.exe"
        if not executable.is_file():
            raise GenerationServiceError(
                f"Renderer not found at {executable}. Build it with mingw32-make first."
            )

        job_name = datetime.now().strftime("render-%Y%m%d-%H%M%S") + f"-{uuid4().hex[:6]}"
        scene_path = self.project_root / "scenes" / "generated" / f"{job_name}.json"
        job_directory = self.project_root / "build" / "jobs" / job_name
        png_path = self.project_root / "Renders" / f"{job_name}.png"
        job_directory.mkdir(parents=True, exist_ok=False)

        emit("scene", "Generating scene description with OpenAI...")
        generation = generate_scene_file(
            request.prompt,
            scene_path,
            api_key=request.api_key,
            model=request.model,
            reasoning_effort=request.reasoning_effort,
            quality=request.quality,
            force=False,
            multithreaded=request.multithreaded,
            image_file="scene.ppm",
            progress_callback=lambda message: emit("scene", message),
        )
        if self._cancelled.is_set():
            raise GenerationCancelled("Generation was cancelled.")

        render_started_at = perf_counter()
        emit(
            "render",
            f"Rendering {generation.scene.image.width}×{generation.scene.image.height} image...",
            0,
        )
        command = [
            str(executable),
            "--scene",
            str(scene_path),
            "--machine-progress",
        ]
        creation_flags = subprocess.CREATE_NO_WINDOW if hasattr(subprocess, "CREATE_NO_WINDOW") else 0
        process = subprocess.Popen(
            command,
            cwd=job_directory,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
            bufsize=1,
            creationflags=creation_flags,
        )
        with self._process_lock:
            self._process = process

        output_tail: list[str] = []
        smoothed_eta: float | None = None
        assert process.stdout is not None
        try:
            for raw_line in process.stdout:
                line = raw_line.strip()
                if line:
                    output_tail.append(line)
                    output_tail = output_tail[-20:]
                percent = parse_progress_line(line)
                if percent is None:
                    continue
                render_elapsed = perf_counter() - render_started_at
                eta = estimate_remaining(render_elapsed, percent)
                if eta is not None:
                    smoothed_eta = eta if smoothed_eta is None else (0.65 * smoothed_eta + 0.35 * eta)
                emit(
                    "render",
                    f"Rendering image: {percent}%",
                    percent,
                    smoothed_eta,
                )
            return_code = process.wait()
        finally:
            process.stdout.close()
            with self._process_lock:
                self._process = None

        if self._cancelled.is_set():
            raise GenerationCancelled("Generation was cancelled.")
        if return_code != 0:
            details = "\n".join(output_tail[-8:]) or "No renderer output was captured."
            raise GenerationServiceError(
                f"The renderer exited with code {return_code}.\n{details}"
            )

        ppm_path = job_directory / generation.scene.image.file
        if not ppm_path.is_file():
            raise GenerationServiceError(f"The renderer did not create {ppm_path}.")

        emit("convert", "Converting the render to PNG...", 100)
        try:
            convert_ppm_to_png(ppm_path, png_path)
        except Exception as exc:
            raise GenerationServiceError(f"PNG conversion failed: {exc}") from exc

        # The PPM is an app-owned intermediate; keep failed jobs for diagnostics.
        ppm_path.unlink()
        try:
            job_directory.rmdir()
        except OSError:
            pass

        elapsed = perf_counter() - started_at
        emit("complete", f"Saved render to {png_path}", 100, 0.0)
        return GenerationResult(png_path, scene_path, elapsed)
