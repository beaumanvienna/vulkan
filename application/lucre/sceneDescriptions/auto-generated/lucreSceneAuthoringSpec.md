# Lucre Scene Authoring (Prompt) File Format Specification

Copyright (c) 2026 JC Technolabs  
License: MIT (tooling-side authoring format; outputs [Lucre Scene Description JSON](https://github.com/beaumanvienna/vulkan/blob/master/application/lucre/sceneDescriptions/lucreSceneDescriptionSpec.md))

---

## Abstract

This document specifies a **tooling-side authoring format** intended for *prompt-engineering* and procedural scene construction for the **Lucre** engine.

- The **Lucre runtime** currently consumes only the **[Lucre Scene Description JSON](https://github.com/beaumanvienna/vulkan/blob/master/application/lucre/sceneDescriptions/lucreSceneDescriptionSpec.md)** (the “scene description file”).  
- This authoring format is a **higher-level description** that is designed to be compiled/transpiled into the existing scene description file format.

The intent is to support statements like:

> “We have asset `ring_1m` (diameter 1 m). Place rings along a path that is straight for 1 m, then ramps up at 45° until a target is reached, then continues straight for 5 m.”

This authoring format is split into two JSON documents:

1. **Asset Catalog**: declares placeable assets and their semantic metadata (dimensions, pivot, etc.).  
2. **Scene Recipe**: declares procedural placement rules (paths, grids, scatters), which compile into the concrete instance transforms required by the Lucre scene description file.

---

## 1. Introduction

Lucre scene description files already support:
- Referencing assets by filename (e.g., glTF/GLB)
- Instancing with `transform` (scale/rotation/translation)

However, scene description files are intentionally low-level:
- They do not contain *semantic asset metadata* (like “ring diameter is 1 m”).
- They do not contain *procedural generation rules* (like “place along this path at 0.5 m spacing”).

This authoring format addresses those needs without requiring changes to the Lucre runtime loader.

---

## 2. Requirements Language

The key words **MUST**, **MUST NOT**, **SHALL**, **SHOULD**, **MAY**, and **OPTIONAL** are to be interpreted as described in RFC 2119.

---

## 3. Files and Roles

### 3.1 Asset Catalog File

- Filename: any (recommended: `assetCatalog.json`)
- Purpose: declares *placeable assets* and their metadata for procedural rules.
- Consumer: authoring tool / transpiler (NOT Lucre runtime).

### 3.2 Scene Recipe File

- Filename: any (recommended: `sceneRecipe.json`)
- Purpose: declares *what to place* and *where/how to place it*.
- Consumer: authoring tool / transpiler (NOT Lucre runtime).

### 3.3 Output Scene Description File

The authoring tool MUST output a standard [Lucre Scene Description JSON](https://github.com/beaumanvienna/vulkan/blob/master/application/lucre/sceneDescriptions/lucreSceneDescriptionSpec.md) that the engine already loads.

---

## 4. Conventions and Units

- Distances are in **meters**.
- Angles are in **radians** unless a field explicitly supports degrees.
- Coordinate system MUST match the engine conventions used by scene description files (typically +Y up).

**Authoring scale convention:** Dimension fields in this authoring format describe the asset’s **real-world size when placed with instance `scale: [1,1,1]`**. For semantic dimension fields, the prefix `authoring*` is RECOMMENDED (e.g., `authoringDiameter_m`, `authoringExtents_m`). 

---

## 5. Asset Catalog JSON Specification

### 5.1 Root Object

```jsonc
{
  "version": "1.0",
  "assets": [
    { /* asset */ }
  ]
}
```

#### Fields

| Field | Type | Required | Description |
|---|---:|:---:|---|
| `version` | string | REQUIRED | Catalog format version, currently `"1.0"`. |
| `assets` | array | REQUIRED | List of asset declarations. |

---

### 5.2 Asset Object

```jsonc
{
  "id": "ring_1m",
  "source": {
    "type": "gltf",
    "filename": "application/lucre/models/props/ring.glb"
  },
  "semantics": {
    "shape": "ring",
    "authoringDiameter_m": 1.0
  },
  "placement": {
    "pivot": "center",
    "forwardAxis": "+Z",
    "upAxis": "+Y"
  }
}
```

#### Fields

| Field | Type | Required | Description |
|---|---:|:---:|---|
| `id` | string | REQUIRED | Unique identifier referenced by the Scene Recipe. |
| `source` | object | REQUIRED | Where the asset comes from (file-based). |
| `semantics` | object | OPTIONAL | Human/tool semantic metadata (dimensions, tags). |
| `placement` | object | OPTIONAL | Conventions for how this asset is oriented when placed. |

---

### 5.3 `source` Object

```jsonc
{ "type": "gltf", "filename": "..." }
```

| Field | Type | Required | Description |
|---|---:|:---:|---|
| `type` | string | REQUIRED | Currently `"gltf"` only (glTF/GLB). |
| `filename` | string | REQUIRED | Path to the asset file. |

**Note:** The authoring tool MUST map `source.filename` into the correct asset collection in the output scene description file (e.g., `fastgltf files` entry with `filename`).

---

### 5.4 `semantics` Object (Recommended Fields)

This object is tooling-only metadata. The Lucre runtime does not consume it.

| Field | Type | Required | Description |
|---|---:|:---:|---|

**Dimension fields:** Tools SHOULD treat `authoring*` dimension fields as authoritative sizes at instance scale `[1,1,1]`. 
| `shape` | string | OPTIONAL | Example: `"ring"`, `"tree"`, `"platform"`. |
| `authoringDiameter_m` | number | OPTIONAL | Ring diameter in meters at instance scale `[1,1,1]`.|
| `authoringExtents_m` | array[3] | OPTIONAL | Approx bounding box extents `(x,y,z)` in meters at instance scale `[1,1,1]`.|
| `tags` | array[string] | OPTIONAL | Free-form tags (e.g., `"collectible"`, `"obstacle"`). |

---

### 5.5 `placement` Object (Recommended Fields)

| Field | Type | Required | Description |
|---|---:|:---:|---|
| `pivot` | string | OPTIONAL | `"center"` (default) or `"bottom"`, etc. |
| `forwardAxis` | string | OPTIONAL | One of `"+X"`, `"-X"`, `"+Y"`, `"-Y"`, `"+Z"`, `"-Z"`. |
| `upAxis` | string | OPTIONAL | One of the same axis values. |

---

## 6. Scene Recipe JSON Specification

### 6.1 Root Object

```jsonc
{
  "version": "1.0",
  "scene": {
    "description": "Rings along a ramp path",
    "author": "JC Technolabs"
  },
  "placements": [
    { /* placement rule */ }
  ]
}
```

| Field | Type | Required | Description |
|---|---:|:---:|---|
| `version` | string | REQUIRED | Recipe format version, currently `"1.0"`. |
| `scene` | object | OPTIONAL | Metadata for the generated scene description file. |
| `placements` | array | REQUIRED | Procedural placement rules. |

---

## 7. Placement Rules

A placement rule describes:
- which asset to place (`assetId`)
- how many / how to distribute (along path, grid, scatter, single)
- optional deterministic randomness (seeded)

### 7.1 Common Rule Fields

| Field | Type | Required | Description |
|---|---:|:---:|---|
| `id` | string | OPTIONAL | Identifier for this rule (useful for debugging). |
| `assetId` | string | REQUIRED | References an `assets[].id` from the Asset Catalog. |
| `mode` | string | REQUIRED | One of: `single`, `along_path`, `grid`, `scatter`. |
| `transform` | object | OPTIONAL | Base transform applied before distribution. |
| `seed` | integer | OPTIONAL | If present, randomness MUST be deterministic. |

---

### 7.2 `single` Placement

Places exactly one instance.

```jsonc
{
  "id": "one_ring",
  "assetId": "ring_1m",
  "mode": "single",
  "transform": {
    "translation": [0, 0, 0],
    "rotation": [0, 0, 0],
    "scale": [1, 1, 1]
  }
}
```

---

### 7.3 `along_path` Placement

Places instances along a parametric path.

```jsonc
{
  "id": "rings_path",
  "assetId": "ring_1m",
  "mode": "along_path",
  "path": {
    "start": { "position": [0, 0, 0], "forward": [0, 0, 1] },
    "segments": [
      { "type": "line", "length_m": 1.0 },
      { "type": "ramp", "angle_deg": 45.0, "until": { "rise_m": 3.0 } },
      { "type": "line", "length_m": 5.0 }
    ]
  },
  "distribution": {
    "spacing_m": 1.0,
    "placeAtStart": true,
    "placeAtEnd": true
  },
  "orientation": {
    "faceAlongPath": true,
    "up": [0, 1, 0]
  }
}
```

#### 7.3.1 `path` Object

| Field | Type | Required | Description |
|---|---:|:---:|---|
| `start` | object | REQUIRED | Start frame (position + forward). |
| `segments` | array | REQUIRED | Ordered path segments. |

`start.forward` MUST be a normalized direction vector.

#### 7.3.2 Segment Types

##### (A) `line`

| Field | Type | Required |
|---|---:|:---:|
| `type` | string | REQUIRED (`"line"`) |
| `length_m` | number | REQUIRED |

##### (B) `ramp`

A ramp is a straight segment with a constant slope.

| Field | Type | Required | Description |
|---|---:|:---:|---|
| `type` | string | REQUIRED (`"ramp"`) |
| `angle_deg` | number | OPTIONAL | If present, degrees; otherwise use `angle_rad`. |
| `angle_rad` | number | OPTIONAL | Radians alternative to `angle_deg`. |
| `until` | object | REQUIRED | Ramp termination condition (see below). |

Ramp termination `until` MAY specify exactly one of:

- `rise_m`: stop when vertical rise reaches this value (relative to ramp start)  
- `run_m`: stop when horizontal run reaches this value  
- `length_m`: stop when along-ramp length reaches this value

**Important:** the authoring tool MUST reject ambiguous ramps that specify more than one of these fields.

This avoids guessing what “until 3 m have reached” means.

##### (C) Future segment types (Reserved)

The following segment `type` strings are RESERVED for future versions:
- `arc`
- `bezier`
- `spline`

---

#### 7.3.3 Distribution

| Field | Type | Required | Description |
|---|---:|:---:|---|
| `spacing_m` | number | REQUIRED | Distance between consecutive placements along the path. |
| `placeAtStart` | boolean | OPTIONAL | Default true. |
| `placeAtEnd` | boolean | OPTIONAL | Default false. |
| `count` | integer | OPTIONAL | Alternative to `spacing_m`; if present, tool distributes evenly. |

If both `count` and `spacing_m` are present, the tool MUST reject the rule as ambiguous.

---

#### 7.3.4 Orientation

| Field | Type | Required | Description |
|---|---:|:---:|---|
| `faceAlongPath` | boolean | OPTIONAL | If true, yaw is aligned to the tangent direction. |
| `up` | array[3] | OPTIONAL | World up vector (default `[0,1,0]`). |
| `additionalRotation` | array[3] | OPTIONAL | Euler offsets applied after alignment (radians). |

---

### 7.4 `grid` Placement (Optional)

Places instances on a 2D grid (useful for test scenes).

```jsonc
{
  "assetId": "ring_1m",
  "mode": "grid",
  "grid": {
    "origin": [0, 0, 0],
    "axisU": [1, 0, 0],
    "axisV": [0, 0, 1],
    "countU": 10,
    "countV": 3,
    "spacingU_m": 1.0,
    "spacingV_m": 1.0
  }
}
```

---

### 7.5 `scatter` Placement (Optional)

Places instances randomly (but deterministically if `seed` is provided).

```jsonc
{
  "assetId": "ring_1m",
  "mode": "scatter",
  "seed": 12345,
  "region": {
    "type": "box",
    "min": [-5, 0, -5],
    "max": [ 5, 0,  5]
  },
  "count": 25,
  "randomize": {
    "yaw_rad": { "min": 0, "max": 6.283185 },
    "scale": { "min": 0.9, "max": 1.1 }
  }
}
```

---

## 8. Compilation to Lucre Scene Description JSON

### 8.1 High-Level Rule

The compiler MUST translate each placement rule into one or more instances in the output scene description file, grouped by `source.filename`.

Concretely:
- Each distinct `source.filename` becomes one asset entry in the output scene description file.
- Each placement rule contributes zero or more `instances[]` objects under that entry.
- Each instance MUST contain a `transform` object with:
  - `scale`: array[3]
  - `rotation`: array[3]
  - `translation`: array[3]

### 8.2 Determinism

Given identical:
- asset catalog
- scene recipe
- tool version

… the compiler SHOULD produce identical output JSON (stable ordering), to keep diffs clean.

---

## 9. Versioning

- This authoring format uses `version: "1.0"` strings.
- Any incompatible change MUST bump the major version.

---

## 10. Non-Goals

This authoring format intentionally does NOT:
- Modify or extend the Lucre runtime loader
- Encode renderer pipelines or material graphs
- Encode physics simulation state
- Encode animation timelines (glTF handles meshes/skins/animations)

---

## 11. Example: “Rings Along a Path” (Catalog + Recipe)

### 11.1 `assetCatalog.json`

```json
{
  "version": "1.0",
  "assets": [
    {
      "id": "ring_1m",
      "source": { "type": "gltf", "filename": "application/lucre/models/props/ring.glb" },
      "semantics": { "shape": "ring", "authoringDiameter_m": 1.0 },
      "placement": { "pivot": "center", "forwardAxis": "+Z", "upAxis": "+Y" }
    }
  ]
}
```

### 11.2 `sceneRecipe.json`

```json
{
  "version": "1.0",
  "scene": { "description": "Rings along a ramp path", "author": "JC Technolabs" },
  "placements": [
    {
      "id": "rings_path",
      "assetId": "ring_1m",
      "mode": "along_path",
      "path": {
        "start": { "position": [0, 0, 0], "forward": [0, 0, 1] },
        "segments": [
          { "type": "line", "length_m": 1.0 },
          { "type": "ramp", "angle_deg": 45.0, "until": { "rise_m": 3.0 } },
          { "type": "line", "length_m": 5.0 }
        ]
      },
      "distribution": { "spacing_m": 1.0, "placeAtStart": true, "placeAtEnd": true },
      "orientation": { "faceAlongPath": true, "up": [0, 1, 0] }
    }
  ]
}
```

### 11.3 Output (Generated) Scene Description JSON (Sketch)

```jsonc
{
  "file format identifier": 1.2,
  "description": "Rings along a ramp path",
  "author": "JC Technolabs",
  "fastgltf files": [
    {
      "filename": "application/lucre/models/props/ring.glb",
      "instances": [
        { "transform": { "scale": [1,1,1], "rotation": [0,0,0], "translation": [0,0,0] } }
        // ... more generated instances
      ]
    }
  ]
}
```

**Note:** The exact numeric transforms depend on the interpretation of the ramp `until` condition and the placement `spacing_m`.

---

## 12. Future Extensions

Future versions MAY add:
- Named groups / layers for rules
- Path curvature (splines)
- Terrain conforming (“project onto terrain”)
- Collision-free scattering
- Rule composition (place rings, then place lights at ring positions)

All additions MUST preserve clear non-ambiguous semantics and deterministic compilation.
