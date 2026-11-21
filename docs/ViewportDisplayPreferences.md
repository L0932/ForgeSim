# Viewport display settings

In **ForgeSim Debug > Viewport**, set **Show reference grid**, **Show scene lights**
(light guides) and **Show object bounds**. The Camera and Selection panels also
provide independent **Show selection highlight** and **Show transform gizmo** controls.

All five visibility settings are stored in the scene file under `rendering.viewport`.
Changing a checkbox or using **Shift+H** marks the scene modified. Save the scene
to keep the settings; reopening it restores them even when the Debug panel is hidden.
**Shift+H** hides both selection overlays if either is visible, or shows both if both
are hidden. It keeps the selection active.

New scenes and older files without these settings use the defaults: reference grid,
light guides, selection highlight and transform gizmo visible; object bounds hidden.
The previous global `Settings/viewport-display.json` file is no longer read or written.
These visibility controls do not change actual scene lighting.

The **Display transform** choice (ACES approximation or Standard / sRGB) is also
stored per scene, under `rendering.toneMapping`, along with exposure and environment
settings. Older files without a display transform use ACES approximation.
