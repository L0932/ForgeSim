# Camera navigation

Open the Camera section of ForgeSim Debug for navigation controls. Scene framing
preserves authored model scale and world coordinates.

- Fly speed defaults to **10 units/s** for new scenes and files without a saved
  `cameraMovementSpeed` (including null). It does not depend on model scale.
- Adjust fly speed with the logarithmic slider; Ctrl-click to type a value.
  Changes are saved with the scene. **Reset fly speed to 10** clears the override.
- Hold RMB and use WASD to fly. While flying, scroll up to increase speed by 25%
  per notch, or down to decrease it by the inverse factor. Fractional wheel input
  is supported. Hold Ctrl while flying for one tenth the speed.
- Wheel speed changes use the same per-scene manual override as the slider:
  they persist when you save. Scrolling outside
  active fly navigation does not change camera speed.
- Near and far clipping can be entered manually; press Enter to apply each field.
  Values must be finite and satisfy `0 < near < far`.
- Frame scene resets framing from all scene objects. Manual fly-speed overrides
  remain in effect.
- Focus selected object frames the selected object's bounds and chooses speed for
  the default 10 unless a manual speed is saved. This operates on scene objects, not individual submeshes in a GLB.

Automatic framing now limits near clipping to avoid scaling it to large fractions
of an interior space. Very small near planes combined with distant far planes
can reduce depth precision, so adjust the range to the area you are inspecting.
Existing saved viewpoints and bookmarks retain their authored clipping values.
Click **Capture current viewpoint** to store changed camera clipping/pose, then
save the scene file. Scene Save writes that captured viewpoint rather than your
current navigation pose. Fly speed is saved independently of viewpoint capture.
Older scene files without `cameraMovementSpeed` use 10 units/s; older builds
may ignore this optional navigation setting when reading or resaving a scene.

A single GLB containing a complete city still frames the complete city. For more
useful initial framing, export the intended area or save a viewpoint inside it.
Camera controls do not provide collision detection: they do not prevent flying
through walls. A wall disappearing before the camera reaches it is a different
issue and can be caused by near clipping.

Opening or creating a scene resets navigation to that scene's saved fly speed,
or uses 10 units/s when no speed is saved.
This also applies after importing models, independently of camera viewpoint
restoration. A manual speed remains attached to the current scene.
