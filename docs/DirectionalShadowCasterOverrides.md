# Directional shadow caster overrides

To let directional light pass through a visible mesh, add the
`forgesim_cast_directional_shadows` custom property to its Blender **object**
and set it to integer `0`. Enable **Custom Properties** in the glTF exporter.
The property is read from the exported glTF node's `extras`:

```json
{
  "extras": {
    "forgesim_cast_directional_shadows": 0
  }
}
```

Boolean `false` also disables casting. Omit the property or use `1` or
`true` to cast normally. Other values for this property are rejected
with the asset path and node index.

The setting affects every primitive attached directly to that node. It does
not propagate to child nodes or other instances of the same mesh or material.
If a pane and its frame are part of the same Blender object, separate the glass
into its own object before assigning the property.

This setting affects only casting into the directional shadow map. The mesh
remains visible and can still receive shadows. Light passes through without
tint or attenuation.
