# Running the Installed ForgeSim Sandbox

## Launch

Extract or copy the complete application folder to a suitable location,
then launch ForgeSimSandbox.exe.

Keep the assets directory beside the executable. Moving only the
executable will leave required resources behind.

The application can be launched from another working directory.

## Graphics Requirements

The current renderer requires an OpenGL 4.6-capable graphics driver.

Install an appropriate graphics driver from your hardware vendor if
the application cannot create its graphics context.

## Bundled Content

The installation includes the PBR material gallery, a basic cube,
an HDR environment, and the shaders required by the renderer.

Additional models and purchased showcase assets are not included.

Asset sources and attribution are recorded in:

- assets/models/README.md
- assets/environments/README.md

Dependency notices are under notices/.

## User Data

On Windows, ForgeSim stores application-managed data under:

%LOCALAPPDATA%/ForgeSim/

- Settings/imgui.ini stores the editor layout.
- Scenes/ is the default scene-file directory.

The native file dialog may restore its last-used folder instead of the
default directory.

Explicitly selected scene destinations remain supported. Scene files
reference assets; saving a scene does not copy those assets.

## Scene Asset References

By default, the project root is the directory containing the executable.
Model references in scene documents are resolved against that root,
independently of the scene file's location.

Use --project-root to open documents referring to another project root.

Moving the application does not rewrite external scene documents or
copy external project assets.

## Development Shader Override

By default, shaders load from assets/shaders beside the executable.

FORGESIM_SHADER_DIRECTORY can override that directory for development.
Remove the override when checking an installed build.

## Troubleshooting

Startup diagnostics report the executable, asset, shader, and layout
settings locations.

When reporting a problem, include the application version, operating
system, graphics hardware, and relevant diagnostic output.