# WapMap: HGE → SFML 3 Migration Plan

## 1. What WapMap actually uses from HGE

Measured across `WapMap/` and `shared/` (~92k LOC, 140 files touch HGE).

| Area | Used API (call count) | Notes |
|---|---|---|
| Window / loop | `System_Initiate/Start/Shutdown`, `System_Get/SetState` (≈245, mostly `HGE_SCREENWIDTH/HEIGHT`, `HGE_HWND`), callbacks: frame, render, exit, focus, resize, fullscreen, gfx-restore, file drag-in/out/drop | Borderless window with custom resize borders (`HGE_WINDOWCAPTION=false`, `HGE_BORDER_WIDTH`, `WM_NCHITTEST`) and OLE drag & drop: both are custom changes in the HGE fork (`hge/src/core/system.cpp`). `System_DoManualMainLoop` is used for modal dialogs (`states/dialog.cpp`). |
| 2D drawing | `Gfx_RenderLine` (438), `Gfx_RenderQuad` (55), `Gfx_RenderTriple` (1), `Gfx_SetClipping/GetClipping` (69), `Gfx_SetTransform` (2, guichan), `Gfx_Clear`, `Gfx_Begin/EndScene` | Plain immediate-mode 2D with per-vertex ARGB colour. No Z-buffer enabled, so `SetZ` calls are no-ops. |
| Blend modes | `BLEND_DEFAULT`, `ALPHAADD`, `COLORMUL`, `NOZWRITE`, `COLORADD` (only 1 use, `wComboButton.cpp`) | |
| Textures | `Texture_Load` (from memory and file), `Texture_Create`, `Texture_Lock/Unlock` (CPU pixel access), `Texture_GetWidth/Height`, `Texture_Free` | Pixel format is ARGB `DWORD`. HGE may pad textures to power-of-two (`GetWidth(tex, true)` = original size). |
| Render targets | `Target_Create/Free/GetTexture`, `Target_GetSurface` + `Surface_Lock` (map screenshot) | Viewport, layer buffers, state manager buffer, mapshot. |
| Helpers | `hgeSprite` (252 refs), `hgeFont` (bitmap `.fnt`), `hgeQuad`, `hgeTriple`, `hgeVector`, `hgeRect` | Helpers sit only on top of the HGE core interface, so their source can be reused unchanged. |
| Input | `Input_GetKeyState/KeyDown/KeyUp` (≈190), `Input_GetMousePos/SetMousePos`, `Input_GetMouseWheel`, `Input_GetChar`, `Input_GetEvent` (guichan) | 69 distinct `HGEK_*` codes. |
| Audio (BASS) | `Effect_Load` (WAV from memory), `Effect_Play/PlayEx`, `Channel_Stop/Pause/Resume/IsPlaying/SetVolume/GetPos/SetPos/GetLength` | Sound browser and game simulator only. |
| Misc | `Timer_GetDelta/GetTime/GetFPS`, `Random_Int/Float` | Trivial. |
| Shaders | `Shader_*` | Dead code (commented out in `shared/cStateMgr.cpp`). Drop it. |

**The UI is not HGE.** It's **guichan** (`shared/guichan`) plus about 30 custom skinned widgets (`shared/gcnWidgets`), connected to HGE through a ~1000-line backend (`shared/guichan/hge/*`). 159 files and about 850 `new SHR::...` widget instantiations depend on it. Win32 is already used directly (HWND, `MessageBox`, `Get(Open|Save)FileName`, `SetCursor`, clipboard), and the app is Windows-only today.

## 2. Options considered

