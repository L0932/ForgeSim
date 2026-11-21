# Display transforms and lighting comparisons

In the Debug panel's **Rendering** section, **Display transform** selects how
linear HDR scene colors are displayed:

| Mode | Output |
| --- | --- |
| ACES approximation (default) | Existing fitted ACES curve, followed by sRGB encoding |
| Standard / sRGB | Exposure, clipping to [0, 1], and sRGB encoding, without the fitted ACES curve |

Standard does not compress bright HDR highlights; values above 1 become display
white. It is useful for a controlled comparison and for distinguishing lighting
from tone mapping, but may look clipped under bright lighting. Neither mode
changes material textures, material factors, lighting calculations, or Inspector
texture previews. Both apply to the HDR scene, including the skybox, before FXAA
and viewport presentation. The selected mode also applies to viewport captures.

Exposure is a linear multiplier. A value of 1 is neutral; Blender's exposure of
0 stops is neutral. Blender exposure E corresponds to a multiplier of 2^E.
Environment intensity separately scales environment lighting and the skybox.

## Persistence

The optional scene JSON field `rendering.toneMapping` accepts `acesApproximation`
or `standard`. Omission retains the ACES approximation for older scenes. Unknown
values are rejected. Save/Open preserves the mode; New Scene resets it to ACES.
No scene version increment is needed for this optional field.

## Cannon comparison for issue #133

Use the same exported cannon GLB and `clarens_midday_4k.hdr` in both engines.
Disable every direct light, disable ForgeSim selection highlighting, and align the
camera and recognizable environment landmarks. An identical numeric rotation is
not sufficient if the engines use different coordinate conventions.

| Setting | ForgeSim | Blender |
| --- | --- | --- |
| Display transform | Standard / sRGB | Standard |
| Exposure | 1 | 0 stops |
| Environment strength | 1 | World Background Strength 1 |
| Display encoding | sRGB | sRGB |
| Working space | Linear Rec.709 RGB pipeline | Linear Rec.709 |
| Additional grading | None | Look None, Gamma 1, no curves or white-balance adjustments |

Use Rendered viewport shading in Blender and the scene World. Equal strength
settings provide a baseline, rather than proof that the lighting integrations
match. Keep track of environment orientation, model rotation, normals, camera
angle, and the chosen render engine.

This Standard path is a baseline for a scene-linear Rec.709 to SDR sRGB display.
It does not reproduce Blender's complete color-management system, HDR/OS display
management, AgX, or ACES view transforms. ForgeSim's fitted ACES approximation is
also not interchangeable with Blender's ACES view transforms.

Compare the isolated cannon first. Do not tune the material's metallic/roughness
factors to compensate for display differences. Remaining discrepancies may require
checking import data, normal mapping, environment convolution, mip selection,
and BRDF integration against the same lighting reference.
