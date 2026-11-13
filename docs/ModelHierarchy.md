# Imported model hierarchy

Expand the arrow beside a model instance in Hierarchy to browse its active glTF
scene roots and descendants. Authored names and relationships are preserved;
unnamed nodes use their asset indices. Mesh nodes show a mesh index, while
transform/grouping nodes are labeled as nodes. Hover entries for mesh, primitive,
and material-reference details.

Click a node label to select that node in that model instance. Expansion arrows
only control expansion. Alt-click in Scene selects the nearest mesh-node triangle;
ordinary clicks and Ctrl/Shift object multi-selection retain their prior behavior.
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

Node editing is not enabled in this first #123 patch. Gizmos, pivot edits,
deletion, and duplication do not operate on selected nodes. Select the model's
main row to author the whole instance. Per-instance transform overrides, reset,
undo/redo and persistence are subsequent work.

Selection and tree expansion are temporary UI state. Scene replacement clears
selection; deleted owners, changed model references, and unavailable node indices
are cleared safely. Hierarchy search filters model-instance names and light labels,
not internal node names. Metadata is cached per immutable loaded asset and rebuilt
when resources are replaced; instance transforms are evaluated at query time.
