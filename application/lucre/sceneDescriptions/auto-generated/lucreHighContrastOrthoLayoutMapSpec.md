# Lucre High-Contrast Ortho Layout Map Specification (AI-Readable)

This document defines a **single, annotated, high-contrast orthographic layout map** format intended for prompt-driven scene authoring in **Lucre**.

The output is an **AI-readable planning image** (not realism, not concept art) that can be reliably converted into:
1) an **asset list**, and  
2) a **Lucre Scene Description JSON** using an asset catalog.

---

## 1. Purpose

The layout map is designed to:
- be **strictly orthographic** (no perspective, no foreshortening),
- convey **clear semantic regions** (hedge, walkway, grass, building footprint, etc.),
- include a **visible, numbered metric grid** so distances can be measured unambiguously,
- include **explicit annotations** (legend + markers + labels) for key landmarks and dimensions,
- be **optimized for retrieval** of translation/rotation/scale (high contrast, crisp edges, minimal categories),
- enforce a **1:1 translation** from the generated concept art (no guessing, no creative add-ons).

---

## 2. Required Deliverables

### 2.1 Layout map (single, annotated, required)
Produce **one** layout map image that is:
- **Top-down orthographic**
- Flat colors, high contrast, crisp edges.
- **Always annotated** (legend + labels + markers + numbered grid).

> The spec does **not** allow an unannotated map variant.  
> Everything in Section 6 (Annotations) is **always applicable**.

---

## 3. Accuracy and Alignment Requirements (No Guessing)

### 3.1 1:1 geometry rule
The layout map must be a **direct, 1:1 translation** of the concept art:
- Every hedge, opening, gate, path, lawn, wall/footprint, and POI that exists in the concept
  must exist in here defined map at the **same position and footprint**
- Do **not** add, remove, “improve”, stylize, or “symmetrize” geometry.
- Do **not** introduce extra elements “because it looks nice”.
- If something is unclear in the concept art, **stop and ask JC** (do not infer).

### 3.2 Camera and framing lock
the concept art and the orthographic map must share:
- identical framing (same bounds / crop),
- identical orientation (north-up),
- identical scale (1m grid matches the same physical area in both images).

### 3.3 Consistency check (must be possible visually)
A human must be able to quickly “flip between” the images and confirm:
- the maze footprint matches,
- path widths and openings match,
- hedge widths match,
- POI locations match.

---

## 4. Coordinate System and Units

### 4.1 World axes convention (Lucre)
- **X/Z** form the horizontal plane.
- **Y** is up.
- The layout map depicts the **X/Z plane**.

### 4.2 Origin marker (required)
The map must include a clearly labeled origin marker:
- **Red filled circle** labeled: `ORIGIN (0,0,0)` plus a short anchor description (e.g., `fountain center`).

If the prompt specifies a different anchor, follow the prompt exactly and label it.

### 4.3 Metric grid (required, numbered)
A visible grid is mandatory:

- **Grid cell size:** **1m × 1m**.
- **Grid lines:** thin, high-contrast lines that do not overwhelm the regions.
- **Numbering:**
  - Along the top and left margins, print integer coordinates at regular intervals (e.g., every 1m or every 2m),
    so every cell can be referenced.
  - The numbering must make it unambiguous which direction increases X and which increases Z.
  - Include labels: `+X →` and `+Z ↓` (or `+Z ↑`, but be consistent and state it).

Optional (allowed) but not required:
- Checkerboard background **outside** the playable region to emphasize scale.
- A small scale bar `0m — 5m — 10m` in the margin.

---

## 5. Rendering Style Rules (Optimized for Parsing)

- **Flat fills only:** no textures, no shading, no shadows, no lighting, no noise.
- **No realism:** this is a planning map, not a render.
- **Crisp boundaries:** hard transitions, minimal anti-aliasing.
- **One color per semantic region:** no gradients.

Resolution:
- 2048×2048 recommended (minimum 1024×1024).

---

## 6. Annotation Rules (Always Required)

### 6.1 Mandatory on-map text
The annotated map must include:
- `NORTH` arrow (top of image is north).
- `ORIGIN (0,0,0)` marker label.
- `GRID: 1m spacing` label.
- A **legend** that maps colors to meaning (Section 7 palette).

### 6.2 Mandatory dimension labels
The map must explicitly label, in meters:
- hedge width (e.g., `HEDGE WIDTH = 1.0m`),
- primary path / walkway width (e.g., `PATH WIDTH = 3.0m`).

If multiple hedge/path widths exist, label each distinct width near an example segment.

### 6.3 Markers (required symbols)
Use consistent marker shapes:
- **Fountain footprint:** red circle (centered).
- **Statue footprint:** red square (centered).
- **Benches:** gold/yellow rectangles with a short orientation arrow if facing matters.
- **Gate openings:** clearly marked (see Section 8.3).

