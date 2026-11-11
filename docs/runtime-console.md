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

Commands are case-sensitive. Arguments are separated by whitespace. The initial
commands accept no arguments except `help`, which accepts an optional command
name. Invalid arguments and handler failures produce error messages.

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
