# Keyboard controls

Open **Help > Keyboard Controls...** for the searchable in-editor reference.
The rows below match `apps/Sandbox/KeyboardControls.hpp`, the catalog used by Help.
Update both when bindings change; list only implemented commands.

| Category | Shortcut | Action | Context |
| --- | --- | --- | --- |
| Camera | RMB + mouse | Look around | Hold RMB over Scene to begin navigation |
| Camera | RMB + W / A / S / D | Fly forward / left / backward / right | Scene navigation |
| Camera | RMB + wheel | Increase / decrease fly speed | Scene navigation; saved with scene |
| Camera | Ctrl while flying | Move at one tenth of fly speed | Scene navigation |
| Selection | Left click | Select an object or light | Scene or Hierarchy; empty Scene space clears selection |
| Selection | F | Focus selected object or light | Scene or Hierarchy focused; navigation stopped |
| Transforms | W | Move tool | Navigation stopped; no text entry or active edit |
| Transforms | E | Rotate tool | Navigation stopped; no text entry or active edit |
| Transforms | R | Scale tool | Navigation stopped; no text entry or active edit |
| Transforms | X | Toggle world / local axes | Navigation stopped; no text entry or active edit |
| History | Ctrl+Z | Undo transform or pivot edit | No text entry, active edit, or scene-change confirmation |
| History | Ctrl+Y / Ctrl+Shift+Z | Redo transform or pivot edit | No text entry, active edit, or scene-change confirmation |
| Files | Ctrl+S | Save scene / choose destination | No text entry, active edit, navigation, or confirmation |
| Panels | F1 | Show / hide ForgeSim Debug | No text entry or active control |
| Console | Enter | Execute command | Console input focused |
| Console | Up / Down | Recall command history | Console input focused |
| Fields | Ctrl+click slider | Type a precise value | Supported numeric sliders, including fly speed |
| Fields | Enter | Apply clipping value | Near clip or Far clip field focused |

## Selection focus

Click the Scene viewport or a Hierarchy row, then press **F** to focus the selected
model instance or light. This changes only the live camera viewpoint. It preserves
fly speed and does not mark the scene modified or overwrite a saved viewpoint.
The Camera panel's **Focus selection (F)** button performs the same operation.

Model focus uses its own transformed bounds, including its pivot and scale.
Directional lights frame their editor marker; point and spot lights frame a
neighborhood of their position sized from their range and the scene scale.

F is suppressed while typing, editing a numeric control, using a gizmo, navigating
with RMB, using modifier keys, or while a popup/scene-change dialog is open. It
does not fire from the Console or from an unrelated panel. With no selection it
does nothing. Internal-node focus will follow when node selection is implemented.

Deletion and duplication shortcuts will be documented when those operations are
implemented. They are not available in this first scene-authoring patch.
