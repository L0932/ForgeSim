# Viewport display preferences

In **ForgeSim Debug > Viewport**, set **Show reference grid**, **Show scene lights**
(light guides) and **Show object bounds**. These editor preferences survive opening
or creating scenes and restarting ForgeSim. They do not change actual scene lighting.

Changes are saved immediately to `Settings/viewport-display.json` in the ForgeSim
user data directory, beside `imgui.ini` and `viewport-tools.json`. They do not mark
the scene dirty and are not stored in bundled scene documents or undo history.
Environment background visibility remains a per-scene control under **Environment asset**.

On first launch the grid and light guides are visible, and object bounds are hidden.
Missing or invalid settings fall back to those defaults. If saving fails, the editor
keeps the current values for the session and reports a warning in the log.

For screenshots without guides, disable all three checkboxes and use **Shift+H**
to hide selection highlighting while keeping the selection.
