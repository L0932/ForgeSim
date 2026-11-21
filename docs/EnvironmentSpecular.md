# Split-sum environment specular lighting

The environment BRDF lookup in `BrdfIntegration.frag` integrates two coefficients:

- A: the geometry/visibility contribution weighted by `(1 - Fc)`.
- B: the geometry/visibility contribution weighted by `Fc`.

Here `Fc = (1 - VdotH)^5`. Schlick Fresnel is `F0 * (1 - Fc) + Fc`, so the
runtime specular response is:

```
prefilteredEnvironment * (F0 * A + B)
```

`F0` is the material's reflectance at normal incidence: 0.04 for the current
core-glTF dielectric model, blended toward base color by metallic. It must not be
replaced with `FresnelSchlickRoughness(NdotV, F0, roughness)` for this lookup.
That would apply an extra grazing-angle boost after the lookup already
integrated the Fresnel response.

The roughness-aware Fresnel approximation remains in the existing diffuse
energy split. This correction changes specular IBL reconstruction only. It does
not alter the prefilter, direct lighting, textures, normal mapping, environment
intensity, exposure, ACES approximation, or Standard/sRGB transform.

## Investigation for issue #133

After correcting normal-map handedness and the cannon's missing export
modifiers, a pale, relatively uniform sheen remained on wood at some viewing
angles. The issue exists in linear HDR lighting, before either display transform.

An isolated white-environment GPU test with F0=0.04, roughness=0.5, and NdotV=0.1
produced approximately 0.27875 from the old expression, versus 0.12167 from the
lookup's intended F0 reconstruction. This is a 2.29x difference in the isolated
specular contribution, not in total scene brightness. At normal incidence the
expressions coincide; they also coincide at roughness 1 in the current model.

A cannon reproduction using evaluated source geometry, the exported material
textures, and Clarens Midday 4K showed less pale grazing-angle sheen with the
correction, while retaining the existing ACES curve. Both runs used identical
geometry, textures, lighting, and camera settings. The numerical LUT test, rather
than screenshot preference, establishes the reconstruction error.

This does not establish exact parity with Blender. Differences in diffuse BRDF,
IBL convolution, visibility/occlusion, normal filtering, and display transforms
can still affect the result. Legitimate bright reflections at grazing angles
remain part of the material response.

## Optional headless GPU regression

This diagnostic is separate from CTest and does not add build dependencies.
It requires Python, `moderngl`, `numpy`, and an OpenGL 4.6 context. On Linux it uses
EGL by default; Mesa software rendering is sufficient. Other platforms use
ModernGL's default standalone context unless `--backend` is supplied.

```
python -m pip install moderngl numpy
python tools/validation/check_ibl_specular.py
```

The script builds the actual `BrdfIntegration.frag` LUT and renders the actual
`Object.frag` into a floating-point target under a constant white specular
environment with diffuse/direct lighting disabled. It checks 100 combinations
of viewing angle, roughness, and F0 against the integrated lookup coefficients,
including F0=0 and F0=1 endpoints. It requires no scene assets or screenshots.
The corrected shader passes all 100 cases; the old shader fails 31. Use
`--fragment-shader path/to/Object.frag` to test another candidate.

The full Windows application build and interactive angle comparison remain
necessary on the target machine.

## Reference

Brian Karis, Epic Games, [Real Shading in Unreal Engine 4](https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf),
Environment BRDF and split-sum approximation.
