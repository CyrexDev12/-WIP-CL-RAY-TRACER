"""Desktop interface for the AI prompt scene generator."""

from __future__ import annotations

import os
from pathlib import Path
import platform
import queue
import subprocess
import threading
from time import perf_counter
import tkinter as tk
from tkinter import messagebox, ttk

try:
    from .generation_service import (
        GenerationCancelled,
        GenerationEvent,
        GenerationRequest,
        GenerationResult,
        GenerationService,
    )
    from .settings_store import (
        SettingsStoreError,
        delete_saved_api_key,
        get_api_key,
        save_api_key,
    )
except ImportError:
    from generation_service import (
        GenerationCancelled,
        GenerationEvent,
        GenerationRequest,
        GenerationResult,
        GenerationService,
    )
    from settings_store import (
        SettingsStoreError,
        delete_saved_api_key,
        get_api_key,
        save_api_key,
    )


QUALITY_OPTIONS = {
    "Auto": "auto",
    "Preview (200 px)": "preview",
    "Standard (400 px)": "standard",
    "High (800 px)": "high",
    "Ultra (1600 px)": "ultra",
}
REASONING_OPTIONS = {
    "Auto": "auto",
    "None": "none",
    "Low": "low",
    "Medium": "medium",
    "High": "high",
    "XHigh": "xhigh",
}


def format_duration(seconds: float | None) -> str:
    if seconds is None:
        return "Estimating time remaining…"
    seconds = max(0, round(seconds))
    if seconds < 60:
        return f"About {seconds} second{'s' if seconds != 1 else ''} remaining"
    minutes, remainder = divmod(seconds, 60)
    if minutes < 60:
        return f"About {minutes}m {remainder:02d}s remaining"
    hours, minutes = divmod(minutes, 60)
    return f"About {hours}h {minutes:02d}m remaining"


