# Lucre Scene Description File Format Specification

Copyright (c) 2025 JC Technolabs  
License: MIT (engine internal format)

---

## Abstract

This document specifies the **Lucre Scene Description File Format** used by the **Lucre** game engine to describe renderable scenes, terrain, and instanced assets.

The format is **JSON-based** and designed to:

- Reference external assets (glTF/GLB, terrain descriptors)
- Describe instancing with transforms
- Remain deterministic and engine-friendly
- Be easily serialized and deserialized by the Lucre C++ runtime

This specification is derived directly from:
- Existing scene JSON files
- The C++ scene loader and serializer implementation

---

## 1. Introduction

Lucre scenes are described declaratively using JSON files. JSON does not allow comments. Do not add comments to the JSON output.
A scene file defines **what assets exist** in a scene and **how they are placed**, but does **not** encode rendering pipelines, ECS runtime state, or GPU resources directly.

The scene loader is responsible for:
- Loading referenced assets
- Instantiating them with transforms
- Integrating them into the engine’s ECS and renderer

---

## 2. Requirements Language

The key words **MUST**, **MUST NOT**, **SHALL**, **SHOULD**, **MAY**, and **OPTIONAL** are to be interpreted as described in RFC 2119.

---

## 2.1 Mandatory Authoring Rules (Strict, No Creativity)

These rules apply **whenever an AI (or any automated tool)** generates a Lucre Scene Description JSON from *any* input source (prompt text, orthographic map, concept art, annotated layout, etc.).

- The provided input is the **single source of truth**. The generator **MUST** translate the input into a scene description **1:1**.
- The generator **MUST NOT** add, remove, “improve”, “fill in”, infer, embellish, or otherwise introduce **any** objects, geometry, or layout details that are not explicitly present in the provided input.
- If an element is ambiguous / not fully specified by the input, the generator **MUST** stop and ask JC for clarification (do **not** guess).
- Every instance **MUST** have explicitly determined:
  - **translation** (position),
  - **rotation** (orientation), and
  - **scale** (uniform or non-uniform only if explicitly allowed by the caller/spec).
- The generator **MUST NOT** default rotation to “no rotation” unless the input explicitly implies that the object is axis-aligned with zero rotation (or the caller explicitly requests zero rotation).
- The generator **MUST NOT** “snap”, “quantize”, or “approximate” placements unless the input explicitly defines snapping/rounding rules (e.g., a numbered grid).
- The generator **MUST** use the asset catalog as the authoritative source for asset IDs/paths and nominal dimensions at scale (1,1,1). If the requested real‑world size differs, the generator **MUST** compute the required scale from the catalog dimensions (and follow any uniform-scaling constraints given by the caller).
- The output **MUST** be a valid Lucre Scene Description JSON that conforms to this spec (no extra fields, no missing required fields).

---


## 3. Scene JSON Specification

Scene files MUST be valid JSON documents.

### 3.1 Root Object

```jsonc
{
  "file format identifier": 1.2,
  "description": "pbr scene",
  "author": "Copyright (c) 2025 Engine Development Team",
  "fastgltf files": [ ... ],
  "terrain": [ ... ],
  "terrainMultiMaterial": [ ... ]
}
```

---

### 3.1.1 Root Fields

| Field | Type | Required | Description |
|-----|-----|----------|-------------|
| `file format identifier` | number | REQUIRED | Scene format version |
| `description` | string | OPTIONAL | Human-readable description |
| `author` | string | OPTIONAL | Author or copyright string |
| `fastgltf files` | array | OPTIONAL | glTF/GLB model references |
| `terrain` | array | OPTIONAL | Heightmap terrain descriptors |
| `terrainMultiMaterial` | array | OPTIONAL | Multi-material terrain descriptors |

---

## 4. Asset Collections

Each asset collection is an **array of asset entries**.

### 4.1 fastgltf files

Defines instanced glTF / GLB assets loaded via FastGLTF.

```jsonc
"fastgltf files": [
  {
    "filename": "application/lucre/models/mario/kart.glb",
    "instances": [ ... ]
  }
]
```

#### Fields

| Field | Type | Required |
|-----|-----|----------|
| `filename` | string | REQUIRED |
| `instances` | array | REQUIRED |

---

### 4.2 terrain

Defines heightmap-based terrain.

```jsonc
"terrain": [
  {
    "filename": "application/lucre/terrainDescriptions/heightmap2.json",
    "instances": [ ... ]
  }
]
```

---

### 4.3 terrainMultiMaterial

Defines terrain using layered materials.

```jsonc
"terrainMultiMaterial": [
  {
    "filename": "application/lucre/terrainDescriptions/terrain1.json",
    "instances": [ ... ]
  }
]
```

---

## 5. Instances

Each asset entry MAY define one or more instances.

```jsonc
"instances": [
  {
    "transform": { ... }
  }
]
```

---

## 6. Transform Object

All instances MUST define a `transform`.

```jsonc
"transform": {
  "scale": [1, 1, 1],
  "rotation": [0, 0, 0],
  "translation": [0, 0, 0]
}
```

### 6.1 Transform Semantics

| Field | Type | Required | Description |
|-----|-----|----------|-------------|
| `scale` | array[3] | REQUIRED | Non-uniform scale |
| `rotation` | array[3] | REQUIRED | Euler rotation (radians) |
| `translation` | array[3] | REQUIRED | World-space translation |

- Rotation order is engine-defined.
- Units are meters and radians.

---

## 7. Coordinate System

- Right-handed coordinate system
- +Y is up
- Units are meters
- Angles are radians

---

## 8. Path Resolution

- All filenames are resolved relative to the Lucre runtime working directory.
- Absolute paths MAY be used but are discouraged.

---

## 9. Versioning

- `file format identifier` controls compatibility.
- Unknown major versions MUST be rejected.
- Minor version differences MAY be accepted if backward compatible.

---

## 10. Serialization Guarantees

The Lucre serializer guarantees:
- Stable field ordering (for diffs)
- No loss of numeric precision beyond JSON limits
- Round-trip safety (serialize → deserialize)

---

## 11. Non-Goals

This format intentionally does NOT encode:
- Rendering pipelines
- Materials (handled by assets)
- Physics runtime state
- ECS entity IDs
- Animation state

---

## 12. Example Minimal Scene

```json
{
  "file format identifier": 1.2,
  "description": "Minimal scene",
  "fastgltf files": [
    {
      "filename": "models/cube.glb",
      "instances": [
        {
          "transform": {
            "scale": [1, 1, 1],
            "rotation": [0, 0, 0],
            "translation": [0, 0, 0]
          }
        }
      ]
    }
  ]
}
```

---

## 13. Future Extensions

Planned extensions MAY include:
- Named instances
- Physics metadata
- Light descriptors
- Scene graph hierarchy
- Streaming / chunked scenes

All extensions MUST remain backward compatible or bump the format identifier.