Place labels in margins when possible and use leader lines/arrows to avoid occluding geometry.

---

## 7. Color Coding (Semantic Layers)

The layout uses **flat, unique colors** per category. Colors must not be reused.

Recommended palette (HEX), chosen for high contrast:

### 7.1 Walkable surfaces
- **Walkway (pebble/sand path):** `#1E4CFF` (blue)
- **Grass lawn:** `#78FF78` (light green)

### 7.2 Barriers / borders
- **Hedge / wall (solid barrier):** `#006400` (dark green)
- **Building footprint (mansion / walls):** `#8B6B4A` (brown)

### 7.3 Points of interest
- **Fountain marker:** `#FF0000` (red circle)
- **Statue marker:** `#FF0000` (red square)
- **Bench footprint:** `#FFD966` (gold rectangle)
- **Round bushes:** `#FFF000` (bright yellow circles)

### 7.4 Optional categories (only if present in concept art)
- **Water (non-fountain):** `#00D5FF` (cyan)
- **Flowerbeds:** `#FF4FD8` (magenta)
- **Trees (canopy footprint):** `#00A86B` (teal-green)

**Rules:**
- Only include optional categories if they exist in the concept art.
- Do not introduce new categories “just in case”.

---

## 8. Geometry Rules (Map Semantics)

### 8.1 No perspective artifacts
All geometry must be plan view. No vanishing points.

### 8.2 Snapping
Prefer snapping hedge and walkway edges to the 1m grid **when the concept art is grid-aligned**.
If the concept art intentionally contains non-grid-aligned geometry, preserve it exactly.

### 8.3 Openings and gates (must be explicit)
Entrances/openings must be obvious and machine-readable:
- Leave a clear gap in the hedge color region where the opening is.
- Optionally add a small label: `GATE OPENING` and/or a thin outline.

Do not “almost close” openings with fuzzy edges.

---

## 9. Scene Authoring Constraints (Asset Placement Friendly)

This map is intended to be converted into instances from an asset catalog. Therefore:
- Avoid shapes that require **non-uniform scaling** to match.
- Prefer modular lengths aligned to common module sizes (e.g., 2m, 1m, 0.5m),
  **but only if the concept art already uses those lengths**.

If a segment length is not an integer multiple of an available module length:
- Keep the geometry correct in the map,
- and label it explicitly (e.g., `HEDGE SEGMENT = 3.2m`).

---

## 10. Required Output Checklist (Hard Requirements)

Before delivering the two images, verify:

### Alignment
- [ ] concept art is top-down orthographic (no perspective).
- [ ] map is camera-matched to concept art  (same framing/orientation/scale).
- [ ] Layout contains **no added or missing geometry** versus concept art.

### Parsing quality
- [ ] Flat fills only; no shading/texture/noise.
- [ ] Crisp boundaries.
- [ ] Unique colors per semantic category.

### Grid + annotations
- [ ] Visible **1m grid** overlaid on the playable region.
- [ ] Grid is **numbered** with clear +X and +Z direction.
- [ ] North arrow present.
- [ ] Origin marker present and labeled.
- [ ] Legend present.
- [ ] Hedge width and path width labeled in meters.

---

## 11. Prompt Template (Use This Exactly)

### 11.1 Concept art prompt 
```
Create ONE image: concept art for this scene:

[SCENE DESCRIPTION]

Hard constraints:
- The geometry (hedges/paths/lawns/openings/POIs) must be clearly readable.
- Do NOT add random elements beyond what the scene description requires.
```

### 11.2 Layout map prompt 
```
Create ONE image: a HIGH-CONTRAST, TOP-DOWN ORTHOGRAPHIC layout map that is a 1:1 translation of the provided concept art image.

Hard constraints:
- Must match the concept art geometry exactly (no guessing, no creative changes, no extra objects).
- Flat solid colors only. No shading, no textures, no shadows.
- Crisp boundaries.

Grid (required):
- Overlay a visible 1m x 1m grid across the entire playable region.
- Number the grid along the margins and label axes directions: +X and +Z.
- Label: "GRID: 1m spacing".

Annotations (required):
- Add NORTH arrow (top is north).
- Add origin marker: a red circle labeled "ORIGIN (0,0,0) — [anchor]".
- Add a legend mapping colors to categories.
- Label hedge width and path width in meters.

Use this exact color legend:
- Walkway: #1E4CFF
- Grass: #78FF78
- Hedge: #006400
- Building footprint: #8B6B4A
- Fountain marker: #FF0000 (circle)
- Statue marker: #FF0000 (square)
- Bench: #FFD966
- Round bushes: #FFF000

This is NOT a realism render. It must be optimized for extracting translation/rotation/scale.
If anything is unclear in the concept art, stop and ask JC.
```

---

## 12. Versioning
- **Spec version:** 1.0
- Increment minor version for small clarifications.
- Increment major version for changes to grid/axes/color rules.