class RayTracerApp:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.root.title("Ray Tracer AI Scene Generator")
        self.root.geometry("1000x720")
        self.root.minsize(760, 560)
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)

        self.service = GenerationService()
        self.events: queue.Queue[tuple[str, object]] = queue.Queue()
        self.worker: threading.Thread | None = None
        self.job_started_at: float | None = None
        self.current_eta: float | None = None
        self.saved_png: Path | None = None
        self.source_image = None
        self.preview_image = None
        self._resize_after_id: str | None = None

        self._configure_styles()
        self._build_interface()
        self._refresh_key_status()
        self.root.after(100, self._poll_events)

    def _configure_styles(self) -> None:
        style = ttk.Style(self.root)
        if "vista" in style.theme_names():
            style.theme_use("vista")
        style.configure("Title.TLabel", font=("Segoe UI", 20, "bold"))
        style.configure("Heading.TLabel", font=("Segoe UI", 13, "bold"))
        style.configure("Muted.TLabel", foreground="#555555")
        style.configure("Error.TLabel", foreground="#a61b1b")
        style.configure("Success.TLabel", foreground="#1f6b32")

    def _build_interface(self) -> None:
        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill="both", expand=True, padx=14, pady=14)

        self.main_tab = ttk.Frame(self.notebook, padding=24)
        self.settings_tab = ttk.Frame(self.notebook, padding=24)
        self.generation_tab = ttk.Frame(self.notebook, padding=24)
        self.notebook.add(self.main_tab, text="Main")
        self.notebook.add(self.settings_tab, text="Settings")
        self.notebook.add(self.generation_tab, text="Generation")

        self._build_main_tab()
        self._build_settings_tab()
        self._build_generation_tab()

    def _build_main_tab(self) -> None:
        self.main_tab.columnconfigure(0, weight=1)
        self.main_tab.rowconfigure(2, weight=1)

        ttk.Label(
            self.main_tab,
            text="Create a ray-traced scene",
            style="Title.TLabel",
        ).grid(row=0, column=0, sticky="w")
        ttk.Label(
            self.main_tab,
            text=(
                "Describe the objects, colors, lighting, camera angle, materials, and mood you want. "
                "The app will generate a validated scene, render it, and save a PNG in the Renders folder."
            ),
            wraplength=850,
            justify="left",
            style="Muted.TLabel",
        ).grid(row=1, column=0, sticky="ew", pady=(8, 18))

        prompt_frame = ttk.LabelFrame(self.main_tab, text="Scene prompt", padding=10)
        prompt_frame.grid(row=2, column=0, sticky="nsew")
        prompt_frame.columnconfigure(0, weight=1)
        prompt_frame.rowconfigure(0, weight=1)
        self.prompt_text = tk.Text(
            prompt_frame,
            wrap="word",
            font=("Segoe UI", 11),
            padx=10,
            pady=10,
            undo=True,
        )
        self.prompt_text.grid(row=0, column=0, sticky="nsew")
        prompt_scrollbar = ttk.Scrollbar(
            prompt_frame, orient="vertical", command=self.prompt_text.yview
        )
        prompt_scrollbar.grid(row=0, column=1, sticky="ns")
        self.prompt_text.configure(yscrollcommand=prompt_scrollbar.set)

        options = ttk.Frame(self.main_tab)
        options.grid(row=3, column=0, sticky="ew", pady=(18, 0))
        options.columnconfigure(1, weight=1)
        options.columnconfigure(3, weight=1)

        ttk.Label(options, text="Quality").grid(row=0, column=0, sticky="w", padx=(0, 8))
        self.quality_var = tk.StringVar(value="Auto")
        self.quality_box = ttk.Combobox(
            options,
            textvariable=self.quality_var,
            values=tuple(QUALITY_OPTIONS),
            state="readonly",
            width=22,
        )
        self.quality_box.grid(row=0, column=1, sticky="ew", padx=(0, 24))

        ttk.Label(options, text="Reasoning effort").grid(
            row=0, column=2, sticky="w", padx=(0, 8)
        )
        self.reasoning_var = tk.StringVar(value="Auto")
        self.reasoning_box = ttk.Combobox(
            options,
            textvariable=self.reasoning_var,
            values=tuple(REASONING_OPTIONS),
            state="readonly",
            width=16,
        )
        self.reasoning_box.grid(row=0, column=3, sticky="ew", padx=(0, 24))

        self.multithreaded_var = tk.BooleanVar(value=True)
        self.multithreaded_check = ttk.Checkbutton(
            options,
            text="Use multithreaded rendering",
            variable=self.multithreaded_var,
        )
        self.multithreaded_check.grid(row=0, column=4, sticky="w")

        footer = ttk.Frame(self.main_tab)
        footer.grid(row=4, column=0, sticky="ew", pady=(18, 0))
        footer.columnconfigure(0, weight=1)
        self.main_status_var = tk.StringVar()
        self.main_status = ttk.Label(
            footer, textvariable=self.main_status_var, style="Error.TLabel"
        )
        self.main_status.grid(row=0, column=0, sticky="w")
        self.generate_button = ttk.Button(
            footer, text="Generate", command=self._start_generation
        )
        self.generate_button.grid(row=0, column=1, sticky="e")

    def _build_settings_tab(self) -> None:
        self.settings_tab.columnconfigure(0, weight=1)
        ttk.Label(
            self.settings_tab,
            text="OpenAI settings",
            style="Title.TLabel",
        ).grid(row=0, column=0, sticky="w")
        ttk.Label(
            self.settings_tab,
            text=(
                "Your API key is stored in the operating system's secure credential store. "
                "It is never written to this project or included in renderer commands."
            ),
            wraplength=800,
            justify="left",
            style="Muted.TLabel",
        ).grid(row=1, column=0, sticky="ew", pady=(8, 24))

        key_frame = ttk.LabelFrame(self.settings_tab, text="OpenAI API key", padding=14)
        key_frame.grid(row=2, column=0, sticky="ew")
        key_frame.columnconfigure(0, weight=1)
        self.api_key_var = tk.StringVar()
        self.api_key_entry = ttk.Entry(
            key_frame, textvariable=self.api_key_var, show="*", font=("Segoe UI", 11)
        )
        self.api_key_entry.grid(row=0, column=0, columnspan=2, sticky="ew")
        self.save_key_button = ttk.Button(
            key_frame, text="Save key", command=self._save_key
        )
        self.save_key_button.grid(row=1, column=0, sticky="w", pady=(12, 0))
        self.clear_key_button = ttk.Button(
            key_frame, text="Clear saved key", command=self._clear_key
        )
        self.clear_key_button.grid(row=1, column=1, sticky="e", pady=(12, 0))

        self.key_status_var = tk.StringVar()
        self.key_status = ttk.Label(
            self.settings_tab,
            textvariable=self.key_status_var,
            wraplength=800,
            justify="left",
        )
        self.key_status.grid(row=3, column=0, sticky="ew", pady=(14, 0))

    def _build_generation_tab(self) -> None:
        self.generation_tab.columnconfigure(0, weight=1)
        self.generation_tab.rowconfigure(4, weight=1)

        self.stage_var = tk.StringVar(value="Ready to generate")
        ttk.Label(
            self.generation_tab,
            textvariable=self.stage_var,
            style="Title.TLabel",
        ).grid(row=0, column=0, sticky="w")

        self.generation_status_var = tk.StringVar(
            value="Complete the Main tab and select Generate to begin."
        )
        self.generation_status = ttk.Label(
            self.generation_tab,
            textvariable=self.generation_status_var,
            wraplength=860,
            justify="left",
        )
        self.generation_status.grid(row=1, column=0, sticky="ew", pady=(8, 14))

        progress_frame = ttk.Frame(self.generation_tab)
        progress_frame.grid(row=2, column=0, sticky="ew")
        progress_frame.columnconfigure(0, weight=1)
        self.progress = ttk.Progressbar(progress_frame, maximum=100, mode="determinate")
        self.progress.grid(row=0, column=0, sticky="ew")
        self.percent_var = tk.StringVar(value="")
        ttk.Label(progress_frame, textvariable=self.percent_var, width=6, anchor="e").grid(
            row=0, column=1, padx=(10, 0)
        )

        self.time_var = tk.StringVar()
        ttk.Label(
            self.generation_tab, textvariable=self.time_var, style="Muted.TLabel"
        ).grid(row=3, column=0, sticky="w", pady=(8, 12))

        self.preview_frame = ttk.Frame(self.generation_tab)
        self.preview_frame.grid(row=4, column=0, sticky="nsew")
        self.preview_frame.columnconfigure(0, weight=1)
        self.preview_frame.rowconfigure(0, weight=1)
        self.preview_label = ttk.Label(
            self.preview_frame,
            text="Your completed render will appear here.",
            anchor="center",
            style="Muted.TLabel",
        )
        self.preview_label.grid(row=0, column=0, sticky="nsew")
        self.preview_frame.bind("<Configure>", self._schedule_preview_resize)

        result_footer = ttk.Frame(self.generation_tab)
        result_footer.grid(row=5, column=0, sticky="ew", pady=(12, 0))
        result_footer.columnconfigure(0, weight=1)
        self.saved_path_var = tk.StringVar()
        ttk.Label(
            result_footer,
            textvariable=self.saved_path_var,
            wraplength=700,
            justify="left",
            style="Success.TLabel",
        ).grid(row=0, column=0, sticky="w")
        self.open_folder_button = ttk.Button(
            result_footer,
            text="Open Renders folder",
            command=self._open_renders_folder,
            state="disabled",
        )
        self.open_folder_button.grid(row=0, column=1, sticky="e", padx=(12, 0))

    def _refresh_key_status(self) -> None:
        try:
            key, source = get_api_key()
        except SettingsStoreError as exc:
            self.key_status.configure(style="Error.TLabel")
            self.key_status_var.set(str(exc))
            return
        if key:
            self.key_status.configure(style="Success.TLabel")
            self.key_status_var.set(f"API key configured via {source}.")
        else:
            self.key_status.configure(style="Muted.TLabel")
            self.key_status_var.set("No API key is currently configured.")

    def _save_key(self) -> None:
        try:
            save_api_key(self.api_key_var.get())
        except (ValueError, SettingsStoreError) as exc:
            self.key_status.configure(style="Error.TLabel")
            self.key_status_var.set(str(exc))
            return
        self.api_key_var.set("")
        self.key_status.configure(style="Success.TLabel")
        self.key_status_var.set("API key saved securely.")
        self.main_status_var.set("")

    def _clear_key(self) -> None:
        try:
            delete_saved_api_key()
        except SettingsStoreError as exc:
            self.key_status.configure(style="Error.TLabel")
            self.key_status_var.set(str(exc))
            return
        self.api_key_var.set("")
        self._refresh_key_status()

    def _start_generation(self) -> None:
        if self.worker is not None and self.worker.is_alive():
            return
        prompt = self.prompt_text.get("1.0", "end").strip()
        if not prompt:
            self.main_status_var.set("Enter a scene description before generating.")
            self.prompt_text.focus_set()
            return
        try:
            api_key, _source = get_api_key()
        except SettingsStoreError as exc:
            self.main_status_var.set(str(exc))
            self.notebook.select(self.settings_tab)
            return
        if not api_key:
            self.main_status_var.set("Save an OpenAI API key in Settings first.")
            self.notebook.select(self.settings_tab)
            self.api_key_entry.focus_set()
            return

        request = GenerationRequest(
            prompt=prompt,
            api_key=api_key,
            quality=QUALITY_OPTIONS[self.quality_var.get()],
            reasoning_effort=REASONING_OPTIONS[self.reasoning_var.get()],
            multithreaded=self.multithreaded_var.get(),
        )
        self.main_status_var.set("")
        self.generate_button.configure(state="disabled")
        self.notebook.select(self.generation_tab)
        self._reset_generation_view()
        self.job_started_at = perf_counter()
        self.worker = threading.Thread(
            target=self._generation_worker,
            args=(request,),
            daemon=True,
            name="ray-tracer-generation",
        )
        self.worker.start()
        self._update_clock()

    def _generation_worker(self, request: GenerationRequest) -> None:
        try:
            result = self.service.run(
                request, lambda event: self.events.put(("event", event))
            )
        except GenerationCancelled as exc:
            self.events.put(("cancelled", exc))
        except Exception as exc:
            self.events.put(("error", exc))
        else:
            self.events.put(("result", result))

    def _reset_generation_view(self) -> None:
        self.saved_png = None
        self.source_image = None
        self.preview_image = None
        self.current_eta = None
        self.stage_var.set("Preparing generation")
        self.generation_status_var.set("Starting the generation pipeline…")
        self.generation_status.configure(style="TLabel")
        self.progress.stop()
        self.progress.configure(mode="indeterminate", value=0)
        self.progress.start(12)
        self.percent_var.set("")
        self.time_var.set("Estimating time remaining…")
        self.preview_label.configure(image="", text="Generating your scene…")
        self.saved_path_var.set("")
        self.open_folder_button.configure(state="disabled")

    def _poll_events(self) -> None:
        try:
            while True:
                event_type, payload = self.events.get_nowait()
                if event_type == "event":
                    self._handle_generation_event(payload)
                elif event_type == "result":
                    self._handle_result(payload)
                elif event_type == "error":
                    self._handle_error(payload)
                elif event_type == "cancelled":
                    self._handle_error(payload)
        except queue.Empty:
            pass
        self.root.after(100, self._poll_events)

    def _handle_generation_event(self, event: object) -> None:
        assert isinstance(event, GenerationEvent)
        stage_titles = {
            "scene": "Generating scene",
            "render": "Rendering image",
            "convert": "Converting to PNG",
            "complete": "Render complete",
        }
        self.stage_var.set(stage_titles.get(event.stage, "Generating"))
        self.generation_status_var.set(event.message)
        if event.percent is None:
            if str(self.progress.cget("mode")) != "indeterminate":
                self.progress.configure(mode="indeterminate")
                self.progress.start(12)
            self.percent_var.set("")
        else:
            self.progress.stop()
            self.progress.configure(mode="determinate", value=event.percent)
            self.percent_var.set(f"{event.percent}%")
        self.current_eta = event.eta_seconds

    def _handle_result(self, payload: object) -> None:
        assert isinstance(payload, GenerationResult)
        self.worker = None
        self.generate_button.configure(state="normal")
        self.progress.stop()
        self.progress.configure(mode="determinate", value=100)
        self.percent_var.set("100%")
        self.stage_var.set("Render complete")
        completion_message = "The PNG has been saved automatically and is ready to view."
        if payload.audit_path is not None:
            completion_message += " A quality audit was saved beside the scene JSON."
        self.generation_status_var.set(completion_message)
        self.generation_status.configure(style="Success.TLabel")
        self.time_var.set(f"Completed in {format_elapsed(payload.elapsed_seconds)}")
        self.saved_png = payload.png_path
        self.saved_path_var.set(f"Saved to: {payload.png_path}")
        self.open_folder_button.configure(state="normal")
        self._load_preview(payload.png_path)

    def _handle_error(self, payload: object) -> None:
        self.worker = None
        self.generate_button.configure(state="normal")
        self.progress.stop()
        self.progress.configure(mode="determinate", value=0)
        self.percent_var.set("")
        self.stage_var.set("Generation failed")
        self.generation_status.configure(style="Error.TLabel")
        self.generation_status_var.set(str(payload))
        self.time_var.set("Review the error, adjust the settings, and try again.")

    def _update_clock(self) -> None:
        if self.worker is None or not self.worker.is_alive() or self.job_started_at is None:
            return
        elapsed = perf_counter() - self.job_started_at
        elapsed_text = f"Elapsed: {format_elapsed(elapsed)}"
        if self.current_eta is None:
            self.time_var.set(f"{elapsed_text} · Estimating time remaining…")
        else:
            self.time_var.set(f"{elapsed_text} · {format_duration(self.current_eta)}")
        self.root.after(500, self._update_clock)

    def _load_preview(self, path: Path) -> None:
        try:
            from PIL import Image

            self.source_image = Image.open(path).copy()
        except Exception as exc:
            self.preview_label.configure(
                image="", text=f"The PNG was saved, but its preview could not be loaded:\n{exc}"
            )
            return
        self.root.after_idle(self._resize_preview)

    def _schedule_preview_resize(self, _event: tk.Event) -> None:
        if self.source_image is None:
            return
        if self._resize_after_id is not None:
            self.root.after_cancel(self._resize_after_id)
        self._resize_after_id = self.root.after(100, self._resize_preview)

    def _resize_preview(self) -> None:
        self._resize_after_id = None
        if self.source_image is None:
            return
        from PIL import Image, ImageTk

        width = max(100, self.preview_frame.winfo_width() - 16)
        height = max(100, self.preview_frame.winfo_height() - 16)
        preview = self.source_image.copy()
        preview.thumbnail((width, height), Image.Resampling.LANCZOS)
        self.preview_image = ImageTk.PhotoImage(preview)
        self.preview_label.configure(image=self.preview_image, text="")

    def _open_renders_folder(self) -> None:
        folder = self.service.project_root / "Renders"
        folder.mkdir(parents=True, exist_ok=True)
        try:
            if platform.system() == "Windows":
                os.startfile(folder)  # type: ignore[attr-defined]
            elif platform.system() == "Darwin":
                subprocess.Popen(["open", str(folder)])
            else:
                subprocess.Popen(["xdg-open", str(folder)])
        except OSError as exc:
            messagebox.showerror("Could not open folder", str(exc), parent=self.root)

    def _on_close(self) -> None:
        self.service.cancel()
        self.root.destroy()


def format_elapsed(seconds: float) -> str:
    seconds = max(0, round(seconds))
    minutes, remainder = divmod(seconds, 60)
    if minutes:
        return f"{minutes}m {remainder:02d}s"
    return f"{remainder}s"


def main() -> int:
    root = tk.Tk()
    RayTracerApp(root)
    root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
