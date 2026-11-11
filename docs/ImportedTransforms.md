# Imported transforms and pivots

ForgeSim preserves authored mesh coordinates. Camera framing does not resize
or recenter imported geometry.

`LoadGltfMesh()` returns mesh-local geometry from the supported single mesh
and primitive. It does not bake node transforms into those vertices.
`LoadGltfScene()` retains mesh-local geometry and stores node transforms and
hierarchy separately. Sandbox renders the scene path.

A glTF node uses its explicit column-major matrix when present. Otherwise its
local matrix is `T * R * S`, with glTF quaternion components in X, Y, Z, W order.
The node world matrix is the parent world matrix times the local matrix.
Sandbox then applies the scene object's transform outside the imported
hierarchy: `object * nodeWorld * vertex`.

The authored asset origin is the default object pivot. A computed bounds center
is a framing aid, not an implicit replacement pivot. Offset geometry therefore
orbits the authored origin when its enclosing object rotates.

Both rendering and imported-model bounds use `CreateSceneNodeLocalMatrix()`.
The headless `ForgeSim.Sandbox.SceneNodeTransform` test protects identity, TRS
order, explicit matrices, hierarchy composition, offset geometry, and signed
scale. It does not validate the complete loader, picking, or GPU submission path.

`tests/assets/OffsetHierarchy.gltf` supplies an original offset triangle with
nested TRS and a second matrix-transformed instance of the same mesh. The
SceneNodeTransform test loads it through both loader APIs and checks that
mesh-local vertices and authored instance placement remain intact.

## Explicit presentation pivot

Inspector provides Use bounds center and Use authored origin for scene objects.
Both preserve current placement and record undo/redo. The center is computed
from imported-model bounds including node transforms, rather than mesh-local
bounds alone. Shared mesh vertices and imported nodes are never rewritten.

`Transform::position` locates the pivot in world space. Its gizmo matrix is
`T(position) * R * S`. Its geometry matrix is `gizmo * T(-pivot)`. Rendering,
picking, and bounds use the geometry matrix; the gizmo manipulates the gizmo
matrix. Moving between pivots adjusts position so the geometry matrix remains
the same. Future rotation and scaling occur about the chosen pivot.

Scene document version 3 persists the asset-space pivot per object. Versions 1
and 2 remain readable and default to zero pivot. Older ForgeSim versions reject
version 3 rather than silently ignoring the new transform semantics. The pivot
is a fixed authored point after selection; it does not continually follow bounds.
