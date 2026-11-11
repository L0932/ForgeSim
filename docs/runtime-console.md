# Runtime Console

The docked Console accepts commands while ForgeSim is running. Click the input
at the bottom of the panel, type a command, and press Enter. Results appear in
the log history; existing severity and text filters still apply.

| Command | Purpose |
| --- | --- |
| `help` | List registered commands and usage. |
| `help command` | Show usage for one command. |
| `clear` | Clear the Console display. |
| `scene.info` | Show the current scene name, path, and unsaved-change state. |
| `version` | Show the ForgeSim build version. |
| `project.root [directory]` | Show or set the root used by subsequent scene opens. |
| `textures.fallback [on|off]` | Show or change texture fallback policy for subsequent model loads. |
| `images.cache [on|off]` | Show or change decoded-image disk caching for subsequent model loads. |
| `loading.diagnostics [on|off]` | Show or change scene-load timing and import-stage diagnostics. |

Commands are case-sensitive. Arguments are separated by whitespace. Double quotes
group paths containing spaces; backslashes remain literal. An unclosed quote is
an error. Invalid arguments and handler failures produce error messages.

For example, enter `project.root "C:/ForgeSimAssets"`, then use File → Open to
open the Cathedral scene. If its model reference is `Cathedral_01/cathedral.glb`,
ForgeSim resolves it beneath that root. Run `project.root` to inspect the setting.
The directory must exist. Relative directories are resolved from the process
working directory. This setting lasts for the session.

The root applies when preparing the next scene. The currently loaded scene keeps
its original root for saving and resolving its existing model. A failed open
leaves that scene untouched. To set both the root and texture policy, enter:

```text
project.root "C:/ForgeSimAssets"; textures.fallback on
```

Then open or reopen the scene. Both settings last for this session. Texture
fallback starts with the policy selected at startup (`--allow-texture-fallbacks`
enables it). Changing it does not reload the current model. Fallback uses the
existing glTF policy: skip unusable texture/UV bindings and retain material
factors. It does not repair missing model files or guarantee recovery from every
texture or import error.

Enable decoded-image caching before opening a large GLB:

```text
project.root "C:/ForgeSimAssets"; textures.fallback on; images.cache on
```

Run `images.cache` to inspect the setting, or `images.cache off` to disable it.
The initial value follows the startup `--cache-decoded-images` flag. The runtime
setting lasts for the session and applies to subsequent model imports. A load
without cached images populates the cache; later eligible GLB loads can reuse it.
The existing cache validation and configured disk budget still apply. Turning
caching off bypasses cache use without deleting existing cache files. The command
does not reload the current model.

To include load timings:

```text
project.root "C:/ForgeSimAssets"; textures.fallback on; images.cache on; loading.diagnostics on
```

Then open or reopen the scene. Diagnostics follow the startup
`--loading-diagnostics` flag until changed for this session. Run
`loading.diagnostics` to query the setting or `loading.diagnostics off` to disable
it. Existing import-stage, cache and GPU resource preparation logs are enabled.
Interactive scene opens additionally report document loading, resource readiness,
and total open completion time in milliseconds. Failed opens report elapsed time.
Total open completion includes loading progress presentation and editor state
replacement; it ends before rendering the first editor frame of the new scene.
Timing messages use the Info severity, so enable that filter to see them.

Semicolons separate commands outside double quotes. They remain literal inside
quoted paths. Commands run left to right and stop at the first error; earlier
successful settings remain applied. An unclosed quote prevents the entire line
from executing. Empty command segments are ignored.

Up and Down recall commands. Returning past the newest entry restores the draft
that was present before browsing history. History keeps the last 100 commands
for the session, excluding consecutive duplicates. Recalled commands can be
edited before submitting. The input accepts up to 1023 UTF-8 bytes.

Keyboard editor shortcuts and camera navigation are suppressed while the Console
input or filter is active. After submission the command input retains focus.
Click another panel to resume editing.

`clear` clears the display without changing command history or terminal output.

## Adding commands

`ConsoleCommands.hpp` contains a UI-independent registry, dispatcher, results,
and command history. Register a command with its description, usage, argument
count limits, and handler. `help` discovers registered commands automatically.
Handlers receive a span of argument strings and the current command context.
They return a message, error status, and optional display-clear request.
Handlers run synchronously on the editor thread. Exceptions become Console
errors so subsequent commands remain available.
