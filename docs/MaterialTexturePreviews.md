# Material texture previews

The Inspector displays source texture content, independently of lit material
appearance. Previews do not apply material factors, UV transforms, lighting,
exposure, or scene tone mapping. RGB previews ignore alpha so transparent texels
do not hide their stored colors. The material's alpha behavior is unchanged.

## Display convention

Base-color and emissive images contain sRGB-encoded RGB. The preview copies those
encoded values into a separate RGBA8 texture, without hardware sRGB decoding, for
the existing SDR ImGui output path. This avoids displaying decoded linear values
as though they were encoded display values. It does not change the sRGB textures
used for scene rendering or modify the decoded source image.

Normal, occlusion, and packed metallic/roughness maps show raw stored RGB values.
They are data visualizations, without an additional sRGB transfer or material
factors. A raw channel value of 0.5 displays as approximately byte 128, rather than
being converted to sRGB byte 188. These thumbnails are not shaded surface colors.

The metallic/roughness map has a **View** selector:

- **Packed RGB**: the stored combined image.
- **Roughness (G)**: green channel repeated into RGB.
- **Metallic (B)**: blue channel repeated into RGB.

In grayscale views, black means 0 and white means 1. The material factors remain
listed above the thumbnails. A texture reused in multiple slots or materials
shares its preview by source image and view mode; rendering color-space choices
remain independent.

## Resource behavior

Previews are created lazily when displayed, with a maximum dimension of 512
pixels. Larger images use nearest source sampling during preview construction;
the UI texture uses linear filtering. The hover preview uses the same texture.
Click a thumbnail to open **Texture inspection** with a separate full-resolution
texture. This view does not reduce the source dimensions and uses nearest
filtering so individual texels remain distinguishable when zoomed in.

- **Fit** keeps the complete image centered and scaled when the window is resized.
- **1:1** centers the image at one source pixel per UI unit and enters Manual mode.
- The mouse wheel zooms around the pointer; left or middle drag pans. Both enter
  Manual mode, which preserves zoom and pan across window resizing. Press **Fit**
  to resume automatic fitting.
- Hovering shows original RGBA byte values and normalized values. Pixel
  coordinates use a top-left origin. Packed maps also report G as roughness and
  B as metallic. Factors and display transforms are not included in these values.
- The channel selector switches packed RGB, roughness, and metallic in place.

Only the most recently requested full-resolution view is retained across frames;
unused full-resolution textures are released after queued UI commands have been
submitted. Thumbnails remain cached separately. Closing the window or changing
selection to another asset clears the inspection view; resource replacement also
clears it, including when the source object's address is reused.

The renderer owns the cache. Successful scene/model binding changes invalidate
it. Old previews survive until the next render frame, after queued ImGui commands
have been submitted. Failed preview creation logs once per cache generation and
shows an unavailable label without interrupting scene rendering.

## Manual verification

1. Open the cannon or brass-pot GLB and inspect its base-color map. Compare the
   same exported image in an image editor displaying sRGB RGB without a scene
   view transform, exposure adjustment, or alpha compositing.
2. Inspect an emissive map if present; it uses the same source-color convention.
3. Switch a packed metallic/roughness map between all three views. Click the
   thumbnail, choose 1:1, and zoom/pan to a small feature. Confirm that detail and
   the hovered source pixel values match the **same exported GLB** in the other
   tool, accounting for its pixel coordinate origin. Scene lighting and material
   factors must not change these previews. Different display conventions can
   change grayscale appearance without changing the raw channel values.
4. Change selection, import another model, and replace the scene. Previews should
   refresh without stale images or affecting the models' appearance.

These preview corrections do not establish a cause for dark metal in the scene.
Issue #133 still requires a controlled comparison of the same exported GLB,
environment, lighting, camera, exposure, and documented display transforms.
