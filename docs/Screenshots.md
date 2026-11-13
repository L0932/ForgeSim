# Screenshots

Use **Capture > Scene viewport PNG** (**F9**) for a PNG at the Scene render
resolution, or **Capture > Entire editor PNG** (**Ctrl+F9**) for the full rendered
editor client area. Native operating-system window borders and mouse cursors are
not part of the editor image. Shortcuts pause during navigation, text entry,
active edits and popup menus.

Scene images include tone mapping and anti-aliasing. They exclude ImGui windows,
viewport tools and gizmo handles. Rendered grid lines, light guides, object bounds
and selection tint follow the existing display settings. Disable those guides
and use **Shift+H** to hide selection highlighting when preparing a clean preview.

Use **Capture > Maximize Scene viewport**, or focus Scene and press **Shift+Space**,
to fill the workspace. Repeat to restore the previous dock layout. **Escape** also
restores a maximized Scene; press Escape again to exit an active orbit mode.
Maximizing changes the render resolution to match the available physical pixels.
It does not change scene camera settings or persist a new dock layout.

Menu captures wait a full frame for the menu to close. Images are read before
buffer swap, and success/failure feedback and Console messages are emitted only
after readback. Screenshot notifications are hidden while another capture is
pending, so the current capture does not include its own feedback.

Choose **Capture > Screenshot folder...** to select an output directory. The
folder choice is saved as an editor preference and is independent of scene files.
The default is `ForgeSim/Screenshots` beside the editor's `Settings` directory in
the platform user-data location. Capture shows the full output path and dimensions
in both an on-screen notification and the Console. PNG filenames include the
capture target, UTC timestamp and a collision suffix when needed. Screenshotting does not mark the scene modified.

For bundled sample previews, capture Scene with guides hidden, inspect the PNG,
then copy it into the sample's preview assets through the normal packaging work.
