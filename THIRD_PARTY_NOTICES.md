# Third-Party Notices

ForgeSim uses the following third-party libraries:

| Dependency | Upstream |
|---|---|
| GLFW | https://github.com/glfw/glfw |
| GLM | https://github.com/g-truc/glm |
| cgltf | https://github.com/jkuhlmann/cgltf |
| Dear ImGui | https://github.com/ocornut/imgui |
| ImGuizmo | https://github.com/CedricGuillemet/ImGuizmo |
| stb_image | https://github.com/nothings/stb |
| libdeflate (optional PNG acceleration) | https://github.com/ebiggers/libdeflate |
| nlohmann/json | https://github.com/nlohmann/json |
| GLAD and Khronos specification/header data | https://github.com/Dav1dde/glad |

The installed notices directory contains upstream license documents
copied from the dependency sources used by the build.

Bundled model and environment attribution is recorded in the README
files under assets/models and assets/environments. The Chess Board sample includes
Poly Haven's Chess Set (Riley Queen), Chinese Tea Table (Kirill Sannikov), and
Thatch Chapel (Dimitrios Savva and Jarod Guest), distributed under CC0-1.0.
Source links and modification details are recorded in those asset README files.

Microsoft compiler runtime files may accompany Windows builds.

These third-party notices do not grant a license to ForgeSim-owned
code or assets. ForgeSim distribution terms are managed separately.

## Optional cache benchmark

The opt-in `ForgeSimCacheBenchmark` tool statically links LZ4 v1.10.0
(commit `ebb370ca83af193212df4dcbadcc5d87bc0de2f0`), distributed under the
BSD 2-Clause license. Upstream: https://github.com/lz4/lz4.
The build copies `lib/LICENSE` beside the benchmark as `LZ4-LICENSE.txt`.
LZ4 is not linked into the Sandbox or included in runtime packages.
