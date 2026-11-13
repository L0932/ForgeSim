# Model hierarchy

Expand the arrow beside a model instance in Hierarchy to browse the glTF asset's
active scene roots and child nodes. Each node shows its name (or index when
unnamed) and mesh index when present. Expand child arrows to navigate deeper.
Hover a node for its index, local translation or matrix status, mesh name,
primitive count and material numbers corresponding to the Inspector.

This first view is read-only: use the model's main row for selection, transforms,
F focus, visibility, deletion and duplication. Asset nodes do not yet have
independent selection or persistent overrides. The existing Hierarchy search
filters model-instance names and light labels, not internal node names.

Duplicated model instances browse the same asset while retaining independent
instance transforms. Expansion is UI state and does not mark scenes modified.
Resource pointers are borrowed for the current frame only. Invalid references,
cycles and excessive depth are guarded in the tree display.