| Option | Fit for WapMap | Verdict |
|---|---|---|
| **SFML 3.0 (graphics + window + audio)** | Maps almost 1:1 onto HGE: textures, `RenderTexture`, vertex arrays with per-vertex colour, blend modes, scissor (new in SFML 3 via `sf::View::setScissor`), `sf::Shader` for the one `COLORADD` case, and **built-in audio** that covers everything BASS does (load WAV from memory, volume, pause, seek, length). Exposes the HWND, so the forked Win32 code (borders, OLE drop) moves over as-is. Already partly used (`mapshot`, `cPieChart`, `editCandy`, `about`). imgui-sfml exists if wanted later. | **Chosen** |
| SDL3 + SDL_Renderer | Very strong technically: native hit-test (custom borders), file-drop events, clip rect, automatic batching, file dialogs, official ImGui backend. But core audio is low-level, so you'd need SDL3_mixer or miniaudio on top. No custom shaders on the 2D renderer (`COLORADD`). C API. | Close runner-up. Pick it if cross-platform (Linux/macOS) becomes a goal. |
| raylib | Simple, has audio and render textures, but it's one global window with global state, has weaker blend/scissor control, and a C API. Fine for games, less so for an editor app with a lot of Win32 integration. | Rejected |
| Replace the UI with Dear ImGui (SFML + ImGui) | ImGui is immediate-mode, and WapMap's UI is retained-mode guichan with custom skinned widgets. Swapping means **rewriting every window and state** (159 files). That's a redesign, not a library swap, and it isn't needed to get rid of HGE. | Not part of this migration. ImGui can be added afterwards **next to** guichan, with its own window-drag-out support (Step 12). |
| Raw D3D11 / OpenGL / bgfx | Lots of work for no benefit at this scale. | Rejected |

### Decision

**SFML 3** as the HGE replacement. **Keep guichan.** Do it as a drop-in **HGE-compatible facade** so the 140 calling files don't change. ImGui (via imgui-sfml) is an optional step after the migration, not the migration itself.

The whole approach depends on one fact: every WapMap call goes through the `HGE` interface pointer (`hge->...`) and the HGE helper classes. If we reimplement that interface (about 70 methods) on SFML, then:
- the 140 game/editor files compile unchanged,
- `hgeSprite`/`hgeFont`/`hgeVector`/`hgeRect` are copied verbatim from `hge/src/helpers` (zlib licence),
- the guichan HGE backend (`shared/guichan/hge/*`) works unchanged because it only calls `hge->...`.

Once that runs, call sites can be moved to native SFML gradually, if anyone wants to.

---

## 3. Step-by-step plan

Each step ends in a state that builds and can be tested. Do them in order.

### Step 0: Baseline (before touching anything)
1. Build the current HGE version. Keep the exe as a reference build.
2. Record **reference screenshots** and a manual **test checklist**:
   - Open a Claw map; pan and zoom the viewport; tile mode (brush, fill, select); object mode (move, multi-select, edit window).
   - Every window: options, tile browser, image set browser, palette browser, new map, go to coords, about.
   - Sound browser: play, pause, seek, volume.
   - Game simulator (sounds, ambients).
   - Mapshot export; image import.
   - Drag a `.wwd` file onto the window (hover highlight, drop).
   - Window resize from all edges and corners, maximise, move via the custom title bar, minimum size 1024×550.
   - Copy/paste, all hotkeys (`procHotkeysHandle.cpp`), mouse wheel, text input in text fields (including non-ASCII).
3. Stay on **x86** for the migration so Lua, curl and zlib don't also need rebuilding. Switching to x64 comes later (Step 11).

### Step 1: Bring in SFML 3
1. Get **SFML 3.0.x** with CMake `FetchContent` or vcpkg (`sfml:x86-windows-static`). Replace the vendored `SFML-2.5.1` and the `SFML_DIR`/`SFML_LIBS` lines in `CMakeLists.txt`. Link `sfml-graphics`, `sfml-window`, `sfml-system`, `sfml-audio` (static, `SFML_STATIC`).
2. Fix the 4 files that already use SFML 2 APIs (`states/mapshot.*`, `shared/cPieChart.cpp`, `objEdit/editCandy.cpp`, `windows/about.cpp`). SFML 3 changes: `sf::Vector2u` arguments, `std::optional` returns, `loadFromFile` → constructors or `[[nodiscard]]` bools, and so on (see the SFML 3 migration guide).
3. ✅ Check: the app still builds and runs **on HGE**, with SFML 3 linked in next to it.

### Step 2: Create the facade project skeleton
1. Create `WapMap/hgecompat/` containing:
   - `include/hge.h`: a **trimmed copy** of `hge/include/hge.h` that keeps only what WapMap uses: the `HGE` class with the ~70 used methods, `hgeVertex/hgeQuad/hgeTriple`, `hgeInputEvent`, `BLEND_*`, `HGEK_*`, `HGEINP_*`, `HGE_*` state enums, and the handle typedefs (`HTEXTURE`, `HTARGET`, `HEFFECT`, `HCHANNEL`). Make it a concrete class (no pure virtuals), so any missing method becomes a link error that's easy to spot.
   - Copies of `hgesprite.h/.cpp`, `hgefont.h/.cpp`, `hgevector`, `hgerect`, `hgecolor` from `hge/include` and `hge/src/helpers`.
   - `src/hge_sfml.cpp` (+ `system.cpp`, `graphics.cpp`, `input.cpp`, `audio.cpp`, `win32.cpp`): the implementation.
