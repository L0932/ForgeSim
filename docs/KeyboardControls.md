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
| Selection | Alt+Left click | Select the nearest imported mesh node | Scene; triangle geometry, not texture alpha |
| Selection | Ctrl+Left click | Toggle object membership; last selected object is active | Scene or Hierarchy |
| Selection | Shift+Left click | Select a visible range from the anchor / add an object | Hierarchy range / Scene add |
| Selection | Ctrl+Shift+Left click | Add a visible range to the selection | Hierarchy |
| Camera | Ctrl+F | Toggle orbit mode around selection | Scene or Hierarchy focused; navigation stopped |
| Camera | RMB drag / Wheel | Orbit / zoom around selection | Orbit mode active; wheel over Scene |
| Camera | Esc | Exit orbit at current view | Scene or Hierarchy focused; no text entry or modal |
| Selection | Shift+H | Show / hide selection highlight | Scene or Hierarchy focused; selection stays active |
| Selection | F | Toggle selection focus / previous view | Scene or Hierarchy focused; navigation stopped |
| Selection | Double-click label | Select and toggle focus on object, imported node, or light | Hierarchy |
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

Click the Scene viewport or a Hierarchy row, then press **F** to frame the selected
objects, imported node subtree, or light. Press F again on the same selection to
restore the view from before focusing; another press frames the selection again.
Double-clicking an object, imported-node, or light label in Hierarchy uses the same
toggle. Expansion arrows and visibility buttons do not focus. Node-label double
clicks no longer collapse/expand the node; use the arrow for that.

The Camera panel's **Toggle focus (F)** button shares this behavior. Focusing and
returning preserve fly speed, saved viewpoints, scene data, and the viewing
parameters captured for the return view. Selection changes, New/Open/import, and
explicit Frame scene reset the return state. If the camera is moved or edited, or
the selected geometry moves, the next focus starts a new pair from the current view.

Model focus fits transformed bounds, including pivot and scale, while preserving
the current viewing direction and field of view. Group focus covers all selected
objects, and node focus includes descendant geometry. Far clipping covers the
complete scene. Directional lights frame their editor marker; point and spot
lights frame a neighborhood sized from their range and scene scale. Empty node
subtrees without geometry have no focus bounds.

F is suppressed while typing, editing a numeric control, using a gizmo, navigating
with RMB, holding modifier keys, or while a popup/scene-change dialog is open.
It does not fire from the Console or an unrelated panel. With no selection it does
nothing. Hierarchy double-click focus is also suppressed while interactions are
blocked or modifier keys are held.

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

## Multiple object selection

Plain click replaces the object selection. Ctrl-click toggles membership, and the
last selected member is active. Shift-click in Hierarchy selects the displayed
range from the last non-range click; Ctrl+Shift-click adds that range. Filtering
excludes rows from new ranges but does not clear existing selections. Shift-click
in Scene adds the hit object, and modified clicks on empty space preserve the
selection. Plain clicks on empty Scene space clear it. Lights remain single-select.

All selected objects highlight in Scene. F frames their combined bounds, including
hidden members. Move applies the active object's translation delta to every
member. Rotate applies the same world rotation delta at each object's own pivot;
Scale applies the active object's scale ratios in each member's local axes at its
own pivot. Local/World still controls the gizmo axes. This preserves relative
placement and avoids introducing shear. The Inspector shows active-object values
and materials, with edits to position, rotation, scale and pivot applied to all
selected objects. Each object's bounds-center pivot uses its own model bounds.

Delete and Duplicate apply to the whole selection as a single undo/redo action.
Duplication selects all copies, preserving member transforms and visibility.
Show/Hide selection and the eye button of any selected row apply to the group;
undo restores each member's previous visibility. An unselected row's eye button
continues to affect that row only. Saving stores the resulting object states;
selection and the range anchor are temporary editor state.

## Imported node selection

Click a node label in the expanded model hierarchy, or Alt-click its geometry in
Scene, to select one imported node in one model instance. Expansion arrows only
expand/collapse the tree. Ordinary clicks and Ctrl/Shift gestures retain their
whole-object selection behavior. Alt-click uses double-sided triangle picking;
it does not evaluate texture alpha, so the transparent part of a leaf card can
still be picked. Light markers retain their existing picking priority.

A node selection highlights its own geometry and descendants. F frames the
subtree's world bounds, preserving camera orientation and movement speed. Empty
nodes without descendant geometry have no framing bounds. The Inspector shows
read-only local transforms (including authored matrices), parent information,
world bounds, mesh/primitive references and the subtree's associated materials.

Numeric node adjustments, Reset, undo/redo and scene persistence are supported
through the Inspector. Node gizmos use W/E/R and X for world/local axes; a drag
is one undo step. Node pivots, deletion and duplication remain unavailable.
Select the model's main row to manipulate the complete instance.
Selection itself is temporary editor state; node adjustments are authored scene data.

## Selection orbit mode

Press Ctrl+F with Scene or Hierarchy focused to frame the current selection and
enter an upright orbit mode. Objects, object groups, imported node subtrees and
lights use the same bounds as ordinary focus. An empty selection or node subtree
without geometry cannot start orbit mode.

- Hold RMB and drag in Scene to orbit around the selection's bounds center.
- Scroll over Scene to move closer/farther without changing FOV or saved fly speed.
- While orbiting, WASD fly movement is suspended. Pitch and zoom distance are
  bounded to avoid camera flips or crossing through the pivot.
- Ctrl+F again or the Camera panel's Exit orbit button restores the view from
  before orbit started. Esc exits while keeping the current orbit view.
  An active-mode hint appears in Scene.
- Changing selection, deleting the target, New/Open/import, explicit Frame scene,
  or restoring/editing a camera pose exits orbit. Moving the same selected target
  updates the pivot while retaining the orbit offset.
- Ordinary F and Hierarchy double-click also restore the pre-orbit view while
  orbit is active. Outside orbit, the existing selection-focus/previous-view
  toggle remains available. Entering or explicitly toggling orbit resets that pair.

Press **Shift+H** with Scene or Hierarchy focused, or uncheck **Show selection highlight** in the Camera panel to hide the selection
tint while retaining the selection, Inspector target, and editing controls.
This shares the existing highlight setting under **ForgeSim Debug > Selection**.

Mouse input pauses during text entry, active controls, gizmo edits, scene-change
confirmation, and popups. RMB capture still begins only inside Scene and releases
when navigation stops. Orbit is temporary navigation state; it does not mark the
scene modified, change saved viewpoints, or save an orbit mode in the scene file.

## Imported node adjustments

Select an imported node in Hierarchy or Alt-click it in Scene. Under Inspector >
Instance Node Adjustment, edit position offsets, rotation offsets in degrees, or
scale multipliers and press Enter. Reset node adjustment restores that node's
original local transform while preserving descendant adjustments. Ctrl+Z/Ctrl+Y
undo and redo these edits. Save the scene to retain the per-instance adjustments.

Node gizmos use the same saved adjustments. Select a node and use W (move), E
(rotate), or R (scale); X toggles world/local move and rotation axes. Scale uses
local axes. A singular parent or a drag requiring local shear is rejected with
feedback in Scene; numeric Inspector editing remains available.
