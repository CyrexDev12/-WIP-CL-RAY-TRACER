# Iteration 2 Provisional Review

## Test conditions

- Software version: `1.1-dev`
- Assessment date: 2026-07-27
- Model: `gpt-5.4-mini`
- Quality: High (800-pixel long edge)
- Reasoning effort: Medium
- Multithreading: enabled
- API timeout: 180 seconds
- Evidence: `TestDocs/Test Renders/Iter2`

All six renders completed. The first Pattern Material Gallery response used direct
pattern `scale` and `rotate` fields instead of the canonical nested `transform`
object. A deterministic compatibility normalization was added, and the runner's
failed-only retry completed that render without replacing the five successful
evidence packages. No API request timed out.

## Provisional assessment

These ratings are an engineering review, not a replacement for the human assessment
entered in the workbook.

| Render | Required | Provisional passed | Failed deliverables | Visual appeal | Assessment score |
|---|---:|---:|---|---:|---:|
| Glass, Chrome, and Reflections | 11 | 9 | 3, 7 | 5 | 65.91 |
| Neon Planetary System | 10 | 9 | 4 | 6 | 75.00 |
| Pattern Material Gallery | 11 | 6 | 3, 4, 5, 8, 9 | 6 | 57.27 |
| Geometric Sci-Fi Temple | 10 | 8 | 3, 9 | 5 | 65.00 |
| Abstract Recursive Sculpture | 12 | 8 | 3, 4, 11, 12 | 4 | 53.33 |
| Surreal Checkerboard Landscape | 11 | 10 | 10 | 7 | 80.45 |

Provisional totals:

- Deliverables: 50 of 65, or 76.92%.
- Average visual appeal: 5.50 of 10.
- Overall assessment score: 66.16.
- Change from Iteration 1 overall score: +6.29 points.

## Findings

### Glass, Chrome, and Reflections

The layout is substantially clearer and the glass, gold, colored spheres, floor,
perspective, highlights, and background read well. The dark chrome cube is present
in scene JSON but nearly disappears against the black background and checker floor.
Its reflections therefore are not visibly assessable.

### Neon Planetary System

The sun retains its orange hue under bloom, five planets are present at multiple
depths, and the reflective plane is readable. The gas giant renders as a smoothly
shaded tan planet rather than a visibly striped planet.

### Pattern Material Gallery

All five spheres and pedestals are cleanly arranged, and the checker and perturbed
materials read clearly. The stripe, gradient, and ring patterns collapse toward
solid black, cream, and red because their generated pattern scales are too large for
the renderer's inverse-transform convention. Consequently, scale and rotation
differences are not consistently visible.

### Geometric Sci-Fi Temple

The scene is centered and readable with columns, gold structure, cyan triangles,
and a reflective floor. The central cubes do not read as distinctly stacked and
rotated, and the warm entrance illumination is too subdued. The structure remains
blocky and shallow despite improved framing.

### Abstract Recursive Sculpture

The silver, blue, and copper vertical twist is much more legible than Iteration 1.
Individual cube and fin counts are still difficult to verify, several fins are only
thin slivers, the floating gap is unclear, and the requested restrained blue
emission is not visually evident. The audit also retains seven too-small and ten
possible-occlusion warnings, correctly identifying this as the weakest composition.

### Surreal Checkerboard Landscape

This is the strongest Iteration 2 render. The three receding spheres, refractive
foreground cube, leaning cylinders, cyan focal sphere, low camera, reflections, and
bloom are all readable. Long shadows are difficult to distinguish against the dense
checker pattern.

## Recommended Iteration 3 priorities

1. Add pattern-frequency normalization for patterns attached to spheres, with
   pattern-type-specific readable ranges.
2. Add contrast checks that compare reflective dark objects with the background and
   nearby projected floor colors, rather than checking material ambient alone.
3. Add recursive-group readability rules for minimum fin thickness, group spacing,
   and visible separation between repeated components.
4. Detect whether requested lighting effects are visually represented using a small
   preview-image analysis pass, especially shadows, pattern variance, and emissive
   contrast.
5. Preserve the failed-only retry procedure; it avoided five unnecessary API calls
   while keeping the controlled settings unchanged.