2. Handles are opaque pointers: `HTEXTURE` → `TexEntry*` holding `{ sf::Texture tex; sf::Image cpu; bool locked; ... }`, `HTARGET` → `TargetEntry*` holding `{ sf::RenderTexture rt; TexEntry view; }`, `HEFFECT` → `sf::SoundBuffer*`, `HCHANNEL` → index into a sound pool.
3. In CMake, put `hgecompat/include` **before** any HGE include path and compile the helper `.cpp` files into WapMap. Remove `hge-static.lib`, `hgehelpers.lib`, `d3d9`/`d3dx9` and the DirectX SDK path.
4. Stub every method (return 0, empty bodies) so the project **links**.
5. ✅ Check: WapMap compiles and links with zero HGE/DirectX dependencies. (It won't run correctly yet.)

### Step 3: Window, main loop, timer, random
Implement in `system.cpp`:
1. `System_Initiate`: create an `sf::RenderWindow` with `sf::Style::None` (because `HGE_WINDOWCAPTION=false`). Set the size from `HGE_SCREENWIDTH/HEIGHT`, the title, the icon (`LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(100))` then `SendMessage(WM_SETICON)`), and VSync on (HGE ran with `HGE_FPS` unset, i.e. unlimited; VSync is the sane default for an editor).
2. `System_Start` loop. Per iteration:
   `pollEvents → update input state → frameFunc() (exit if it returns true) → renderFunc() → window.display()`.
   Put one iteration in a private `RunOneFrame()` and have `System_DoManualMainLoop()` call it, so modal dialogs keep working.
3. Events → callbacks: `FocusLost/Gained` → focus funcs, `Resized` → update `HGE_SCREENWIDTH/HEIGHT`, **reset the window's `sf::View` to the new size**, call `resizeFunc`. `Closed` → `exitFunc` (only close if it returns true).
4. `System_GetState(HGE_HWND)` → `window.getNativeHandle()`. This keeps all existing Win32 code (MessageBox, file dialogs, clipboard) working.
5. `Timer_GetDelta/GetTime/GetFPS/GetDeltaRealtime` with `sf::Clock`. `Random_Int/Float` with `std::mt19937`.
6. `HGE_DONTSUSPEND`, `HGE_SHOWSPLASH`, `HGE_LOGFILE`, `HGE_USESOUND`: accept and ignore, or write a simple log.
7. ✅ Check: a blank window opens, resizes, and closes properly.

### Step 4: Win32 extras (custom border, drag & drop, cursors)
Port from `hge/src/core/system.cpp`, in `win32.cpp`:
1. **Subclass** the SFML window: `SetWindowLongPtr(hwnd, GWLP_WNDPROC, WapProc)`. Keep the old proc and call it with `CallWindowProc` for anything you don't handle.
2. Copy the `WM_NCHITTEST` (resize borders from `HGE_BORDER_WIDTH`), `WM_NCCALCSIZE`/`WM_NCPAINT` (return 0) and `WM_GETMINMAXINFO` (1024×550) handlers over unchanged. Also add `WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX` to the style so snap and maximise work on a borderless window.
3. Copy the **`FileDropListener` OLE `IDropTarget`** (`OleInitialize` + `RegisterDragDrop`). It fills the list behind `System_GetDraggedFiles()`/`System_GetDroppedFiles()` and calls the drag-in/out/drop callbacks. This is what gives the hover highlight; `WM_DROPFILES` alone can't do it.
4. Cursors: SFML resets the cursor in `WM_SETCURSOR`. Either handle `WM_SETCURSOR` in the subclass proc (return TRUE when the app has set its own cursor) or call `window.setMouseCursorVisible` and keep the existing `SetCursor` calls. Test the GRAB, HAND and ZOOM cursors from `res/`.
5. The fullscreen toggle callback: implement by recreating the window or by toggling a maximised borderless window. **Recreating the window destroys GL resources in some cases. Prefer borderless-maximise.**
6. ✅ Check: move, resize, snap, minimum size, drag-and-drop of a `.wwd` with highlight, cursors.

### Step 5: Input
In `input.cpp`:
1. Make a table from `sf::Keyboard::Key` to `HGEK_*` (69 codes used; include `HGEK_LBUTTON/RBUTTON/MBUTTON`, which HGE treats as keys).
2. Keep `cur[256]` and `prev[256]` arrays. `Input_GetKeyState` = cur, `Input_KeyDown` = cur && !prev, `Input_KeyUp` = !cur && prev. Update from events, not from `sf::Keyboard::isKeyPressed`, so keys don't register while the window is unfocused.
3. Mouse position from `MouseMoved` events (window coordinates), and accumulate the wheel per frame from `MouseWheelScrolled`. `Input_SetMousePos` → `sf::Mouse::setPosition(p, window)`.
4. `Input_GetChar` from `TextEntered`. Check how the old code converted to code page / ANSI (the app is `char`-based). Convert UTF-32 to the same encoding.
5. `Input_GetEvent`: push `hgeInputEvent`s (`INPUT_KEYDOWN/KEYUP/MBUTTONDOWN/MBUTTONUP/MOUSEMOVE/MOUSEWHEEL`, with flags SHIFT/CTRL/ALT/REPEAT, `chr`, `x/y`) into a queue. **guichan's `hgeinput.cpp` depends on this matching exactly.** Compare it against `hge/src/core/input.cpp`.
6. ✅ Check: hotkeys, text fields, mouse wheel zoom, drag-select.

### Step 6: Graphics core and batching (the biggest step)
In `graphics.cpp`:
1. **Batcher**: keep an `sf::VertexArray` (Triangles) plus the current texture, blend mode, render target, clip rect and transform. Every `Gfx_RenderQuad/Triple` adds vertices. **Flush** when any of those state values changes, or on `Gfx_EndScene`. (HGE batches the same way; drawing every sprite separately in SFML would be too slow for tile maps.)
2. **Lines**: `Gfx_RenderLine` goes to a separate `sf::PrimitiveType::Lines` batch (flush ordering matters: keep a single queue of mixed primitives, or flush the triangles before lines and the other way round). Add **+0.5 px** to coordinates so 1-px lines stay sharp (D3D9 and GL rasterise lines differently).
3. **Colour**: HGE's `DWORD` is `0xAARRGGBB`. Convert with `sf::Color(c>>16&255, c>>8&255, c&255, c>>24)`. Texture coordinates: HGE uses normalised 0..1 UVs and SFML uses pixels, so multiply by the texture size when emitting vertices.
4. **Blend modes**: per-vertex colour × texture (`COLORMUL`) is SFML's default. `ALPHABLEND` → `sf::BlendAlpha`, `ALPHAADD` → `sf::BlendAdd`, `NOZWRITE`/Z → ignore. `COLORADD` (1 use) → a 5-line GLSL fragment shader `gl_FragColor = texture2D(tex, uv) + vec4(color.rgb, 0) ` with alpha from the texture × vertex. Alternatively, change that single button hover effect.
5. **Textures**: `Texture_Load(mem/file)` → `sf::Texture::loadFromMemory/File`, `setSmooth(false)` (`HGE_TEXTUREFILTER=false`). `Texture_Create(w,h)` → empty texture. **Don't pad to power of two.** `GetWidth(tex, original)` returns the real size in both cases. Check the few spots (`imageImport.cpp`, `tiles.cpp`, `cTextureAtlas.cpp`) that compute strides from `Texture_GetWidth(tex)` without `true`.
6. **`Texture_Lock/Unlock`**: on lock, `copyToImage()` (or keep a CPU `sf::Image` shadow), convert RGBA → ARGB into a `std::vector<DWORD>`, and return the pointer (respect the region arguments `cDataController.cpp:272` uses). On unlock (if not read-only), convert back and `texture.update(...)`. This is used by the texture atlas, tiles, image import, pie chart and guichan `hgeimage`.
7. **Render targets**: `Target_Create` → `sf::RenderTexture`. `Gfx_BeginScene(target)` makes it the current draw target (flush first). `Gfx_EndScene` flushes and calls `rt.display()` (this fixes the Y-flip). `Target_GetTexture` returns a `TexEntry` that wraps `rt.getTexture()`. `Target_GetSurface` + `Surface_Lock` → `copyToImage()` + ARGB convert (mapshot). Keep a gfx-restore callback hook, but SFML/GL doesn't lose resources the way D3D9 did, so `GfxRestoreFunc` will mostly never fire.
8. **Clipping**: `Gfx_SetClipping(x,y,w,h)` → flush, then set an `sf::View` covering the target with `setScissor(normalised rect)`. `(0,0,0,0)` = no clipping. Keep the current value for `Gfx_GetClipping`. (Fallback if scissor behaves oddly: `glEnable(GL_SCISSOR_TEST); glScissor` with the Y-flip, inside `target.setActive()`.)
9. **`Gfx_SetTransform`** (guichan offsets): store it as an `sf::Transform` translation and apply it to the batch's `RenderStates`.
10. `Gfx_Clear(color)` → `target.clear()`.
11. ✅ Check: map viewport, tile and object rendering, layer buffers (`cLayerRenderBuffer`), state buffer (`cStateMgr`), rulers, selection lines, mapshot export. Compare with the Step 0 screenshots, ideally overlaid.

### Step 7: Sprites, fonts and guichan
1. `hgeSprite`, `hgeFont` (they parse the existing `.fnt` files), `hgeVector`, `hgeRect` should work as soon as Step 6 is done, because they only call `hge->Gfx_RenderQuad`, `Texture_GetWidth` and so on. Check `Render`, `RenderEx`, `RenderStretch`, `Render4V`, `SetFlip`, `SetHotSpot`, `SetTextureRect`, `SetBlendMode`, `printf/printfb`, `GetStringWidth`.
2. The guichan backend (`shared/guichan/hge/*.cpp`) needs no changes. Test every widget type: buttons, checkboxes, radio buttons, dropdowns, sliders, scroll areas, tabbed areas, text fields and boxes, trees, context menus, tooltips, windows (drag and focus).
3. ✅ Check: the whole UI pass from the Step 0 checklist.

### Step 8: Audio (replaces BASS)
In `audio.cpp`:
1. `Effect_Load(mem,len)` → `new sf::SoundBuffer` + `loadFromMemory`. Check the Claw WAV variants (8-bit and 16-bit PCM, mono and stereo all load; ADPCM WAVs don't, so if any exist, decode them with `dr_wav` into samples and use `loadFromSamples`).
2. Channel pool: a `std::vector<std::unique_ptr<sf::Sound>>` with generation-tagged handles so a stale `HCHANNEL` can't hit a reused slot. Reuse finished sounds (SFML/OpenAL caps at roughly 256 voices).
3. `Effect_PlayEx(eff, volume 0-100, pan -100..100, pitch, loop)` → `setVolume`, `setPan` (SFML 3) or position, `setPitch`, `setLooping`. Channel functions map directly: `stop`, `pause`, `play`, `getStatus()==Playing`, `setVolume`, `setPlayingOffset(sf::seconds(pos))`, `getPlayingOffset`, `buffer.getDuration()`.
4. Delete `bass.dll` from the distribution.
5. ✅ Check: sound browser (play, pause, seek, volume, end-of-track icon reset) and game simulator sounds and ambients.

### Step 9: Cleanup
1. Remove the `../hge` dependency, DirectX SDK references, `/NODEFAULTLIB:libci`, and `bass.dll` from both `CMakeLists.txt` and `WapMap.vcxproj`.
2. Delete the now-unused shader code in `cStateMgr.cpp`.
3. Update `README.md` build steps (no DirectX SDK, no VS2017 toolset, SFML 3 via vcpkg/FetchContent).
4. Check the other projects in the repo (`Gif2Pid`, `WMUpdater`, `ShellExtensionHandlers`) for HGE use and point them at `hgecompat` too.

### Step 10: Full regression pass
Run the full Step 0 checklist and compare screenshots. Also check performance on the largest map available (frame time while panning at max zoom-out). If it's slow, look first at batch flush counts. Usually this means texture switching between tiles from different atlases.

### Step 11 (optional): x64
Rebuild Lua and curl for x64, switch the triplet, and fix any `DWORD`/pointer-size casts (`(DWORD)ptr` patterns, HWND stored in `int` via `System_GetState`, and so on).

### Step 12: Dear ImGui alongside guichan, with real window drag-out

**Goal:** ImGui panels (new tool/debug windows, and optionally existing guichan windows wrapped as described in 12.6) can be dragged outside the main window and become their own OS window — true multi-viewport, not just docking tabs.

**Why this needs non-default repos** (researched 2026-10-13, see chat for the full investigation): ImGui's `master` branch has never had multi-viewport support — only the `docking` branch does, and it hasn't merged in 6+ years. The canonical `imgui-sfml` (`github.com/SFML/imgui-sfml`) targets SFML 3 but has **no** viewport backend; the only implementation (`nocheacosador/imgui-sfml`, PR #213 / branch `viewports`) is unmerged and unmaintained since 2022, written against SFML 2. Verified facts that make porting it low-risk:
- Every `ImGuiPlatformIO` hook the 2022 backend sets (`Platform_CreateWindow/DestroyWindow/ShowWindow/SetWindowPos/GetWindowPos/SetWindowSize/GetWindowSize/SetWindowTitle/SetWindowFocus/GetWindowFocus/GetWindowMinimized/SetWindowAlpha/UpdateWindow/RenderWindow/SwapBuffers`, `Monitors`, `Viewports`) still exists under the same name in today's `docking` branch.
- It already sets `ImGuiBackendFlags_HasMouseHoveredViewport` (correct hover detection across OS windows) — a detail many hand-rolled backends skip.
- The feature is isolated: diffing it against its exact merge-base shows **only `imgui-SFML.cpp` (463 added lines), `CMakeLists.txt`, and `.gitignore` changed** — nothing else in the repo. It's also already `#ifdef IMGUI_SFML_VIEWPORTS_ENABLE`-gated behind a CMake option (`IMGUI_SFML_BUILD_VIEWPORTS`/`IMGUI_SFML_ENABLE_VIEWPORTS`), so it's additive, not a fork of the whole render path.
- Of those 463 lines, **~253 sit inside `#ifdef _WIN32` blocks**; the rest is Linux/X11/macOS/Android code that WapMap (Windows-only) can delete outright, roughly halving the real porting surface.
- It uses only `<SFML/OpenGL.hpp>` (SFML's own bundled legacy-GL header) and plain `window->setActive(true)` for context switching — no glad/gl3w/GLEW dependency to add, and no special "shared context" setup beyond what SFML already does per-thread. `opengl32.lib` is already linked in `CMakeLists.txt` today.
- The real porting work is syntax, not design: the 2022 branch uses SFML 2's polymorphic `sf::Event`/`.type==`; current SFML 3 uses the variant-style `event.getIf<sf::Event::Resized>()`/`event.is<...>()`, plus `std::optional<sf::Texture>` returns elsewhere in the file. This is the same kind of mechanical port Step 1 already does for 4 other files — just one more file, with a clear reference (the rest of the canonical `imgui-SFML.cpp`) showing the target style line-by-line.

#### 12.1 Prep the `imgui` repo (`C:\Projects\ai-learning\imgui`)

Current state: `origin` = `Zax37/imgui` (your fork), `upstream` = `ocornut/imgui`, checked out on a stale `master` from 2024-05-16. Point it at the live `docking` branch instead:

```bash
cd /c/Projects/ai-learning/imgui
git fetch upstream docking
git checkout -b wapmap upstream/docking
git push -u origin wapmap
```

To pick up later `docking` updates (ocornut periodically rebases this branch rather than merging into it, so treat it as "reset to latest", not "merge"):

```bash
git fetch upstream docking
git reset --hard upstream/docking
git push --force origin wapmap
```
Only do the reset if `wapmap` has no commits of its own on top — if you ever patch something locally, rebase those commits instead of force-resetting over them.

#### 12.2 Prep the `imgui-sfml` repo (`C:\Projects\ai-learning\imgui-sfml`)

Current state: `origin` = `Zax37/imgui-sfml`, `upstream` = `nocheacosador/imgui-sfml` (this is the author of the viewport branch — that's why it's already a remote), checked out on a stale `master` from 2022-05-25. The canonical, SFML-3-targeting repo (`github.com/SFML/imgui-sfml`) is not yet a remote here.

**Step 1 — extract the viewport feature as a clean patch**, isolated from its real merge-base (`a80d9547832c57a149cda452ed9456def5bb6328`, verified ancestor of both `upstream/master` and `upstream/viewports`):

```bash
cd /c/Projects/ai-learning/imgui-sfml
git fetch upstream viewports
git diff a80d9547832c57a149cda452ed9456def5bb6328 upstream/viewports \
  -- imgui-SFML.cpp CMakeLists.txt .gitignore > ../imgui-sfml-viewports.patch
```

**Step 2 — branch off the current SFML-3 canonical master:**

```bash
git remote add sfml-org https://github.com/SFML/imgui-sfml.git
git fetch sfml-org master
git checkout -b wapmap sfml-org/master
```

**Step 3 — apply the patch and port it:**

```bash
git apply --3way --reject ../imgui-sfml-viewports.patch
```
This will not apply cleanly — the SFML 2→3 rewrite touches the same window/event code the patch does. Expect `.rej` files. Resolve them by hand, file by file, using the rest of `imgui-SFML.cpp` on this branch as the style reference:
- Delete the non-Windows branches (`#elif defined(__linux__) || defined(__unix__)`, `#elif defined(__APPLE__)`, Android) inside the ported code — WapMap doesn't need them, and it removes about half the lines to port.
- Convert `sf::Event` handling in the ported code from `.type == sf::Event::X` to `event.is<sf::Event::X>()` / `event.getIf<sf::Event::X>()`, matching how the rest of this file's `ProcessEvent`/`Update` already do it (see around line 371 onward in `wapmap`'s `imgui-SFML.cpp`).
- Check window creation/move/resize calls (`sf::Window` construction, `setPosition`/`getPosition`/`setSize`) against SFML 3's current signatures (several return `std::optional` or take `sf::Vector2` types now) — build errors will point at every spot that needs this.
- Re-add the CMake option from the patch (`IMGUI_SFML_ENABLE_VIEWPORTS` → `-DIMGUI_SFML_VIEWPORTS_ENABLE`) to `CMakeLists.txt` on this branch, since the ported code should stay behind that `#ifdef` (keeps the viewport code opt-out-able and easy to diff against upstream later).

**Step 4 — verify and push**, using the bundled `examples/minimal/main.cpp` (the patch touched it too, 14 lines — it's the reference integration showing `ImGuiConfigFlags_ViewportsEnable` enabled end-to-end) as your first smoke test before wiring it into WapMap:

```bash
git add -A
git commit -m "Port nocheacosador/imgui-sfml viewports branch to SFML 3, Windows-only"
git push -u origin wapmap
```

#### 12.3 Wire both into WapMap's `CMakeLists.txt`

1. Add variables next to the existing dependency ones:
   ```cmake
   SET(IMGUI_DIR "${PROJECT_SOURCE_DIR}/../imgui")
   SET(IMGUI_SFML_DIR "${PROJECT_SOURCE_DIR}/../imgui-sfml")
   ```
2. Add to `include_directories(...)`: `"${IMGUI_DIR}"`, `"${IMGUI_SFML_DIR}"`.
3. Add to the `add_executable(WapMap ...)` source list: `${IMGUI_DIR}/imgui.cpp`, `imgui_draw.cpp`, `imgui_tables.cpp`, `imgui_widgets.cpp` (skip `imgui_demo.cpp` in the real build), and `${IMGUI_SFML_DIR}/imgui-SFML.cpp`.
4. Add `-DIMGUI_SFML_VIEWPORTS_ENABLE` to `add_definitions(...)` (matches the `#ifdef` from Step 12.2's port).
5. No new link libraries needed — `opengl32.lib` is already in `OTHER_LIBS`, and both `imgui-SFML.cpp` and SFML's own GL use only `<SFML/OpenGL.hpp>`.
6. Make sure the `imgui`/`imgui-sfml` checkouts the build points at are on the `wapmap` branches from Steps 12.1–12.2, and update `WapMap/README.md`'s clone instructions to add:
   ```
   git clone https://github.com/Zax37/imgui -b wapmap
   git clone https://github.com/Zax37/imgui-sfml -b wapmap
   ```
   next to the existing `hge`/`curl`/`lua` clone lines.

#### 12.4 Main-loop integration

In the HGE-facade's `System_Initiate`/`RunOneFrame` (Step 3):
1. After window creation: `ImGui::CreateContext(); ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable; ImGui::SFML::Init(window);`
2. In the event loop, alongside the existing input translation (Step 5): `ImGui::SFML::ProcessEvent(window, event);` — this is non-destructive, both ImGui and the HGE-facade input code can see the same event.
3. Once per frame, before `frameFunc()`: `ImGui::SFML::Update(window, deltaClock.restart());`, then `ImGui::NewFrame()`, build/draw ImGui panels.
4. End of frame, after guichan/game rendering but before `window.display()`: `ImGui::SFML::Render(window);` for the main viewport, then `ImGui::UpdatePlatformWindows(); ImGui::RenderPlatformWindowsDefault();` to draw and swap the secondary OS windows the viewport backend created. This is the standard docking-branch pattern — match it against `examples/minimal/main.cpp` on the ported branch.
5. Input priority: if `ImGui::GetIO().WantCaptureMouse/Keyboard` is set, don't forward that event into the HGE-facade input state or the guichan input queue.

#### 12.5 What drag-out actually applies to

Multi-viewport only detaches **ImGui windows**. Existing guichan windows (tile browser, palette browser, object properties, etc.) are not ImGui windows — they're widgets drawn onto the single SFML surface by the guichan HGE-backend (Step 7) — so by default they **cannot** be dragged out, no matter which backend is used. Plain new ImGui windows (an inspector, a performance overlay, a Lua console) get drag-out for free as soon as 12.1–12.4 are done.

#### 12.6 (optional) Making an *existing* guichan window drag-out-able without rewriting it

If the actual goal is to drag out one of today's guichan windows specifically, there's a middle ground short of rewriting its widgets to native ImGui:
1. Give that one guichan window its own `sf::RenderTexture` and its own `gcn::Gui` instance (same technique as `cStateMgr`'s buffer, Step 6.7), sized to the window's content.
2. Host it as `ImGui::Begin("Tile Browser"); ImGui::Image(texture); ImGui::End();` — this makes it a real ImGui window, so it docks and drags out like any other.
3. Forward mouse/keyboard input from ImGui's window-local coordinates into that guichan `Gui` instance manually (translate `ImGui::GetMousePos() - ImGui::GetItemRectMin()` into the texture's local space, feed it the same way `hgeinput.cpp` does today).
4. Do this one window at a time, starting with something low-traffic (About, Go To Coords) to validate the technique before touching anything load-bearing (tile/object property editors).

#### 12.7 Test checklist (add to Step 10's regression pass)
- Drag a plain ImGui panel out past the main window's edge → a separate OS window appears, is independently movable/resizable, stays interactive, and reports correctly to Windows (shows in Alt-Tab/taskbar as expected for a tool window).
- Close that detached window via its own close button and via dragging it back into the main window to re-dock.
- Multiple panels detached simultaneously; move the main window — detached ones should not jump or lose their position.
- Minimize/restore the main window with panels detached.
- If 12.6 was done for a guichan window: detach it, interact with every one of its widgets, and confirm input doesn't also leak to whatever's behind it in the main window.

### Step 13 (optional, long-term): remove the facade
Gradually replace `hge->Gfx_RenderLine/RenderQuad` call sites and `hgeSprite` with a small native WapMap renderer API on SFML types. The facade means this never has to happen all at once.

---

## 4. Risks and gotchas

| Risk | Mitigation |
|---|---|
| Pixel or half-texel differences (D3D9 vs GL) | +0.5 offsets for lines; compare screenshots; `setSmooth(false)`. |
| RenderTexture Y-flip | Always call `rt.display()` before sampling (in `Gfx_EndScene`). |
| ARGB vs RGBA | Do the conversion in one place only (the facade's emit, lock and unlock). |
| Too many draw calls | The batcher (Step 6.1) is required, not optional. |
| SFML overriding cursor or window proc behaviour | Subclass the window proc (Step 4), handle `WM_SETCURSOR`. |
| Modal dialogs re-entering the loop | `System_DoManualMainLoop` → `RunOneFrame()`; make sure it doesn't recurse into itself. |
| Text input encoding | Match the old HGE `chr` behaviour (ANSI); test Polish and other non-ASCII characters, since `cLang` uses localised strings. |
| Exotic WAV formats | Fall back to `dr_wav` decoding. |

## 5. Rough effort

| Step | Effort |
|---|---|
| 0–2 | 1–2 days |
| 3–5 (window, Win32, input) | 3–4 days |
| 6–7 (graphics, batcher, guichan verification) | 5–8 days |
| 8 (audio) | 1–2 days |
| 9–10 (cleanup, regression) | 2–3 days |
| **Total (HGE → SFML core, Steps 0–10)** | **about 2.5–4 weeks for one developer**, with zero changes to editor or UI logic |
| 12 (ImGui docking+viewports: port the backend, wire it in, test) | 3–5 days — most of it is Step 12.2's port, which is bounded (463 lines, ~253 Windows-relevant) and has the rest of the file as a style reference |
| 12.6 per guichan window converted to the texture+ImGui::Image wrapper (optional, only if existing windows need drag-out, not just new ones) | 0.5–1 day each |

For comparison, rewriting the UI in ImGui would mean touching 159 files and about 850 widget instantiations, which is an estimated several months.
