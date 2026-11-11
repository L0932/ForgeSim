# Editor font

ForgeSim uses the unmodified Karla Regular font by Jonathan Pinhorn,
distributed with Dear ImGui in `misc/fonts/Karla-Regular.ttf`.

Copyright (c) 2011, Jonathan Pinhorn (pinhorn.typedesign@gmail.com),
with Reserved Font Names "Karla" and "Karla Tamil".

The SIL Open Font License 1.1 is included in `Karla-OFL.txt`.
Keep that license with redistributions of the font.

The font is loaded at editor initialization and shared by every theme.
If it is missing, the editor falls back to Dear ImGui's embedded font.
The existing Sandbox asset-copy and packaging rules include this directory.
