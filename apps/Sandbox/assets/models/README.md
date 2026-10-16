# Sandbox Model Assets

## BoxVertexColors.glb

A glTF 2.0 unit cube using per-vertex colors. The model is used to
demonstrate ForgeSim's initial static-mesh loading path.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/BoxVertexColors
- Author: Marco Hutter
- Copyright: Public, 2023
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: None

## BoxTextured.glb

A glTF 2.0 unit cube containing texture coordinates. The model is used
to validate ForgeSim's texture-coordinate import and texture-sampling
path.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/BoxTextured
- Author: Cesium
- Copyright: Cesium, 2017
- License: Creative Commons Attribution 4.0 International (CC-BY-4.0)
- License URL: https://creativecommons.org/licenses/by/4.0/
- Modifications: None

## Mesh_Primitives_00.gltf and Mesh_Primitives_00.bin

A glTF 2.0 validation asset containing one mesh with two primitives
and two base-color materials. It is used to validate ForgeSim's
material-aware render-submission path.

- Source: https://github.com/KhronosGroup/glTF-Asset-Generator/tree/main/Output/Positive/Mesh_Primitives
- Copyright: © 2017 Khronos Group
- License: MIT License
- License URL: https://github.com/KhronosGroup/glTF-Asset-Generator/blob/main/LICENSE
- Modifications: None

## Avocado.glb

A natural object using core glTF 2.0 materials, including base-color,
metallic-roughness, and normal textures. The model is used to validate
ForgeSim's initial PBR material and normal-mapping paths.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/Avocado
- Author: Microsoft
- Copyright: Public, 2017
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: None

## Metal-Rough Spheres (textureless)

A glTF 2.0 material test asset containing spheres with varying
metallic and roughness factors. It is used to validate ForgeSim's
PBR material and image-based-lighting implementation.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/MetalRoughSpheresNoTextures
- Author: Krill Gavrilov
- Copyright: Public, 2019
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: None

## Alpha Blend Mode Test

A glTF 2.0 validation asset covering `OPAQUE`, `BLEND`, and `MASK`
materials, including multiple alpha-cutoff values.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/AlphaBlendModeTest
- Author: Ed Mackey
- Copyright: © 2018 Analytical Graphics, Inc.
- License: Creative Commons Attribution 4.0 International
- License URL: https://creativecommons.org/licenses/by/4.0/
- Modifications: None

## Texture Settings Test

A glTF 2.0 validation asset used to verify single-sided and
double-sided rendering, back-face culling, and texture sampler modes.

- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/TextureSettingsTest
- Author: Ed Mackey
- Copyright: © 2017 Analytical Graphics, Inc.
- License: Creative Commons Attribution 4.0 International
- License URL: https://creativecommons.org/licenses/by/4.0/
- Modifications: None

## ForgeSim PBR Material Gallery

A ForgeSim-owned visual-validation scene derived from the Khronos
Metal-Rough Spheres sample. The gallery demonstrates supported glTF
metallic-roughness material inputs under consistent lighting and
rendering conditions.

The initial version preserves the original dielectric and metallic
sphere rows while providing a separate asset that can be expanded with
base-color, normal, occlusion, emissive, texture, and fallback-material
examples.

- Based on: MetalRoughSpheresNoTextures
- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/MetalRoughSpheresNoTextures
- Original author: Krill Gavrilov
- Original copyright: Public, 2019
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: Renamed and prepared as the foundation of ForgeSim's expanded PBR material gallery.

## PbrMaterialGallery.glb

A ForgeSim-owned glTF 2.0 visual-validation scene derived from the
Khronos Metal-Rough Spheres sample. The gallery demonstrates supported
metallic-roughness material inputs under consistent lighting and
rendering conditions.

The gallery includes:

- Dielectric and metallic material comparisons.
- A progression from smooth to rough surfaces.
- Base-color factors and textures.
- Normal-map comparison samples.
- Ambient-occlusion comparison samples.
- Emissive-factor and emissive-texture samples.
- Materials without optional textures for fallback validation.
- Metal-plate and lava showcase materials combining multiple maps.

### Original Metal-Rough Spheres Asset

- Based on: MetalRoughSpheresNoTextures
- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/MetalRoughSpheresNoTextures
- Original author: Krill Gavrilov
- Original copyright: Public, 2019
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: Rearranged and expanded with additional geometry,
  labels, materials, textures, and validation samples for ForgeSim.

### BoomBox Normal Texture

A normal texture from the Khronos BoomBox sample is used by the
normal-mapping comparison.

- Source asset: BoomBox
- Source: https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/BoomBox
- Original creator: Microsoft
- Original copyright: Public, 2017
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: Applied to ForgeSim's sphere geometry and embedded in
  the material-gallery GLB.

### Metal Plate Texture Set

The Metal Plate texture set is used by the combined metal-material
showcase. The sample uses its base-color, roughness, OpenGL normal, and
ambient-occlusion maps.

- Asset: Metal Plate
- Source: https://polyhaven.com/a/metal_plate
- Creator: Rob Tuytel
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: Applied to ForgeSim's sphere geometry and embedded in
  the material-gallery GLB.

### Lava001 Texture Set

The Lava001 texture set is used by the combined emissive-material
showcase. The sample uses its base-color, roughness, OpenGL normal, and
emission maps.

- Asset: Lava001
- Source: https://ambientcg.com/a/Lava001
- Creator: ambientCG
- License: Creative Commons Zero v1.0 Universal (CC0-1.0)
- License URL: https://creativecommons.org/publicdomain/zero/1.0/
- Modifications: Applied to ForgeSim's sphere geometry and embedded in
  the material-gallery GLB.

### ForgeSim Test Textures

The checkerboard and other project-created validation textures were
created specifically for ForgeSim to exercise base-color, emissive,
and missing-texture rendering behavior.

- Source: ForgeSim project
- License: Covered by the ForgeSim repository license
- Modifications: Embedded in the material-gallery GLB.