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
| Selection | Ctrl+D | Duplicate selected model, point light, or spot light | Scene or Hierarchy focused; navigation stopped |
| Selection | Delete | Delete selected model or light | Scene or Hierarchy focused; navigation stopped |
| Transforms | W | Move tool | Navigation stopped; no text entry or active edit |
| Transforms | E | Rotate tool | Navigation stopped; no text entry or active edit |
| Transforms | R | Scale tool | Navigation stopped; no text entry or active edit |
| Transforms | X | Toggle world / local axes | Navigation stopped; no text entry or active edit |
| History | Ctrl+Z | Undo transform, pivot, creation, or deletion | No text entry, active edit, or scene-change confirmation |
| History | Ctrl+Y / Ctrl+Shift+Z | Redo transform, pivot, creation, or deletion | No text entry, active edit, or scene-change confirmation |
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

Model focus fits its own transformed bounds, including its pivot and scale,
while preserving the current viewing direction and field of view. Far clipping
covers the complete scene so rotating after light focus does not cut off models.
Directional lights frame their editor marker; point and spot lights frame a
neighborhood of their position sized from their range and the scene scale.

F is suppressed while typing, editing a numeric control, using a gizmo, navigating
with RMB, using modifier keys, or while a popup/scene-change dialog is open. It
does not fire from the Console or from an unrelated panel. With no selection it
does nothing. Internal-node focus will follow when node selection is implemented.

## Deletion and undo

With Scene or Hierarchy focused, press **Delete** to remove the selected complete
model instance or light. The Hierarchy also provides a
**Delete selection** button. Internal glTF nodes are not independently deletable.

Directional lights are removed as scene entities, including when disabled or
hidden. Undo restores their authored properties, visibility, and identity.

**Ctrl+Z** undoes deletion and selects the restored entity. **Ctrl+Y** or
**Ctrl+Shift+Z** reapplies it. Undo restores IDs, authored properties, visibility,
and original list position. Saving writes the current scene state;
undo history itself is not saved. Existing transform/pivot edits use the same
history. New objects receive IDs that do not collide with deleted objects still
retained by undo.

Delete has the same typing, popup, modifier, navigation, and active-edit guards as
F. Empty or stale selections do nothing. Deleted models' resources are retained
for undo during the authoring session, including across further imports, and are
released when a different scene is opened or a new scene is created.

## Create lights

Choose **GameObject > Light > Directional Light / Point Light / Spot Light**.
New lights appear in front of the current camera, are selected for Inspector
editing, and participate in the same undo/redo history. Spot lights aim along the
camera's viewing direction; directional lights use it as their emission direction.

The renderer supports zero or one directional light. Its creation menu item is
disabled while one exists, and becomes available after deletion. Point and spot
lights can be created repeatedly; the renderer currently submits up to 16 active
lights of each local type.

Scenes without a directional light write `directionalLight: null`. Existing
scene files with a directional-light object remain supported. Older builds that
require the object cannot read the new null form.

Duplicating entities will follow in the next scene-authoring patch.

## Built-in cube

Use **GameObject > 3D Object > Cube** to create a plain, one-unit cube three
units in front of the camera. The new cube is selected and supports transforms,
F focus, deletion, undo/redo, and scene saving. Repeated cubes share mesh resources.
The reserved `@builtin/cube` model reference is generated internally when loading;
it requires no external asset file or texture. Saved transforms remain independent
for each cube. Existing imported models and environment lighting are preserved.

## Duplication

Select a model instance, built-in cube, point light, or spot light and press
**Ctrl+D** in Scene or Hierarchy, or click **Duplicate selection** in Hierarchy.
The copy starts at the same position and becomes selected; use the Move tool to
separate it. Model copies share their asset resources but have independent
transforms, IDs, and visibility. Light copies preserve all authored settings.
Hidden and disabled states are preserved. Creation is undoable and redo restores
the same copy and ID. Copies are included in normal scene saving.
Directional-light duplication is disabled because the renderer supports one.
