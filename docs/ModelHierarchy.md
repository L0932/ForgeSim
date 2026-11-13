# Imported model hierarchy

Expand the arrow beside a model instance in Hierarchy to browse its active glTF
scene roots and descendants. Authored names and relationships are preserved;
unnamed nodes use their asset indices. Mesh nodes show a mesh index, while
transform/grouping nodes are labeled as nodes. Hover entries for mesh, primitive,
and material-reference details.

Click a node label to select that node in that model instance. Expansion arrows
only control expansion. Alt-click in Scene selects the nearest mesh-node triangle;
ordinary clicks and Ctrl/Shift object multi-selection retain their prior behavior.
Double-click a node label to toggle between its framed view and the previous
view. Expansion arrows remain responsible for opening and closing nodes.
Picking is double-sided and geometric: texture-alpha cutouts are not evaluated.

Selection highlights the selected node's geometry and descendants without
highlighting another instance of the same asset. F frames the selected subtree's
world bounds while preserving viewing direction and fly speed. Empty groups
without geometry have no focus bounds. Nested parent transforms, authored
matrices, and instance transforms contribute to picking and bounds. Bounds are
conservative axis-aligned boxes, so rotated geometry may have extra framing space.

The node Inspector shows the owning model instance, node index, parent, child
count, read-only local translation/quaternion/scale or authored matrix, world
bounds, mesh/primitive references, and associated material values/texture previews.
Material numbers retain their original asset indices. A grouping node includes
materials used by descendant geometry.

The **Instance Node Adjustment** section edits position offsets, rotation offsets
(in degrees), and scale multipliers. Press Enter to apply each field. These are
local adjustments after the authored transform:

`instance × parent nodes × authored local transform × adjustment`

An adjusted group moves its descendants. A child adjustment composes with its
adjusted parent; siblings outside the group and other model instances remain
unchanged. **Reset node adjustment** removes only that node's adjustment. Numeric
edits and Reset support Ctrl+Z/Ctrl+Y. Scale may be mirrored but cannot be zero;
nonfinite values and overflowing hierarchy matrices are rejected.

Scene document version 5 saves each instance's `nodeOverrides` as an array of
`nodeIndex` and `transform` records. Older scene versions load with no adjustments.
Duplicating a model copies its adjustments; editing the copy stays independent.
Overrides use active-scene node indices from the referenced asset, not node names.
Keep the source asset's node layout stable: re-exporting or replacing it can change
those indices. Missing or unreachable referenced indices fail scene preparation
rather than silently applying the edit elsewhere. Indices cannot detect a reordered
asset that still contains the same indices; review adjustments after re-export.

Selected nodes also support Scene gizmos: W translates, E rotates, and R scales.
X toggles world/local translation and rotation axes; scaling uses local axes.
Handles sit at the node's authored origin plus its adjustment, rather than the
subtree bounds center. Dragging a group affects its descendants. A complete drag
is one undo step and saves through the same per-instance adjustment records.
Navigation and hidden model instances suppress node gizmos. An active drag stays
on its original target until release, even if selection changes.

Authored matrices and parent adjustments are preserved. A singular parent disables
the gizmo. A drag that would require shear in the local adjustment is rejected
with feedback in Scene; use local axes or numeric Inspector fields instead.
Node pivots, deletion and duplication remain unavailable. Select the model row
for whole-instance pivot editing, deletion or duplication.

Selection and tree expansion are temporary UI state. Scene replacement clears
selection; deleted owners, changed model references, and unavailable node indices
are cleared safely. Hierarchy search filters model-instance names and light labels,
not internal node names. Metadata is cached per immutable loaded asset and rebuilt
when resources are replaced; instance transforms are evaluated at query time.

Ctrl+F can orbit a selected node subtree using the same bounds center as F focus.
RMB drag rotates around it and the wheel zooms. Ctrl+F restores the pre-orbit
view; Esc keeps the current view. Selecting a different node or model exits orbit. This is camera navigation,
not a node-transform edit.
