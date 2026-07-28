# Version 1.1 Render Quality Improvement Plan

## Purpose

Version 1.0 is the controlled baseline. Iteration 1 scored **59.87** with an
average deliverable score of **76.41%** and an average visual-appeal rating of
**4.33/10**. Version 1.1 focuses on making requested objects clearly visible,
improving composition, and preventing highlight clipping while keeping the six
benchmark prompts, High quality, Medium reasoning, and multithreading unchanged.

## Current implementation status

- Implemented: explicit generation composition guardrails.
- Implemented: recursive finite-object bounds and inventory.
- Implemented: camera projection audit, safe-frame diagnostics, and conditional
  automatic camera fitting.
- Implemented: per-scene audit sidecars with Version 1.1 development metadata.
- Confirmed: the pulled controlled SOP and application both use an 800-pixel High
  preset, preserving direct comparison with Version 1.0.
- Implemented: hue-preserving HDR tone mapping, display gamma, and threshold-energy
  bloom extraction.
- Implemented: conservative top-level occlusion separation plus dark, transparent,
  and excessive-emission material corrections with audit records.
- Implemented: spherical pattern mapping and correct pattern transforms through
  recursive object groups.
- Implemented: workbook cleanup and a six-prompt benchmark runner that locks High
  quality, Medium reasoning, and multithreading.
- Completed: the full Iteration 2 benchmark retained all six PNG, scene, audit, and
  manifest evidence packages with no request timeouts.
- Completed: provisional review against the official 65-item deliverables checklist;
  final workbook ratings remain a human assessment step.
- Next iteration: address pattern-frequency visibility, background-relative material
  contrast, and nested-group component readability.

## Success criteria

The Iteration 2 release gate is:

- Overall assessment score at or above **75.00**.
- Average deliverable score at or above **85%**.
- Average visual-appeal rating at or above **6.5/10**.
- No required focal object entirely outside the camera frame.
- No benchmark render lost to an API or renderer failure.
- The scene JSON, audit report, PNG, software version, and test settings are retained.

The stretch target is an **82.5** overall score, using approximate per-render targets
of 80, 85, 90, 80, 80, and 80.

## Engineering principles

1. Preserve benchmark comparability. Do not edit the six prompts during Iteration 2.
2. Prefer deterministic corrections after model generation over relying only on
   subjective prompt wording.
3. Separate structural presence from visual visibility. An object in JSON does not
   count as successful if it is clipped, hidden, too small, or effectively black.
4. Keep every correction observable through a machine-readable audit artifact.
5. Add changes in independently testable phases and retain Version 1.0 evidence.

## Phase 0 — Measurement integrity

### Work

- Confirm the SOP High-quality definition and application preset remain aligned at
  an 800-pixel long edge.
- Record software version as `1.0` or `1.0+<commit>` rather than `1`.
- Export no failed deliverables as `[]`, not `[0]`.
- Normalize workbook chart series to a common 0–100 scale.
- Keep blank future iterations blank instead of displaying version 0 or a 1900 date.

### Gate

One dry-run workbook export contains correct versioning, settings, blank handling,
and an empty failed-deliverables list when all requirements pass.

## Phase 1 — Generation constraints and scene inventory

### Work

- Strengthen the system prompt with explicit safe-frame, separation, lighting,
  recursive-scale, backdrop, and bloom limits.
- Inventory every renderable leaf after schema validation.
- Preserve object paths through recursive groups so diagnostics can identify the
  exact generated object.
- Record object-type and material-risk counts in a sidecar audit report.

### Gate

Unit tests prove that recursive groups are inventoried with correct world transforms
and that all current scene-schema types are supported.

## Phase 2 — Deterministic composition audit and camera correction

### Work

- Calculate world-space bounds for spheres, cubes, finite cylinders, triangles, and
  recursively transformed group children.
- Exclude infinite planes and detect oversized thin backdrop geometry when fitting.
- Project object bounds into camera space.
- Detect objects behind the camera, outside the frame, clipped by the safe margin,
  too small to read, or heavily covered by another projected object.
- Recenter and refit the camera while preserving the generated viewing direction,
  field of view, aspect ratio, and upward-looking/low-angle character.
- Re-run the audit after correction and save a JSON report beside the scene.
- Provide an opt-out for intentionally cropped artistic scenes.

### Gate

- All finite test objects fit inside a 10% safe-frame margin.
- A tiny recursive sculpture is enlarged to readable frame coverage.
- A clipped temple emblem is brought inside the frame.
- Giant thin backdrop cubes do not force the subject to become tiny.
- Existing quality, API, UI, and render-pipeline tests remain green.

## Phase 3 — Exposure, tone mapping, and bloom control

### Work

- Add configurable exposure and tone-mapping fields to the scene schema.
- Apply tone mapping after HDR lighting and bloom but before gamma encoding and PPM
  quantization.
- Replace hard white clipping with a highlight-compressing curve.
- Change bright-pass bloom extraction to use energy above the threshold rather than
  copying the complete bright pixel.
- Add prompt and audit warnings for excessive emission/bloom combinations.

### Gate

The Neon Planetary System retains visible orange highlight gradation in the sun while
the planets remain readable; non-emissive regression renders remain acceptably close.

## Phase 4 — Material and lighting readability

### Work

- Flag dark low-ambient materials that have insufficient contrast.
- Add support for a limited fill or rim light, or add a deterministic ambient
  correction when the one-light schema must remain fixed.
- Cap reflective/transmissive combinations that erase local material identity.
- Improve transparent-object edge readability.

### Gate

The chrome cube, temple columns, doorway, gold accents, and glass cube remain visually
distinct without violating their requested materials.

## Phase 5 — Pattern fidelity and advanced capabilities

### Work

- Add spherical UV coordinates for patterns or explicitly classify current checkers
  as object-space 3D checkers in the benchmark.
- Add optional preview rendering and image-statistics checks for black-frame ratio,
  saturation, clipping, and subject coverage.
- Consider a single automatic repair/rerender pass only after deterministic checks
  prove insufficient.

### Gate

The Pattern Material Gallery checker sphere reads as an intentional checker pattern,
and preview checks do not add unacceptable runtime or nondeterminism.

## Test strategy

- Python unit tests: transform composition, bounds, projection, backdrop filtering,
  audit issue codes, camera fitting, and sidecar serialization.
- API tests: generated scene is audited and corrected before being written.
- Pipeline test: desktop generation retains both scene and audit paths.
- C++ tests: tone-map transfer values, bloom threshold behavior, and PPM limits.
- Benchmark test: rerun all six prompts in numerical order under the SOP conditions.

## Rollout sequence

1. Implement and merge Phases 1–2.
2. Run a preview-resolution engineering pass on all six saved Version 1.0 scenes.
3. Tune only documented thresholds; do not edit benchmark prompts.
4. Implement Phase 3 and compare exposure/bloom A/B renders.
5. Complete Iteration 2 at High quality and Medium reasoning.
6. Use the structured workbook export to select Phase 4 or Phase 5 work based on the
   remaining failure categories.

## Risks and controls

- **Camera correction changes artistic intent:** preserve viewing direction and offer
  an explicit opt-out.
- **Backdrop geometry distorts bounds:** exclude infinite planes and oversized thin
  backdrop proxies from subject fitting.
- **False occlusion warnings:** treat projected overlap as a warning, not a hard error.
- **Tone mapping changes the Version 1.0 look:** introduce it as a Version 1.1 setting
  and keep regression renders for comparison.
- **Optimization overfits six prompts:** test generic scenes for portrait/landscape
  aspect ratios, nested groups, single objects, and intentionally large subjects.
