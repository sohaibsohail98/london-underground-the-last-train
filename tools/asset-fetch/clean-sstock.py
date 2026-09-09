#!/usr/bin/env python3
"""De-brand the CC-BY S Stock glTF so it can live in this repository.

The Sketchfab download ships two texture sets this project must never carry:
the operator's circle-and-bar station mark, and a door warning sign set in the
operator's corporate typeface. Both are covered by the hard constraints in
CLAUDE.md and docs/art-direction.md, so neither may be committed, and neither
may be shipped in a build.

CC-BY 4.0 permits modification provided the change is declared, so this script
is the declaration: it is the exact, repeatable transform from the untouched
Sketchfab download to what SourceArt/ThirdParty/SStock/ contains.

The geometry those materials are assigned to is kept. It is 168 triangles of
flat quad on the cab front and the door leaves, and it is where Phase F5 mounts
the original station mark that replaces what was stripped.

Usage:
    python3 tools/asset-fetch/clean-sstock.py <source-dir> <output-dir>

<source-dir> is an unpacked Sketchfab glTF download containing scene.gltf,
scene.bin, license.txt and textures/.
"""

from __future__ import annotations

import json
import shutil
import sys
from pathlib import Path

# Texture files that must never enter the repository or a build, by URI as it
# appears in the Sketchfab scene.gltf.
STRIP_IMAGES = {
    "textures/Underground_Logo_baseColor.png",
    "textures/Underground_Logo_metallicRoughness.png",
    "textures/Material.001_baseColor.png",
}

# What a stripped material becomes. Flat unbranded painted metal, so the mesh
# still reads as a panel and Phase F5 has something to reassign.
NEUTRAL_BASE_COLOUR = [0.82, 0.82, 0.82, 1.0]

# Stripped materials are renamed so the import creates an obviously empty slot
# rather than one named after the mark that is never going back into it.
RENAME_MATERIALS = {
    "Underground_Logo": "Station_Mark_Placeholder",
    "Material.001": "Door_Warning_Placeholder",
}

EXPECTED_TRIANGLES = 579544


def triangle_count(gltf: dict) -> int:
    total = 0
    for mesh in gltf["meshes"]:
        for primitive in mesh["primitives"]:
            if "indices" in primitive:
                total += gltf["accessors"][primitive["indices"]]["count"] // 3
    return total


def neutralise(material: dict) -> None:
    """Drop every texture reference and leave flat painted metal behind."""
    pbr = material.setdefault("pbrMetallicRoughness", {})
    for key in ("baseColorTexture", "metallicRoughnessTexture"):
        pbr.pop(key, None)
    for key in ("normalTexture", "occlusionTexture", "emissiveTexture"):
        material.pop(key, None)
    pbr["baseColorFactor"] = list(NEUTRAL_BASE_COLOUR)
    pbr.setdefault("metallicFactor", 0.0)
    pbr.setdefault("roughnessFactor", 0.5)


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__.strip())
        return 2

    source = Path(sys.argv[1])
    output = Path(sys.argv[2])

    scene_path = source / "scene.gltf"
    if not scene_path.is_file():
        print(f"error: {scene_path} does not exist")
        return 1

    gltf = json.loads(scene_path.read_text(encoding="utf-8"))

    before = triangle_count(gltf)
    if before != EXPECTED_TRIANGLES:
        print(f"warning: expected {EXPECTED_TRIANGLES} triangles, found {before}")

    images = gltf.get("images", [])
    strip_image_indices = {
        i for i, image in enumerate(images) if image.get("uri") in STRIP_IMAGES
    }
    missing = STRIP_IMAGES - {images[i].get("uri") for i in strip_image_indices}
    if missing:
        print(f"error: source does not contain {sorted(missing)}; wrong download?")
        return 1

    textures = gltf.get("textures", [])
    strip_texture_indices = {
        i for i, texture in enumerate(textures)
        if texture.get("source") in strip_image_indices
    }

    # Any material pointing at a stripped texture loses every map it has. A
    # material carrying one forbidden map is not trustworthy in its other slots.
    stripped_materials = []
    for material in gltf.get("materials", []):
        pbr = material.get("pbrMetallicRoughness", {})
        referenced = {
            slot.get("index")
            for slot in (
                pbr.get("baseColorTexture"),
                pbr.get("metallicRoughnessTexture"),
                material.get("normalTexture"),
                material.get("occlusionTexture"),
                material.get("emissiveTexture"),
            )
            if isinstance(slot, dict)
        }
        if referenced & strip_texture_indices:
            old_name = material.get("name", "<unnamed>")
            new_name = RENAME_MATERIALS.get(old_name, old_name)
            material["name"] = new_name
            stripped_materials.append(f"{old_name} -> {new_name}")
            neutralise(material)

    # Rebuild images and textures without the stripped entries, then remap every
    # surviving index. Order is preserved so the diff stays readable.
    keep_images = [i for i in range(len(images)) if i not in strip_image_indices]
    image_remap = {old: new for new, old in enumerate(keep_images)}
    gltf["images"] = [images[i] for i in keep_images]

    keep_textures = [i for i in range(len(textures)) if i not in strip_texture_indices]
    texture_remap = {old: new for new, old in enumerate(keep_textures)}
    gltf["textures"] = [
        {**textures[i], "source": image_remap[textures[i]["source"]]}
        for i in keep_textures
    ]

    for material in gltf.get("materials", []):
        pbr = material.get("pbrMetallicRoughness", {})
        for holder, key in (
            (pbr, "baseColorTexture"),
            (pbr, "metallicRoughnessTexture"),
            (material, "normalTexture"),
            (material, "occlusionTexture"),
            (material, "emissiveTexture"),
        ):
            slot = holder.get(key)
            if isinstance(slot, dict) and "index" in slot:
                slot["index"] = texture_remap[slot["index"]]

    # Samplers orphaned by the removal go too, with the same remap treatment.
    samplers = gltf.get("samplers", [])
    used_samplers = sorted(
        {t["sampler"] for t in gltf["textures"] if "sampler" in t}
    )
    sampler_remap = {old: new for new, old in enumerate(used_samplers)}
    gltf["samplers"] = [samplers[i] for i in used_samplers]
    for texture in gltf["textures"]:
        if "sampler" in texture:
            texture["sampler"] = sampler_remap[texture["sampler"]]

    after = triangle_count(gltf)
    if after != before:
        print(f"error: triangle count changed, {before} to {after}")
        return 1

    output.mkdir(parents=True, exist_ok=True)
    (output / "textures").mkdir(exist_ok=True)

    (output / "scene.gltf").write_text(
        json.dumps(gltf, indent=1) + "\n", encoding="utf-8"
    )
    shutil.copy2(source / "scene.bin", output / "scene.bin")

    copied = []
    for image in gltf["images"]:
        uri = image["uri"]
        shutil.copy2(source / uri, output / uri)
        copied.append(uri)

    print(f"wrote {output}")
    print(f"  triangles kept:   {after}")
    print(f"  textures removed: {sorted(STRIP_IMAGES)}")
    print(f"  textures kept:    {copied}")
    print(f"  materials reset:  {stripped_materials}")
    print()
    print("license.txt is not copied. Keep the attribution file that ships beside")
    print("the download, and record the modification made here alongside it.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
