# Camera navigation

Open the Camera section of ForgeSim Debug for navigation controls. Scene framing
preserves authored model scale and world coordinates.

- Automatic fly speed is derived from the bounds radius, independently of viewport
  aspect ratio, with conservative limits for very small and large imports.
- Fly speed can be adjusted with a logarithmic slider; Ctrl-click to type a value.
- Hold RMB and use WASD to fly. Hold Ctrl while flying for one tenth the speed.
- Near and far clipping can be entered manually; press Enter to apply each field.
  Values must be finite and satisfy `0 < near < far`.
- Frame scene resets framing and navigation defaults from all scene objects.
- Focus selected object frames the selected object's bounds and resets speed for
  its scale. This operates on scene objects, not individual submeshes in a GLB.

Automatic framing now limits near clipping to avoid scaling it to large fractions
of an interior space. Very small near planes combined with distant far planes
can reduce depth precision, so adjust the range to the area you are inspecting.
Existing saved viewpoints and bookmarks retain their authored clipping values.
Save the current viewpoint explicitly to preserve changed camera clipping/pose;
fly speed is a session navigation setting, not authored scene data.

A single GLB containing a complete city still frames the complete city. For more
useful initial framing, export the intended area or save a viewpoint inside it.
Camera controls do not provide collision detection: they do not prevent flying
through walls. A wall disappearing before the camera reaches it is a different
issue and can be caused by near clipping.
