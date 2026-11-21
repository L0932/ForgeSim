"""Optional OpenGL regression for split-sum specular IBL.

Requires moderngl and numpy; a software EGL context is sufficient on Linux.
Runs the actual BRDF-integration and object fragment shaders without a window.
"""

import argparse
import math
from pathlib import Path
import sys

import moderngl
import numpy as np

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument(
    "--backend",
    default="egl" if sys.platform.startswith("linux") else None,
    help="ModernGL standalone backend (default: EGL on Linux)",
)
parser.add_argument(
    "--shader-directory",
    type=Path,
    default=Path(__file__).resolve().parents[2] / "apps/Sandbox/assets/shaders",
)
parser.add_argument(
    "--fragment-shader",
    type=Path,
    help="Override Object.frag, for checking a candidate or pre-fix shader",
)
args = parser.parse_args()
root = args.shader_directory
context_options = {"require": 460}
if args.backend:
    context_options["backend"] = args.backend
ctx = moderngl.create_standalone_context(**context_options)
# Generate the actual shader's split-sum LUT, independent of Object.frag.
lut = ctx.texture((256, 256), 2, dtype="f4")
lut.filter = (moderngl.LINEAR, moderngl.LINEAR)
lut.repeat_x = lut.repeat_y = False
fb = ctx.framebuffer([lut])
fb.use()
ctx.viewport = (0, 0, 256, 256)
p = ctx.program(
    vertex_shader=(root / "ToneMapping.vert").read_text(),
    fragment_shader=(root / "BrdfIntegration.frag").read_text(),
)
ctx.vertex_array(p, []).render(vertices=3)
table = np.frombuffer(lut.read(), "f4").reshape(256, 256, 2)
white = ctx.texture((1, 1), 4, np.ones(4, "f4").tobytes(), dtype="f4")
normal = ctx.texture((1, 1), 4, np.array([0.5, 0.5, 1, 1], "f4").tobytes(), dtype="f4")


def cube(value):
    t = ctx.texture_cube((1, 1), 3, dtype="f4")
    for face in range(6):
        t.write(face, np.full(3, value, "f4").tobytes())
    return t


blackcube = cube(0)
whitecube = cube(1)
vert = """#version 460 core
out vec3 vColor;
out vec2 vTextureCoordinates;
out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec4 vWorldTangent;
void main()
{
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
    vColor = vec3(1.0);
    vTextureCoordinates = vec2(0.5);
    vWorldPosition = vec3(0.0);
    vWorldNormal = vec3(0.0, 0.0, 1.0);
    vWorldTangent = vec4(1.0, 0.0, 0.0, 1.0);
}
"""
out = ctx.texture((1, 1), 4, dtype="f4")
fb = ctx.framebuffer([out])
fb.use()
ctx.viewport = (0, 0, 1, 1)
texnames = [
    "uBaseColorTexture",
    "uMetallicRoughnessTexture",
    "uNormalTexture",
    "uOcclusionTexture",
    "uEmissiveTexture",
    "uDiffuseIrradianceMap",
    "uPrefilteredSpecularMap",
    "uBrdfLookupTexture",
]
textures = [white, white, normal, white, white, blackcube, whitecube, lut]


# GL_LINEAR sampling at normalized coordinates, with clamp-to-edge.
def sample_lut(v, r):
    x = np.clip(v * 256 - 0.5, 0, 255)
    y = np.clip(r * 256 - 0.5, 0, 255)
    i = int(x)
    j = int(y)
    fx = x - i
    fy = y - j
    return (1 - fy) * ((1 - fx) * table[j, i] + fx * table[j, min(i + 1, 255)]) + fy * (
        (1 - fx) * table[min(j + 1, 255), i]
        + fx * table[min(j + 1, 255), min(i + 1, 255)]
    )


def run(fragment):
    p = ctx.program(vertex_shader=vert, fragment_shader=fragment)
    vao = ctx.vertex_array(p, [])

    def set(n, v):
        if n in p:
            p[n].value = v

    for i, (name, t) in enumerate(zip(texnames, textures)):
        t.use(i)
        set(name, i)
    for slot in ["BaseColor", "MetallicRoughness", "Normal", "Occlusion", "Emissive"]:
        set("u" + slot + "UvRow0", (1, 0, 0))
        set("u" + slot + "UvRow1", (0, 1, 0))
    set("uNormalScale", 0)
    set("uOcclusionStrength", 1)
    set("uEnvironmentIntensity", 1)
    set("uLightDirection", (0, 0, 1))
    set("uMetallicFactor", 1)
    failed = []
    count = 0
    for r in [0.045, 0.25, 0.5, 0.75, 1]:
        set("uRoughnessFactor", r)
        for v in [1, 0.8, 0.5, 0.2, 0.1]:
            set("uCameraPosition", (math.sqrt(1 - v * v), 0, v))
            a, b = sample_lut(v, r)
            # A white environment isolates the integrated specular BRDF.
            # F0=0 and F0=1 also exercise the coefficient endpoints.
            for f0 in [0, 0.04, 0.5, 1]:
                set("uBaseColorFactor", (f0, f0, f0, 1))
                vao.render(vertices=3)
                actual = np.frombuffer(out.read(), "f4")[:3]
                # The lookup integrates (1-Fc) and Fc, so its material
                # parameter is normal-incidence reflectance, not F(NdotV).
                expected = f0 * a + b
                if not np.allclose(actual, expected, atol=3e-4, rtol=0.002):
                    failed.append((r, v, f0, float(actual[0]), float(expected)))
                count += 1
    return count, failed


fragment = args.fragment_shader or root / "Object.frag"
count, failures = run(fragment.read_text())
print(f"Specular IBL: {count} white-environment cases; {len(failures)} failures")
for roughness, ndotv, f0, actual, expected in failures[:5]:
    print(
        f"  roughness={roughness}, NdotV={ndotv}, F0={f0}: "
        f"actual={actual:.6f}, expected={expected:.6f}"
    )
ctx.release()
raise SystemExit(1 if failures else 0)
