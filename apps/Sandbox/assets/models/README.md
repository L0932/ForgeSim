# Sandbox Model Assets

This directory contains third-party sample assets and ForgeSim-owned
validation assets used by the Sandbox. Each asset's purpose, source,
license, and modifications are recorded below.

## BoxVertexColors.glb

A glTF 2.0 unit cube using per-vertex colors. The model validates
ForgeSim's initial static-mesh loading and vertex-color paths.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/BoxVertexColors
- Author: Marco Hutter
- Copyright: Public, 2023
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: None

## BoxTextured.glb

A glTF 2.0 unit cube containing texture coordinates. The model
validates ForgeSim's texture-coordinate import and texture-sampling
paths.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/BoxTextured
- Author: Cesium
- Copyright: Cesium, 2017
- License: Creative Commons Attribution 4.0 International (CC-BY-4.0)
- License URL: https://creativecommons.org/licenses/by/4.0/
- Modifications: None

## Mesh_Primitives_00.gltf and Mesh_Primitives_00.bin

A glTF 2.0 validation asset containing one mesh with two primitives
and two base-color materials. It validates ForgeSim's multi-primitive,
material-aware render-submission path.

- Source: https://github.com/KhronosGroup/glTF-Asset-Generator/tree/main/Output/Positive/Mesh_Primitives
- Copyright: © 2017 Khronos Group
- License: MIT License
- License URL: https://github.com/KhronosGroup/glTF-Asset-Generator/blob/main/LICENSE
- Modifications: None

## Avocado.glb

A natural object using core glTF 2.0 materials, including base-color,
metallic-roughness, and normal textures. The model validates
ForgeSim's PBR material-import and normal-mapping paths on
non-primitive geometry.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/Avocado
- Author: Microsoft
- Copyright: Public, 2017
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: None

## MetalRoughSpheresNoTextures.glb

A glTF 2.0 material test asset containing spheres with varying
metallic and roughness factors. It validates ForgeSim's PBR material
and image-based-lighting implementation without relying on material
textures.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/MetalRoughSpheresNoTextures
- Author: Krill Gavrilov
- Copyright: Public, 2019
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: None

## AlphaBlendModeTest.glb

A glTF 2.0 validation asset covering `OPAQUE`, `BLEND`, and `MASK`
materials, including multiple alpha-cutoff values.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/AlphaBlendModeTest
- Author: Ed Mackey
- Copyright: © 2018 Analytical Graphics, Inc.
- License: Creative Commons Attribution 4.0 International
- License URL: https://creativecommons.org/licenses/by/4.0/
- Modifications: None

## TextureSettingsTest.glb

A glTF 2.0 validation asset used to verify single-sided and
double-sided rendering, back-face culling, and texture-sampler modes.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/TextureSettingsTest
- Author: Ed Mackey
- Copyright: © 2017 Analytical Graphics, Inc.
- License: Creative Commons Attribution 4.0 International
- License URL: https://creativecommons.org/licenses/by/4.0/
- Modifications: None

## PbrMaterialGallery.glb

A ForgeSim-owned glTF 2.0 visual-validation scene derived from the
Khronos Metal-Rough Spheres sample. It contains material comparisons
used to validate ForgeSim's glTF metallic-roughness, texture,
normal-mapping, ambient-occlusion, emissive, fallback-resource, and
image-based-lighting paths.

### Original Metal-Rough Spheres Asset

The original dielectric and metallic sphere rows are based on
`MetalRoughSpheresNoTextures`.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/MetalRoughSpheresNoTextures
- Original author: Krill Gavrilov
- Original copyright: Public, 2019
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: Rearranged and expanded with additional geometry,
  materials, textures, comparison groups, and ForgeSim validation
  samples.

### BoomBox Normal Texture

A normal texture from the Khronos BoomBox sample is used by the
normal-mapping comparison.

- Source asset: BoomBox
- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/BoomBox
- Original creator: Microsoft
- Original copyright: Public, 2017
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: Applied to ForgeSim sphere geometry and embedded in
  the material-gallery GLB.

### Metal Plate Texture Set

The Metal Plate texture set is used by the combined metal-material
showcase. The material uses its base-color, roughness, OpenGL normal,
and ambient-occlusion maps.

- Asset: Metal Plate
- Source: https://polyhaven.com/a/metal_plate
- Creator: Rob Tuytel
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: Applied to ForgeSim sphere geometry and embedded in
  the material-gallery GLB.

### Lava001 Texture Set

The Lava001 texture set is used by the combined emissive-material
showcase. The material uses its base-color, roughness, OpenGL normal,
and emission maps.

- Asset: Lava001
- Source: https://ambientcg.com/a/Lava001
- Creator: ambientCG
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: Applied to ForgeSim sphere geometry and embedded in
  the material-gallery GLB.

### ForgeSim Test Textures

The checkerboard and other project-created validation textures were
created for ForgeSim to exercise base-color, emissive, occlusion, and
missing-texture rendering behavior.

- Source: Original ForgeSim project assets
- Third-party attribution: None
- License: No separate public license is currently granted
- Modifications: Embedded in the material-gallery GLB

## Intel Sponza Base Scene

ForgeSim supports the Intel Sponza Base Scene as an optional external
showcase asset. The scene is used to validate large glTF environments,
multi-mesh rendering, material and texture handling, scene transforms,
and image-based lighting.

The asset is not distributed with ForgeSim. Download it from Intel's
GPU Research Samples page and follow the ForgeSim showcase preparation
instructions before launching the Sandbox.

- Source: https://www.intel.com/content/www/us/en/developer/topic-technology/graphics-research/samples.html
- Creator and distributor: Intel Corporation
- License: Creative Commons Attribution, as stated by Intel's download page
- ForgeSim modifications: Optional derived copy with texture dimensions reduced to a maximum of 1024 pixels
- Redistribution: Not included in the ForgeSim repository