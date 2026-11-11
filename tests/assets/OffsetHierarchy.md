# OffsetHierarchy.gltf

Original ForgeSim regression fixture; no third-party assets. The buffer is
embedded so the fixture can be copied as a single file.

One triangle has mesh-local vertices `(4, 1, 0)`, `(6, 1, 0)`, `(4, 3, 0)`.
Its bounds center is `(5, 2, 0)`, distinct from the authored origin `(0, 0, 0)`.
Two instances share the same mesh:

- A parent translates by `(10, 20, 30)`, rotates 90 degrees around Z, and
  scales uniformly by 2. Its child translates by `(3, 0, 0)` and scales by
  `(1, 0.5, 1)`. Final vertices are `(9, 34, 30)`, `(9, 38, 30)`, `(7, 34, 30)`.
- A second root uses a column-major matrix translating by `(-2, 3, 0)`.
  Final vertices are `(2, 4, 0)`, `(4, 4, 0)`, `(2, 6, 0)`.

Combined scene bounds are `(2, 4, 0)` to `(9, 38, 30)`. Neither loader should
recenter the mesh or bake either instance's transform into shared vertices.
The headless SceneNodeTransform test checks imported geometry, topology,
hierarchy, and composed positions. Rendering/picking and a centered-pivot
option still require separate validation.
