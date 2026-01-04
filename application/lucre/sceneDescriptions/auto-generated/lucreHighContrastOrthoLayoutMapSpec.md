# Lucre High-Contrast Ortho Layout Map Specification (AI-Readable)

This document defines a **high-contrast, orthographic (top-down) layout map** format intended for prompt-driven scene authoring in **Lucre**.  
The output is an **AI-readable planning image** (not concept art) that can be reliably converted into:
1) an **asset list**, and  
2) a **Lucre Scene Description JSON** using an asset catalog.

---

## 1. Purpose

The layout map is designed to:
- be **strictly orthographic** (no perspective, no foreshortening),
- convey **clear semantic regions** (hedge, walkway, grass, water, etc.),
- include a **metric reference** (grid/checkerboard) so distances can be measured,
- include **explicit annotations** (labels + markers) for key landmarks.

The map should be easy to parse by humans, AI, and simple tooling (color masking / flood-fill).

---

## 2. Output Format

### 2.1 Image requirements
- **Projection:** Orthographic top-down (map view).
- **Camera:** perfectly vertical, looking down (no tilt).
- **Style:** flat colors, **no shadows**, **no textures**, **no lighting**, **no noise**.
- **Edges:** crisp boundaries (hard transitions).
- **Resolution:** 2048×2048 recommended (minimum 1024×1024).
- **Background:** neutral (white or light gray) and consistent.

### 2.2 Two deliverables (recommended)
Produce **two images** for each layout:

1) **`layout_clean.png`**  
   - Only color-coded regions and the grid.  
   - **No text**, no arrows, no overlays besides the grid.  
   - Intended for segmentation / automated processing.

2) **`layout_annotated.png`**  
   - Same as clean map **plus** labels/arrows/markers.  
   - Intended for debugging + communicating intent.

If only one image is produced, it must be the **annotated** one.

---

## 3. Coordinate System and Units

### 3.1 World axes convention (Lucre)
- **X/Z** form the horizontal plane.
- **Y** is up.
- The layout map depicts the **X/Z plane**.

### 3.2 Origin marker
The map must include a clearly labeled origin marker:
- **Red filled circle** labeled: `ORIGIN (0,0,0) — fountain center`  
  (or another chosen anchor if the prompt specifies it).

### 3.3 Metric reference
The map must include a metric reference that is unambiguous.

**Preferred: Checkerboard grid**
- Alternating black/white squares.
- **Each square = 1 meter × 1 meter**.
- Grid covers the entire map or at least the playable region plus margins.
- Label at top: `CHECKERBOARD: 1m spacing`.

Alternative: Line grid with scale bar `0m — 5m — 10m` (only if checkerboard is undesirable).

---

## 4. Color Coding (Semantic Layers)

The layout uses **flat, unique colors** per category. Colors must not be reused.

Recommended palette (RGB / HEX), chosen for high contrast:

### 4.1 Walkable surfaces
- **Walkway (pebble / sand path):** `#1E4CFF` (blue)
- **Grass lawn (open play area):** `#78FF78` (light green)

### 4.2 Barriers / borders
- **Hedge / wall (solid barrier):** `#006400` (dark green)
- **Mansion footprint / building boundary:** `#8B6B4A` (brown)

### 4.3 Points of interest
- **Fountain basin footprint:** `#FF0000` (red circle)
- **Statue footprint:** `#FF0000` (red square)
- **Bench footprint:** `#FFD966` (gold/yellow rectangle)
- **Round bushes:** `#FFF000` (bright yellow circles)

### 4.4 Reserved / optional
- **Water (if different from fountain):** `#00D5FF` (cyan)
- **Flowerbeds:** `#FF4FD8` (magenta)
- **Trees (canopy footprint):** `#00A86B` (teal-green)

**Rules:**
- Every semantic category must be a **single solid color** (no gradients).
- Regions must be **fully filled** (no hatching unless explicitly requested).
- Do not anti-alias boundaries if possible (or keep it minimal).

---

## 5. Geometry Rules

### 5.1 No perspective artifacts
- No vanishing points.
- No painterly blending.
- All shapes are drawn in **plan view**.

### 5.2 Snapping
- Prefer snapping hedge and walkway edges to the 1m grid (unless the prompt demands curves).
- Curves are allowed, but should be clearly drawn and continuous with consistent thickness.

