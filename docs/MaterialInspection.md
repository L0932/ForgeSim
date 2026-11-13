# Material inspection

Select a model instance or built-in cube in Scene or Hierarchy, then expand
**Materials** in the Inspector. This section is read-only and does not modify
the model, scene, or shared material resources.

Each asset material lists its base RGBA, metallic/roughness factors, emissive
RGB, alpha mode/cutoff, double-sided setting, normal scale and occlusion strength.
Maps are shown for base color, metallic/roughness, normal, occlusion and emissive.
Texture cards reflow into one to three columns as the Inspector is resized or
docked. Hover a thumbnail for a larger preview. Each assigned map lists its zero-based
texture/image indices, dimensions, channel count and UV set. These identify
embedded GLB images too; source filenames and material names are not currently
retained by the asset loader. Material numbering in the heading starts at one.

Metallic/roughness maps pack roughness in green and metallic in blue; occlusion
uses red. The packed RGB image is shown as-is, rather than as individual grayscale
channels. Color textures use sRGB; normal and other data maps use linear values.
Factors multiply texture values, so a map alone does not describe the final result.
Normal scale and occlusion strength also affect the resulting shading.

Only assigned maps occupy grid cells; unassigned slots are summarized underneath. An asset without authored materials uses
the default glTF material. Asset materials unused by the rendered scene may still
appear in this list and have no GPU preview. The list describes loaded assets;
when texture fallbacks are enabled, recovered maps may contain fallback imagery.

Repeated imports and duplicated objects share asset materials. Switching selection
shows the selected model's resources; this view does not retain GPU handles across
scene replacements. Use Blender or another asset editor to change source materials.
