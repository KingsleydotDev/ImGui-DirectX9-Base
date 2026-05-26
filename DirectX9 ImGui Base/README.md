# DX9 ImGui Base

C++ DLL template for DirectX 9 overlays using **ImGui** and **MinHook**.

## Controls

| Key | Action |
|-----|--------|
| `INSERT` | Toggle menu |
| `END` | Unload DLL |

Menu tabs: **Player**, **Visuals**, **Misc** (theme, accent, save/load config).

## Build

1. Install the [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812) and set `DXSDK_DIR`.
2. Open `DirectX9 ImGui Base.sln` in Visual Studio (Release \| Win32).
3. Build — output goes to `Build/`.

**Requirements:** DLL, Windows SDK 10, toolset v143, C++20, Multi-Byte, x86.

## Layout

```
src/
  dllmain.cpp
  gui/             — menu, DX9 hook
  gui/pages/       — player, visuals, misc
  game/            — hooks, offsets, structs
  project/         — your code: config (INI), helpers, features
ext/               — imgui, minhook, mINI
```

Put config and your own logic in `src/project/`. Saves to `Corrupted.ini` next to the game exe.

Inject the compiled .dll into a DirectX9 process.

![Screenshot](https://i.ibb.co/1YjmL9gj/iw3mp-Iw-KVZJb-NN0.jpg)