### 5.3 Thickness semantics
The prompt or spec must define thicknesses in meters. Example:
- Hedge width: **1.0 m**
- Walkway width: **3.0 m**

If not specified by the user, the **annotated** image must label:
- `HEDGE WIDTH = ?m`
- `PATH WIDTH = ?m`

---

## 6. Annotation Rules (Annotated Map Only)

### 6.1 Mandatory labels
The annotated map must include:
- `CHECKERBOARD: 1m spacing`
- `NORTH` arrow (top of image is north)
- `ORIGIN (0,0,0)` marker label
- A legend mapping colors to meaning (hedge, walkway, grass, etc.)

### 6.2 Markers
Use:
- **Red circle** = fountain center
- **Red square** = statue center
- **Yellow circles** = round bushes centers
- **Gold rectangles** = benches (include orientation if meaningful)

### 6.3 Text placement
- Put labels in margins when possible.
- Use thin leader lines/arrows to avoid occluding regions.

---

## 7. Scene Authoring Constraints

The map is intended to be converted into instances from an asset catalog. Therefore:
- Avoid shapes that require **non-uniform scaling** to match.
- Prefer modular lengths aligned to common module sizes (e.g. 2m, 1m, 0.5m).
- If a segment length is not an integer multiple of a module length, label it explicitly:
  - `HEDGE SEGMENT LENGTH = 3.2m (4×0.8m modules)`

---

## 8. Required Output Checklist

Before delivering a layout map, verify:
- [ ] Orthographic top-down, no perspective.
- [ ] High contrast, flat fills, no shading/texture.
- [ ] 1m checkerboard grid present + labeled.
- [ ] Legend present (annotated map).
- [ ] Origin marker present and labeled.
- [ ] North arrow present.
- [ ] All semantic categories use unique, consistent colors.
- [ ] Hedge / path widths stated or implied by grid.
- [ ] Clean map version provided (recommended).

---

## 9. Prompt Template for Generating a Layout Map

Use this template when you provide a garden prompt.

### 9.1 Clean map prompt
```
Create a STRICTLY ORTHOGRAPHIC, TOP-DOWN layout map (no perspective) for the following scene:

[SCENE DESCRIPTION]

Output a high-contrast planning map suitable for AI asset placement:
- Flat solid colors only, no shading, no textures, no shadows.
- Crisp boundaries.
- Include a full-image checkerboard where each square is exactly 1m x 1m.
- The map represents the X/Z plane; Y is up in the engine.
- Place a labeled origin marker: a red circle labeled "ORIGIN (0,0,0) — fountain center".
- Add a north arrow (top of image is north).
- Use this color legend exactly:
  - Walkway (pebble/sand): #1E4CFF
  - Grass: #78FF78
  - Hedge/wall: #006400
  - Mansion footprint: #8B6B4A
  - Fountain footprint: #FF0000 (circle)
  - Statue footprint: #FF0000 (square)
  - Bench footprint: #FFD966 (rectangle)
  - Round bushes: #FFF000 (circles)

Do NOT draw any 3D perspective. Do NOT render realism. This is a map, not concept art.
```

### 9.2 Annotated map prompt (optional second image)
```
Create a second version of the same map with annotations:
- Add a legend with text labels for each color category.
- Label hedge width and walkway width in meters.
- Label the statue (red square) and fountain (red circle).
- Add arrows/leader lines where helpful, without obscuring the map.
Keep the underlying geometry identical to the clean map.
```

---

## 10. Example Scene Constraints (Garden)

If the user prompt is a garden:
- Maze area should be ~25% of the playable garden.
- Include an English garden vibe (formal hedges, symmetry, borders).
- Include a walkway with two benches.
- Border a mansion (croft/wayne-manor-like footprint).

All of these must be encoded as **plan-view geometry**.

---

## 11. Notes for Robust AI Parsing

To make downstream conversion easier:
- Keep hedge segments straight unless a curve is explicitly required.
- Keep stubs/openings obvious (don’t hide openings behind vegetation).
- Avoid ambiguous “soft edges” around flowerbeds; use hard shapes.
- Prefer fewer categories over many micro-categories early on.

---

## 12. Versioning

- **Spec version:** 1.0
- Increment minor version when adding categories or constraints.
- Increment major version if changing coordinate conventions or color rules.
