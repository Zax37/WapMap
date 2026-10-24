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

### Review and corrections (2026-10-14)

The facade + retained guichan approach remains appropriate. Compilation and smoke
tests establish progress, not full visual/input equivalence. In particular, the
original expectation that the guichan backend needs no changes was too strong:
its mouse adapter sampled final frame state for every queued event, dropping
press/release pairs within one frame and repeating wheel input for unrelated
events. It now consumes ordered mouse transitions and wheel deltas directly;
focus-loss releases also reach the adapter.

Corrections in this review:
- Begin each graphics scene with full-target clipping and identity transform;
  reject nested scenes. Reset the guichan transform when its clip stack empties.
  This prevents an object-properties/preview pass from clipping the next frame's
  toolbar background.
- Match HGE's transform contract: zero vertical scale means identity, and rotation
  has the opposite sign from SFML's default transform rotation.
- Restore the default client cursor before legacy mouse handling and suppress
  ImGui's main cursor update when it does not capture the mouse. Begin the ImGui
  frame after legacy frame callbacks, so their modal loops cannot re-enter an
  already active ImGui frame.
- Reconcile mouse position, focus and native window ownership once per frame.
  Return that snapshot to all legacy polling callers; retain outside-client
  coordinates during legacy drags. Clear captured-input ownership on focus loss.
- Clear menu keyboard highlighting when the pointer moves, including movement
  that bypasses the menu listener in maximized mode.

Validation: Release x64 and Win32 with ImGui build successfully. The standalone regression
suite in `tests/migration` renders into real SFML render textures and reads pixels
back; it also exercises the real guichan mouse adapter. It covers clipping across
passes, transform reset, nested-scene rejection, the identity-transform sentinel,
ordered same-frame clicks, simultaneous
buttons, focus-loss releases, mouse-leave coordinates and wheel duplication.
Interactive confirmation of the three reported visual symptoms remains pending.

Follow-up: the cursor fix was confirmed interactively, but toolbar/status-bar
backgrounds still disappeared while Object Properties' Cancel button was visible.
A real render-texture regression reproduced the cause: imgui-sfml's
`pushGLStates`/`popGLStates` restores the last widget's actual GL scissor after
SFML has reset its cache to scissor-disabled. Resetting only the view at the next
scene cannot disable that stale scissor. `Gfx_BeginScene` now reconciles the actual
GL state with `resetGLStates` once per pass before applying the view. The regression
checks full background coverage after this exact save/reset/restore sequence.

Remaining architectural gates:
- CMake defaults ImGui to OFF; the current Visual Studio project explicitly
  enables it in all configurations. Do not describe those builds as equivalent.
- The multi-viewport backend source revision is recorded in the 2026-10-24 audit
  below. Builds still consume sibling checkouts; use the documented revisions.
- `BLEND_COLORADD` is texture-stage colour addition in HGE, not framebuffer
  additive blending. The facade's `BlendAdd` approximation is not exact parity.
  Verify button hover against the reference before calling blend support complete.
- Step 12.6 is a separate UI integration project. A texture wrapper alone cannot
  isolate legacy globals, screen-coordinate assumptions, modal focus or input
  ownership. Those need explicit per-panel routing before interactive guichan
  windows can safely detach.
- Keep manual mapshot/import, audio/ADPCM, non-ASCII input, detached-window focus,
  minimize/restore, multi-monitor/DPI and performance checks open. Do not advance
  them to complete based only on successful builds.

Run the focused tests with a matching SFML installation and architecture:
```powershell
cmake -S tests/migration -B obj/migration-tests -A x64 `
  -DCMAKE_PREFIX_PATH=D:/Projects/SFML-3.0.2-x64-install -DSFML_STATIC_LIBRARIES=ON
cmake --build obj/migration-tests --config Release --parallel 4
ctest --test-dir obj/migration-tests -C Release --output-on-failure
```

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
1. ✅ Build/install **SFML 3.0.2** for x86 and link its graphics, window, system, and audio components from CMake.
2. ✅ Port the existing SFML call sites (`states/mapshot.*`, `shared/cPieChart.cpp`, `objEdit/editCandy.cpp`, `windows/about.cpp`) to SFML 3 APIs.
3. ✅ Check: the app builds against SFML 3 and the in-tree compatibility renderer. Runtime checks are tracked in Step 10.

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

### Step 6: Graphics core and batching (implementation complete; runtime validation pending)
The SFML batcher, primitive rendering, textures and locks, render targets, clipping, and transforms are implemented in `WapMap/hgecompat/src/graphics.cpp`. The x86 Release target builds; the manual rendering and screenshot comparisons below still need to be completed.

In `graphics.cpp`:
1. **Batcher**: keep a vertex vector (Triangles) plus the current texture, blend mode, render target, clip rect and transform. Every `Gfx_RenderQuad/Triple` adds vertices. **Flush** when any of those state values changes, or on `Gfx_EndScene`. (HGE batches the same way; drawing every sprite separately in SFML would be too slow for tile maps.)
2. **Lines**: `Gfx_RenderLine` goes to a separate `sf::PrimitiveType::Lines` batch (flush ordering matters: keep a single queue of mixed primitives, or flush the triangles before lines and the other way round). Add **+0.5 px** to coordinates so 1-px lines stay sharp (D3D9 and GL rasterise lines differently).
3. **Colour**: HGE's `DWORD` is `0xAARRGGBB`. Convert with `sf::Color(c>>16&255, c>>8&255, c&255, c>>24)`. Texture coordinates: HGE uses normalised 0..1 UVs and SFML uses pixels, so multiply by the texture size when emitting vertices.
4. **Blend modes**: per-vertex colour × texture (`COLORMUL`) is SFML's default. `ALPHABLEND` → `sf::BlendAlpha`, additive modes including the single `COLORADD` use → `sf::BlendAdd`, `NOZWRITE`/Z → ignore. Verify the button hover appearance during manual regression.
5. **Textures**: `Texture_Load(mem/file)` → `sf::Texture::loadFromMemory/File`, `setSmooth(false)` (`HGE_TEXTUREFILTER=false`). `Texture_Create(w,h)` → empty texture. **Don't pad to power of two.** `GetWidth(tex, original)` returns the real size in both cases. Check the few spots (`imageImport.cpp`, `tiles.cpp`, `cTextureAtlas.cpp`) that compute strides from `Texture_GetWidth(tex)` without `true`.
6. **`Texture_Lock/Unlock`**: on lock, `copyToImage()` (or keep a CPU `sf::Image` shadow), convert RGBA → ARGB into a `std::vector<DWORD>`, and return the pointer (respect the region arguments `cDataController.cpp:272` uses). On unlock (if not read-only), convert back and `texture.update(...)`. This is used by the texture atlas, tiles, image import, pie chart and guichan `hgeimage`.
7. **Render targets**: `Target_Create` → `sf::RenderTexture`. `Gfx_BeginScene(target)` makes it the current draw target (flush first). `Gfx_EndScene` flushes and calls `rt.display()` (this fixes the Y-flip). `Target_GetTexture` returns a `TexEntry` that wraps `rt.getTexture()`. `Target_GetSurface` + `Surface_Lock` → `copyToImage()` + ARGB convert (mapshot). Keep a gfx-restore callback hook, but SFML/GL doesn't lose resources the way D3D9 did, so `GfxRestoreFunc` will mostly never fire.
8. **Clipping**: `Gfx_SetClipping(x,y,w,h)` → flush, then set an `sf::View` covering the target with `setScissor(normalised rect)`. `(0,0,0,0)` = no clipping. Keep the current value for `Gfx_GetClipping`. (Fallback if scissor behaves oddly: `glEnable(GL_SCISSOR_TEST); glScissor` with the Y-flip, inside `target.setActive()`.)
9. **`Gfx_SetTransform`** (guichan offsets): store it as an `sf::Transform` translation and apply it to the batch's `RenderStates`.
10. `Gfx_Clear(color)` → `target.clear()`.
11. ✅ Check: map viewport, tile and object rendering, layer buffers (`cLayerRenderBuffer`), state buffer (`cStateMgr`), rulers, selection lines, mapshot export. Compare with the Step 0 screenshots, ideally overlaid.

### Step 7: Sprites, fonts and guichan (implementation compiles; runtime validation pending)
The existing HGE sprite/font helpers and guichan HGE backend compile against the SFML graphics layer without backend changes. Runtime widget and visual checks remain pending until the graphics pass can be exercised with the application's external SFS assets.
1. `hgeSprite`, `hgeFont` (they parse the existing `.fnt` files), `hgeVector`, `hgeRect` should work as soon as Step 6 is done, because they only call `hge->Gfx_RenderQuad`, `Texture_GetWidth` and so on. Check `Render`, `RenderEx`, `RenderStretch`, `Render4V`, `SetFlip`, `SetHotSpot`, `SetTextureRect`, `SetBlendMode`, `printf/printfb`, `GetStringWidth`.
2. The guichan backend (`shared/guichan/hge/*.cpp`) needs no changes. Test every widget type: buttons, checkboxes, radio buttons, dropdowns, sliders, scroll areas, tabbed areas, text fields and boxes, trees, context menus, tooltips, windows (drag and focus).
3. ✅ Check: the whole UI pass from the Step 0 checklist.

### Step 8: Audio (replaces BASS; implementation complete, runtime validation pending)
`WapMap/hgecompat/src/audio.cpp` now loads SFML sound buffers from memory or files and provides pooled playback channels with generation-checked handles, volume, pan, pitch, looping, seeking, and FX master volume. The x86 Release build succeeds. The Claw sound browser populated from the installation's loose WAV assets, and a selected sample reported a one-second duration. Audible playback, seek/volume interaction, simulator playback, and packed Claw ADPCM decoding remain unverified.

In `audio.cpp`:
1. `Effect_Load(mem,len)` → `sf::SoundBuffer::loadFromMemory`. Check the Claw WAV variants (8-bit and 16-bit PCM, mono and stereo all load; ADPCM WAVs may not, so if any exist, decode them with `dr_wav` into samples and use `loadFromSamples`).
2. Channel pool: a `std::vector<std::unique_ptr<sf::Sound>>` with generation-tagged handles so a stale `HCHANNEL` can't hit a reused slot. Reuse finished sounds (SFML/OpenAL caps at roughly 256 voices).
3. `Effect_PlayEx(eff, volume 0-100, pan -100..100, pitch, loop)` → `setVolume`, `setPan` (SFML 3) or position, `setPitch`, `setLooping`. Channel functions map directly: `stop`, `pause`, `play`, `getStatus()==Playing`, `setVolume`, `setPlayingOffset(sf::seconds(pos))`, `getPlayingOffset`, `buffer.getDuration()`.
4. ✅ Delete `res/copy/bass.dll` from the distribution.
5. ⏳ Check sound browser (play, pause, seek, volume, end-of-track icon reset) and game simulator sounds and ambients; runtime UI displayed the sample list and a valid one-second duration, but playback controls still need direct confirmation.

### Step 9: Cleanup
1. ✅ Remove HGE/DirectX link dependencies from the main CMake and Visual Studio projects. CMake now requires a standalone zlib package.
2. ✅ Remove inactive shader declarations and implementation while retaining the console command's explicit unsupported message.
3. ✅ Update the README with the current x86 SFML 3 build prerequisites and CMake instructions.
4. ✅ Check auxiliary projects: Gif2Pid only needs the compatibility header types, ShellExtensionHandlers had no HGE API calls, and WMUpdater has no HGE dependency. Remove obsolete HGE/zlib link paths where applicable.
5. ✅ Reconfigure and rebuild the x86 Release target. ⏳ Complete the runtime rendering, input, and audio checks in Step 10.

### Step 10: Full regression pass
✅ Startup smoke test: the app initializes with the Win32 window hooks and OLE file-drop support enabled, remains responsive for at least 12 seconds, and reaches `Everything OK.` / `All up to date!`. A screenshot confirmed the welcome screen renders.

✅ Claw map smoke test: configured the local Claw install path and opened `Assets/LEVEL1/WORLDS/WORLD.WWD`. After initial asset loading, the level rendered with its real tile and object textures. Mouse-wheel zoom, middle-button panning, and the Ctrl+M tile/object mode toggle all responded without a crash.

✅ Additional runtime checks: the sound browser populated and showed a valid one-second Claw WAV duration; resizing the main window to 1280×720 kept the app responsive and map/assets rendering visible. The largest bundled level map (`Assets/LEVEL13/WORLDS/WORLD.WWD`) eventually rendered with real assets. Its initial load took about two minutes and temporarily showed Windows as not responding, before becoming responsive. At maximum zoom-out, a rough idle sample used 6.95 process CPU seconds over 10 seconds of wall time (~0.70 CPU-core equivalents); no HGE baseline or frame-time measurement is available.

The remaining Step 0 checklist, reference screenshot comparisons, tile/object editing operations, widgets, confirmed audio playback/seek/volume, simulator playback, mapshot/image import, file-drop/window-resize-grip/hotkey/text-input checks, and panning performance measurements are still pending. The long cold-load path was traced to the SFML implementation of `Texture_Lock`/`Texture_Unlock`: every tile copied its entire atlas from the GPU to the CPU and uploaded the full atlas again. The compatibility layer now caches texture pixels on the CPU and uploads only the modified lock rectangle. Follow-up zoomed-out rendering work replaced linear tile-ID lookup with binary search, quadratic vector-front erasure with a linear pass, and quadratic quadtree-result deduplication with hash-based deduplication. Re-time the level-13 cold load and zoomed-out panning/rendering; these source-level changes still need runtime benchmarking.

The Debug-only x86 exit crash was traced to the memory-usage worker outliving `cGlobals` and its critical section. The worker is tracked and joined before teardown and cannot overlap another sample. A separate Release x86 shutdown crash was identified from its call stack: `EditingWW::Destroy()` detached the active viewport before `cMDI` destroyed its documents, then `DocumentSwitched()` attempted to remove that widget a second time. `EditingWW::Destroy()` now leaves viewport detachment to the document-switch cleanup before the container is destroyed.

### Step 11 (optional): x64
✅ Rebuilt Lua 5.5, curl, zlib 1.2.5, and SFML 3.0.2 for x64 in separate dependency prefixes, preserving the x86 builds. Added x64 configurations and isolated output directories to the Visual Studio solution and CMake build.

✅ Fixed pointer-width assumptions in the application, including pointer-valued console variables and state return codes, Lua pointer handles, callback indices, and pointer-derived IDs. Corrected x64 `size_t` formatting in image import and asset-loading progress messages.

✅ Verified CMake Debug and Release x64 builds and a Visual Studio Release x64 build. The produced executables have PE machine type `0x8664` (AMD64).

The Visual Studio build still reports existing `size_t`-to-`int` conversion warnings (C4267) at 181 distinct source locations; these were not comprehensively audited as part of enabling the x64 build. Runtime behavior, including compatibility with third-party Lua brush scripts, still needs validation. Keep the remaining interaction, audio, mapshot/import, and performance checks listed in Step 10 open.

An x64 startup attempt exposed heap corruption in `hgeFont`: its float arrays were zeroed using the byte size of the pointer array, which is twice as large on x64. Each array now uses its own size. The x64 Debug target rebuilds successfully; verify startup at runtime.

An x64 map-click crash in `hgeFont::printf` was caused by manually locating variadic arguments on the stack, which is not valid for the x64 calling convention. `printf` and `printfb` now use `va_list`; the x64 Release target rebuilds successfully. The Debug link could not overwrite its executable/PDB while that build was running, so close the running Debug instance before rebuilding Debug. Verify map interaction in the rebuilt application.

### Step 12: Dear ImGui alongside guichan, with real window drag-out

**Goal:** ImGui panels (new tool/debug windows, and optionally existing guichan windows wrapped as described in 12.6) can be dragged outside the main window and become their own OS window — true multi-viewport, not just docking tabs.

ImGui's `docking` branch is required for multi-viewport support; the canonical SFML 3 backend did not provide platform-window support, while the historical third-party implementation targeted SFML 2. Its callbacks and event processing therefore had to be adapted to the current backend and SFML 3 APIs. The port stays in the separate imgui-sfml checkout and behind a build option rather than changing default WapMap behavior.

#### 12.1–12.2 Dependency branches and backend port

Local working checkouts are `D:\Projects\imgui` (branch `wapmap` based on upstream `docking`) and `D:\Projects\imgui-sfml` (branch `wapmap` based on canonical SFML `master`). The backend port is published as `8851118` on the WapMap fork's `wapmap` branch (https://github.com/Zax37/imgui-sfml). The matching local ImGui revision is `778992face67ecefaa5c2932db4b2101545ea4df`, version 1.93.0 WIP with docking/viewports enabled. See the 2026-10-24 audit for the current integration status.

The historical SFML 2 viewport patch did not apply cleanly to current SFML 3, so the backend hooks were ported manually in `imgui-SFML.cpp`. The optional `IMGUI_SFML_ENABLE_VIEWPORTS` CMake option enables platform-window creation/destruction, window position/size/focus/title callbacks, secondary-window input polling, monitor enumeration on Windows, hovered-viewport reporting from the OS cursor position, and render/swap callbacks. `ImGui::SFML::RenderPlatformWindows()` wraps ImGui's platform update/render pass and restores the main SFML context afterward.

Validation so far: the standard backend and the viewport-enabled backend both build against SFML 3.0.2 and ImGui 1.93.0. Interactive detach, close, re-dock, and multi-monitor behavior still needs runtime testing.

#### 12.3–12.4 Optional WapMap integration

`WAPMAP_ENABLE_IMGUI` in the WapMap CMake project defaults to `OFF`. Enabling it adds the local imgui-sfml subdirectory with viewport support. Configure with `-DWAPMAP_ENABLE_IMGUI=ON`; override `IMGUI_DIR` and `IMGUI_SFML_DIR` if the sibling checkout paths differ.

The SFML backend creates and owns the ImGui context during `ImGui::SFML::Init`; WapMap then enables docking and viewports, allows windows to merge into the main viewport while they remain inside it, and disables native window decorations for detached viewports. Thus the ImGui window keeps its own title bar for dragging while avoiding a second OS title bar when it is detached. The facade forwards main-window events to ImGui, filters captured mouse/keyboard events from the legacy input queue while still allowing already-held inputs to release and preserving Alt+Enter/Alt+F4, aggregates focus across the main and detached windows, updates ImGui each frame, and renders the main and detached viewports after the existing HGE/guichan rendering. With the option enabled, the existing About menu item opens in the main viewport and can be dragged outside it into a borderless native window; its ImGui contents preserve the original logo, version, authors, and credits. Other guichan windows are unchanged. Normal builds retain existing behavior with the option disabled.

#### 12.5 What drag-out actually applies to

Multi-viewport only detaches **ImGui windows**. Existing guichan windows (tile browser, palette browser, object properties, etc.) are not ImGui windows — they're widgets drawn onto the single SFML surface by the guichan HGE-backend (Step 7) — so they cannot become independent OS windows merely by moving outside the main window. Their previous screen-edge movement clamp has been removed, and the HGE facade now preserves mouse coordinates outside the main client area while dragging. As a result, a guichan window can be moved partly or fully beyond the main window, where its contents are clipped; native drag-out still requires the separate render-target and input-forwarding bridge described below. With ImGui enabled, floating ImGui windows such as About merge into the main viewport while inside it and become borderless native windows only when dragged outside.

#### 12.6 (optional) Making an *existing* guichan window drag-out-able without rewriting it

If the actual goal is to drag out one of today's guichan windows specifically, there's a middle ground short of rewriting its widgets to native ImGui:
1. Give that one guichan window its own `sf::RenderTexture` and its own `gcn::Gui` instance (same technique as `cStateMgr`'s buffer, Step 6.7), sized to the window's content.
2. Host it as `ImGui::Begin("Tile Browser"); ImGui::Image(texture); ImGui::End();` — this makes it a real ImGui window, so it docks and drags out like any other.
3. Forward mouse/keyboard input from ImGui's window-local coordinates into that guichan `Gui` instance manually (translate `ImGui::GetMousePos() - ImGui::GetItemRectMin()` into the texture's local space, feed it the same way `hgeinput.cpp` does today).
4. Do this one window at a time, starting with something low-traffic (About, Go To Coords) to validate the technique before touching anything load-bearing (tile/object property editors).

#### 12.7 Test checklist (add to Step 10's regression pass)
- Open About from the WapMap menu and drag its ImGui title bar beyond the main window's edge → a separate OS window appears, is independently movable/resizable, stays interactive, and reports correctly to Windows (shows in Alt-Tab/taskbar as expected for a tool window).
- Close that detached window via its own close button and via dragging it back into the main window to re-dock.
- Multiple panels detached simultaneously; move the main window — detached ones should not jump or lose their position.
- Minimize/restore the main window with panels detached.
- If 12.6 was done for a guichan window: detach it, interact with every one of its widgets, and confirm input doesn't also leak to whatever's behind it in the main window.

#### 12.8 Legacy appearance for ImGui panels

`WapMap/imguiTheme.h` provides the shared theme, initialized with the ImGui context:
opaque `#2f2f2f` bodies, `#373737` title bars, square one-pixel borders, centered
titles, grey controls and the existing blue accent. About now uses the existing
Myriad bitmap glyphs, logo and close-button atlas through ImGui draw-list image
commands, preserving their hotspots and advances without mutating HGE sprite
state. Its logo/version positions and author/credit spacing follow the legacy
layout. Credits wrap to the available width and participate in ImGui scrolling.
Opening About centers it in the main viewport; floating-window drag, close,
resize and multi-viewport behavior remain owned by ImGui. Docked tabs retain
native ImGui chrome. Other legacy windows continue using guichan.

The shared palette applies to future ImGui panels. A follow-up now imports the
original glyph pixels, hotspots and advances into the ImGui font atlas, mapping
legacy bytes to Unicode with the selected language's code page (1250/1251/1252),
before the first frame and uploads it through imgui-sfml. Native coordinate inputs
use those bitmap glyphs at their original size rather than scaling ImGui's default
font. ImGui's protected fallback and ellipsis glyphs retain their original atlas
entries: attempting to replace `?` triggered an assertion during startup. The
font regression now imports the full ASCII range with assertions enabled even in
Release, reproduces that failure before the fix, and checks fallback integrity
after the fix. Map Properties now uses strict UTF-8/legacy conversion with byte
limits and rejection of lossy encoding. Confirm visual
parity and detached-window rendering interactively.

#### 12.9 First interactive ImGui panel: Go To Coordinates

The existing menu item and keyboard shortcut now open a native ImGui Go To
Coordinates panel when `WAPMAP_ENABLE_IMGUI` is enabled. It retains both world
and tile coordinate groups, fills them from the camera on open, targets tile
centers, and stays open after Go. Enter navigates and Escape closes the focused
panel. Switching documents refreshes the fields; closing the map closes the
panel. Navigation is disabled while another editor state is on top. Builds with
ImGui disabled retain the guichan panel.

The panel uses the shared legacy window chrome and original atlas skins for
radio controls and nine-slice buttons. Numeric fields use the imported bitmap
font. The original 160x228 layout, 120x20 fields and right-aligned Go button were
restored after screenshot review exposed the oversized default ImGui controls.
Input remains owned by ImGui, including tab/keyboard navigation. The font
regression checks glyph pixels, dimensions, advances and native input metrics
across multiple ImGui frames. `cWindow` now exposes a common
ImGui draw hook and virtual open-state query, so migrated panels participate in
normal window lifecycle handling. About uses those hooks too, including Escape.

World/tile conversion is shared between both UI implementations. It rejects
invalid numbers and overflowing tile multiplication without throwing exceptions,
and preserves empty-field-as-zero behavior. The regression suite covers tile
centers, signed coordinates, integer limits, incomplete input and invalid tile
dimensions. Interactive main/detached input routing, switching maps and visual
checks remain pending; do not migrate larger editing panels until these pass.

#### 12.10 Map Properties

With ImGui enabled, the existing Map Properties menu opens a detachable native
ImGui panel with the original 460x290 layout, field dimensions, separators,
bitmap font, checkbox skins and nine-slice OK/Cancel buttons. Date remains
disabled. The panel holds its own draft tied to the originating parser; Cancel,
Escape, the close button and document switching discard it without editing the
map. Opening always refreshes the draft from the active document.

OK converts all editable UTF-8 fields back to the selected legacy code page and
enforces the existing limits (name 58 bytes, author 63, REZ path 255, tile/palette/
executable paths 127). Unsupported characters or overlong text disable OK and
mark the affected field; its tooltip explains the limit. Existing required-field
validation, level-name normalization, flag changes and unsaved marking still run
through `SaveWorldOptions`, which now reports success to the new panel. Validation
dialogs temporarily hide the ImGui panel so they remain visible and interactive.
Without ImGui, the original guichan panel remains in use.

Tests cover Polish legacy/UTF-8 round trips, encoded-byte limits, rejection of
invalid UTF-8/lossy conversion and extended bitmap glyph metrics in native ImGui
frames. Interactive OK/Cancel, save/reopen, validation dialogs, map switching and
detached-window behavior remain to be checked with a real map.

#### 12.11 Fixed About and Plane Properties

About now has `NoResize` and restores its original dimensions on every frame;
dragging/detaching and content scrolling remain available.

Plane Properties is the next native ImGui panel. It retains the legacy plane
list (including New Plane), tileset selection, numeric fields, hidden offset
values, nine resize anchors, flags, Save and Delete. Controls use the shared
bitmap font and original checkbox/button/icon textures. Drafts are tied to the
document and plane identity; switching documents or an external plane-list
change closes the panel rather than applying a stale index. Selecting another
plane refreshes its draft, as in the legacy UI.

The plane list uses the legacy scroll area's opaque `#131313` background.
Tileset choices combine loaded graphics with names declared by the document's
planes, so missing resource files do not erase the current assignment or leave
the dropdown empty. Save uses the same choices as the native dropdown.

Save and Delete reuse the existing plane operations, including validation,
resize-anchor behavior, main-plane protection, object-drop confirmations,
sorting, render-data updates and unsaved marking. They are queued from the UI
and executed during the next legacy frame callback. `SavePlaneProperties` now
reports success and checks the selection/tileset bounds. Integer overflow in tile
counts/pixel extents is rejected before Save. Tests cover the dimension guard.

ImGui NewFrame now runs after legacy frame callbacks, with cursor suppression
retained, because changing the main plane can run a synchronous modal loop.
ImGui panels are hidden while a different state is on top, allowing legacy
validation/confirmation dialogs to receive input and remain unobscured.

Interactive tests still required: edit/save/reopen, New Plane, resizing at all
nine anchors, changing Z/sorting, main-plane confirmations/protected deletion,
non-main deletion, switching maps and detached input. Do not consider these
complete based on builds or arithmetic/font tests alone.

#### 12.12 Tile Properties and New Map

Tile Properties is a native fixed-size ImGui panel with the original bitmap
font, radio/button skins, icons and collision colors. The preview supports
zoom, Double rectangle dragging and Mask rectangle painting in detached
viewports. Navigation uses the main plane's actual tileset rather than a
hard-coded ACTION set. Drafts are owned by the panel and discarded on tile or
document changes; Apply runs before NewFrame, checks the attribute-table index
and uses the existing parser save/unsaved logic. Mask resizing allocates a new
buffer, preserves the overlapping rectangle and clears new cells; it commits
on Enter/focus departure rather than cropping during intermediate keystrokes.
Allocation size and row-stride preservation have regression coverage.

New Map retains the existing game/level tree data, icons, folder expansion,
Claw bonus-plane option and map-creation path. Its native tree uses opaque
legacy scroll-area colors and keyboard-enabled rows. UTF-8 drafts are converted
with the existing byte limits before creation, and dimensions are checked for
integer overflow. Creation is queued until the legacy frame callback, allowing
validation dialogs and state transitions outside an active ImGui frame.

Interactive checks remain required: Tile Properties navigation, all three
attribute types, resize/paint/Apply/reopen and detached input; New Map expansion
and selection, each game's creation, Claw bonus planes, validation, Polish text,
close/reopen and detached input. Builds and helper tests do not complete these
runtime gates.

#### 12.13 Locations Browser

The fixed-size native panel retains Favorites, Checkpoints and Warps tabs,
the opaque legacy list background, selection color, bitmap text and six
skinned toolbar buttons. Row activation navigates to the selected location;
favorites, rename, delete, reorder and warp destination reuse existing editor
operations. Actions are queued before NewFrame, with document and location
identity checks and a guard against recursive synchronous input dialogs.
Changing documents closes the native browser. Deleted objects are pruned
before rendering, rather than erasing rows while generating their labels;
linked coordinates follow moved objects. Adding favorites now validates the
actual input without padding short names to 63 bytes or ignoring errors.

Runtime checks remain required: navigation in all tabs, favorite add/rename/
delete, checkpoint/warp delete and reorder persistence, warp destinations,
external object deletion/movement, map switching and detached input. Dialogs
for naming locations remain legacy dialogs.

#### 12.14 Measure Options, Tile Browser and Image Set Browser

Measure Options opens with the measurement tool in the viewport's bottom-right
corner. It uses the original checkbox and clear-button artwork. Absolute
distance reads the same retained checkbox state as the existing renderer;
Clear and closing the tool are applied before the next ImGui frame. Switching
tools closes the panel, while existing measurement points retain their prior
semantics.

Tile Browser and Image Set Browser now use native fixed-size ImGui panels.
Their two independently scrolling areas retain set thumbnails, counts, metadata,
source groups, original preview dimensions, checkerboard, bitmap labels and
button skins. A shared helper draws sprites without mutating HGE sprite state
and clips grid rows to the visible area. Frame previews preserve aspect ratio
and never enlarge small images. Existing custom-resource visibility/enabled
rules and import/delete/details operations remain in place; native actions are
queued before NewFrame and rebound by document/set/asset identity. Empty sets
and missing documents are guarded rather than dereferencing a missing asset.
Changing documents closes both browsers. Import and Image Details remain
existing editor states; options previously disabled are not newly implemented.
The adapter reads guichan controls' own visibility/enabled flags, independently
of their hidden retained parent. New Map's bonus-plane checkbox uses the same
local visibility check.

Runtime gates: measurement checkbox, Clear, close/reopen, tool switching and
detaching; browser selection, long-list scrolling, source grouping, empty banks,
resource refresh, custom-resource operations, map switching and detached input.
These require interactive verification beyond builds and existing regressions.

Fixed native forms now share a Begin helper with zero outer padding, disabled
mouse-wheel scrolling and a reset scroll offset. Legacy absolute control
positions already include their margins; adding ImGui padding previously caused
hidden scrolling by a few pixels despite NoScrollbar. Child lists/grids retain
normal scrolling. About keeps genuine content scrolling with no vertical outer
padding. Regression coverage reproduces the old hidden overflow, verifies the
fixed form has zero vertical scroll range, and retains child overflow scrolling.

#### 12.15 Settings

Settings now uses a fixed native panel with the original category icons,
bitmap controls, per-game executable paths, language/restart notice and Claw
version/CrazyHook controls. Native controls read local enabled/visibility flags
and retain edits in the existing settings widgets until Save. Executable
detection, file selection and Save run before NewFrame; native file dialogs
remain the Windows picker. Closing restores saved paths in both the native
controller and retained fields, preventing executable-change notifications
from reviving cancelled path edits. First-run game-specific opening/picking
continues to use the same public methods.

Path drafts convert UTF-8 through the Windows system code page with strict
validation, including systems configured with a UTF-8 ACP. Existing save/INI
and CrazyHook logic is reused, with dropdown bounds checks, initialized browse
button slots and a closed language-enumeration handle. Existing regression
tests cover system/UTF-8 path conversion alongside fixed-form scrolling.

Runtime gates: all categories, checkbox and dropdown save/reopen, restart-only
language changes, typed/browsed paths, Claw detection/CrazyHook gating, cancel
followed by executable notifications, first-run flow and detached input.
Settings and New Map also receive their queued-action Think calls on the
welcome screen, before the editor returns early for a missing document.

#### 12.16 Animation and Sound Browsers

The former combined database panel is replaced by two independent native
windows when ImGui is enabled. Both match the image/tile browsers: dark resource
list, source/file/hash/size metadata and fixed outer bounds with scrolling
confined to lists. Resource filtering and full-name hover text support long
asset lists. Animation rows show first-frame thumbnails and frame counts;
the detail area offers a hotspot-aligned preview, frame grid/keyframe tooltips,
border, mirror, loop, play/pause, speed and imageset override. Sprites are drawn
without changing shared HGE flip/color/hotspot state; preview origins come from
image metadata. The imageset resolver retains game/level/Gruntz heuristics with
guarded string operations.

Animation stepping uses actual durations, clamps nonpositive times to 1 ms,
fast-forwards complete loops and holds the final frame in non-looping playback.
Regression tests cover variable durations, end behavior, empty/zero-duration
animations, large time steps and missing imageset separators.

The sound browser owns its preview channel: row selection plays, with pause,
resume, stop, seek, elapsed/total time and volume. Closing/changing documents or
reloading the selected asset stops that channel. Selection resolves by resource
name and checks instance/hash/effect changes before using assets; missing or
unloaded data is guarded. Existing combined guichan UI remains for ImGui-off
builds. No new resource editing/import/delete operations are added.

Runtime gates remain: representative Claw/Gruntz animation resolution, stable
origins/mirroring, frame selection/playback/speed/loop, missing frames and file
refresh; audio play/pause/seek/volume/end behavior; independent windows,
document switches, filtering, long lists and detached input.

#### 12.17 Modeless hover and widget identity

Legacy pointer reconciliation accepts focus in any application viewport while
still requiring the main HWND under the pointer and respecting ImGui capture.
Legacy dragging retains main-window focus ownership; guichan modal event
filtering remains in force. The integration supplies global pointer positions
to ImGui while a detached window is focused because the SFML backend otherwise
polls only with main-window focus.

Skinned controls scope IDs by label (and radio choice); Settings additionally
scopes its icon-only browse button by widget identity. Regression coverage checks
distinct button/checkbox/radio IDs. Runtime gates: move between detached windows
and main menu/toolbars without clicking first; verify Object properties blocks
underlying controls and leaving the application clears hover.

Full ImGui keyboard navigation is enabled. Skinned checkbox/radio focus outlines
and button focus feedback follow visible keyboard navigation rather than mouse
focus. Input-driven regression covers mouse activation without an outline, Tab,
Shift+Tab, directional movement, Space/Enter activation and disabled-item skipping.
Go to coordinates submits Enter from its text inputs or Go button, avoiding a
second action when Enter activates a coordinate-mode radio button. Detached-window
keyboard routing and complete form traversal remain runtime verification gates.

Settings categories share the parent navigation scope and form one Tab stop.
Arrow focus selects a category immediately; Tab enters its form and Shift+Tab
returns to the selected row. Input-driven regression covers leaving the sidebar
from multiple categories and returning to it without trapping focus in the child.

#### 12.18 Remaining-window migration

The palette browser now uses a native ImGui window with a 256-color grid,
index/RGB tooltips and a scrollable palette list sharing keyboard navigation
with the form. Palette selection is queued until the legacy frame phase, where
the existing bank operation regenerates images/tiles and redraws the viewport.
Pending selection clears on document change; absent documents/banks close the
window. The guichan implementation remains available in ImGui-off builds.
Runtime gates: representative palette switching, detached rendering, document
switches and empty banks.

Duplicate and object-spacing forms now use native ImGui inputs and original
labels/buttons. Tool Open/Close and document changes own their lifecycle;
queued Apply revalidates the active tool/document/selection before forwarding
to the existing editor operation. Signed numeric parsing rejects incomplete or
out-of-range input; duplication validates multiplied offsets and final XYZ,
spacing validates its final coordinate. Regression covers offset arithmetic
boundaries. Spacing change detection now compares only the axis being moved.
Runtime gates: copies/IDs/rect updates, spacing on both axes, undo/save behavior,
selection/document changes, cancel and detached keyboard input.

Remaining scope includes camera,
plane tools and tile picker/tool options.
This inventory is a work list, not a completion claim; each entry requires
inspection of its actual actions, lifecycle and rendering before migration.

Object-brush settings now use a fixed native panel positioned at the viewport's
lower-left corner on open. Live controls retain rectangle-update behavior,
independent X/Y scatter, per-object scatter eligibility and the existing
10–700 px placement interval. Invalid/out-of-range or negative scatter drafts
are marked and do not replace the last valid tool setting. Closing/Escape stops
the tool; document changes discard drafts. Runtime gates: single/group brush,
independent/shared scatter, placement interval, rectangle updates, document
changes and detached-window input.

Object search now uses a native fixed window with term/query/case filters,
clipped result rows, sprite previews, metadata and Go to object/Select all.
Results are recomputed from the active plane; queued selection validates plane
and live object membership instead of resolving potentially duplicated IDs.
Preview UV mirroring leaves shared sprite state unchanged. Empty queries do
not match everything; exact ID parsing rejects overflow/trailing input without
exceptions, covered by matcher regression. Closing, mode/document changes cancel
pending selection. Runtime gates: all five search terms, codepage/case behavior,
duplicate IDs, mirrored previews, long results, edits/deletions, plane/document
switches, keyboard traversal and detached input.

The custom-logic browser now uses a native resource-list/details layout with
name/path/size/hash/modification time, full-value tooltips and original action
captions/icons. New/edit/rename/delete/browse actions run through existing editor
handlers before NewFrame; a busy guard suppresses this browser during nested
name/confirmation dialogs. Pending actions resolve selection by name against
the current bank, and document changes cancel pending work. Original script
creation, external editor and rename-reference prompts remain in use.
Runtime gates: file watcher refresh, creation/autoselection, edit/browse, rename
with referenced objects, delete/cancel, missing files, document changes and
detached keyboard/mouse input.

The first-run welcome uses a fixed native window with bounded, scrollable text,
legacy color/newline markup and original game-location/community actions. Its
queued actions run in the editor frame phase even without a document, and
closing/Escape cancels queued work. Existing game-directory picking, settings
opening and update-ready popup ordering remain in use; the retained guichan
welcome stays hidden in ImGui builds. Runtime gates: clean-profile startup,
all locales/short displays, choosing/canceling the game directory, community
link, close/Escape and an update becoming ready while the welcome is open.

Shared message/input dialogs now submit native ImGui modal popups from their
existing state, with queued return codes applied in Think. Original icon/button
sets and synchronous callers remain compatible. Input is UTF-8 in the widget,
validated back to the active legacy codepage/byte limit before acceptance;
Cancel/No/Skip remain available for invalid drafts. Escape follows the final
button, including Yes/No/Cancel. Text uses shared wrapped legacy markup, with
regression for newline height and green/white vertex colors; a real ImGui-frame
test verifies modal capture and blocked clicks on background controls. ShowDialog leaves
Init to the state manager, removing the former double initialization/leak.
Runtime gates: all button sets and returned codes, Enter/Space/Tab/Escape,
non-ASCII input/cancel/limits, short displays and long messages, modal exclusion
across detached viewports, repeated/nested synchronous dialogs and shutdown.

Mapshot migration is in progress. The configuration form is native (path picker,
plane, scale/dimensions, objects, background, compression and directory option),
with queued browse/save and overwrite confirmation. The existing menu disable
remains until the export progress/rendering state is migrated and verified;
this form alone does not establish export readiness. The legacy destination
probe no longer creates/deletes files, including during document switches.
Regression verifies existing file contents survive path validation and rejects
zero-pixel scaled dimensions. Saving now uses the full destination path without
opening/truncating a second file, and directory reveal uses a quoted Explorer
argument rather than a shell command. Remaining export work includes progress
UI, forced-render integration, compression handling and end-to-end file checks.

Export progress now uses native ImGui labels, bars, preview and cancellation.
Generation runs as Allocate/Draw-one-chunk/Save phases in normal Think/Render,
with an initial display frame and no forced/nested render loop. Each chunk
restores camera, zoom, viewport, background and plane visibility through a scoped
guard. Readback copies only actual edge dimensions, and preview textures remain
owned by their targets. The export render path uses the chosen background rather
than editor checkerboard. Both native and retained progress widgets are released.
Regression exercises real render-target readback, partial chunk edges and PNG
save/load at a full path. Remaining readiness gates include PNG/JPEG compression
handling, parallax plane coordinates and end-to-end exports on representative
maps; the menu remains disabled until those are addressed.

PNG/JPEG export encoding now uses a private vendored stb_image_write v1.16
implementation with the existing zlib dependency. PNG levels 0–9/default and
JPEG quality are applied; compression UI mapping no longer produces negative
JPEG quality. The writer supports filesystem/Unicode paths and commits a fully
written sibling temporary file, preserving the destination on failure. Regression
compares encoded sizes, PNG RGBA/Unicode roundtrips, JPEG dimensions, locked-file
failure preservation and temporary-file cleanup.

Each rendered chunk temporarily uses 100% plane movement, no higher-plane fade,
grid, boundary or tile-attribute overlay, and bypasses the editor render cache.
Object coordinates use the exported plane. The scoped guard restores those
properties along with the editor viewport/camera. The export menu is enabled
again. Export temporarily overrides render bounds without reallocating or moving
the editor viewport target/widget; restoring geometry does not allocate memory.
Runtime gates remain: representative Claw/Gruntz maps, parallax/zero-speed
planes, objects, multi-chunk seam checks, cancellation/resize and verifying
unchanged editor/document state after success/failure.

The object-area bounds picker now uses a native fixed panel with live X1/Y1/X2/Y2,
four edge-pick modes, rectangle drag selection and Accept. Commands are queued
before NewFrame and use the existing map-selection/return-code handlers. Closing
returns cancellation to Object properties when invoked from it; standalone use
stops the tool without flipping an unrelated state. Escape cancels a pending pick
first and closes on the next press. Tool/document lifecycle clears pending work,
and the retained guichan panel stays hidden with ImGui enabled.
Runtime gates: each edge, reverse-direction rectangle drags, map/panel focus,
keyboard traversal, Escape/close, negative coordinates and Object properties
accept/cancel return behavior.

Object properties now submits a native modal form and a native imageset/frame
picker. The original property model supplies control positions, captions,
visibility/enabled state and save/cancel/logic/rectangle operations; each visible
field, flag, radio, dropdown and button is rendered as an ImGui control rather
than a bitmap-wrapped legacy window. UTF-8 drafts validate numeric syntax/range
and legacy text bytes before updating the retained draft model. The native
imageset list/frame grid supports automatic I=-1, explicit image IDs, empty banks
and selection cancellation. Commands that can open dialogs run before NewFrame
with a busy guard. Returning from rectangle picking invalidates only its four
coordinate drafts; other edited fields are retained. ImGui-off builds retain the
original guichan rendering and input.
Nested confirmations render a disabled backdrop of the property form rather
than hiding it; it has a separate window ID and cannot submit actions. The
interactive modal reopens with its retained field drafts after the dialog.

Runtime gates remain: every property/flag/rectangle, Save/Cancel and new-object
cancel semantics, standard/custom logic transitions and external script editor,
text/codepage limits, missing assets, picker/frame IDs, repeated nested dialogs,
map rectangle pick return, main-window modal exclusion, keyboard traversal and
small displays. Existing shared widget/font/modal tests are relevant building
blocks, not proof of complete Object properties behavior.

The fill-color tool panel now uses native palette cells with hover/keyboard
preview and current-index feedback. Its expanded/collapsed sizes preserve the
bottom edge, and it follows the existing tile-fill sentinel/tool eligibility.
Queued color changes validate document and active-plane identity, redraw the
viewport and mark the map unsaved; the legacy palette changed the header without
marking it dirty. Tool/document changes cancel pending selection, and the legacy
auto-show paths are disabled in ImGui builds. Escape collapses the palette first,
then exits the tool. Runtime gates: fill-tile selection, palette keyboard/mouse
interaction, plane/document switches, palette switching and saved fill color.

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

### Current window migration audit (2026-10-15)

The native tile/brush picker now covers tile selection, attribute previews and
retained tool/Lua brush settings. Tool transitions pass the destination tool to
CloseTool and validate brush indices. Pointer capture uses event coordinates and
current native window bounds before NewFrame, preventing first-click leakage
through modeless panels while retaining drag ownership. Both migration test
executables pass, and x64/Win32 builds produce WapMap.exe.

The window rewrite is not complete. Remaining legacy UI includes specialized
object editors under WapMap/objEdit (enemy, elevator/path, crate/treasure and
inventory, warp, ambient/sound trigger, and other object-specific forms), image
import and image-details states, and the map-loading progress window. Audit
reachability before removing dormant camera/statistics UI. Main editor chrome
also still uses guichan and is a separate remaining step if the scope includes
removing guichan entirely. Keep retained model/fallback code until replacement
behavior is verified. Runtime checks still required: actual dialogs, nested
modals, keyboard navigation, tool/document transitions and export on real maps.

### Native image details and initial specialized object editors

Image details now uses an ImGui modal with a clipped checkerboard preview,
independent image scrollbars, keyboard-accessible zoom, Cancel/X/Escape, and
queued state removal in Think. It submits the source atlas through ImGui without
mutating shared sprite state. Negative zoom steps now decrease monotonically
from 100% to 1/6 instead of the old inverted scale sequence. Legacy controls are
not constructed in the ImGui path.

The specialized object-editor base now supports modeless native windows,
position persistence, shared Save/Add next controls, queued legacy model actions,
and reentry suppression. Health/ammo/powerup and curse editors use this path,
with animated previews, existing radio selection models, hover/keyboard feedback
and effect descriptions. Other specialized editors remain on their old UI until
their custom controls/previews are ported. Runtime gates: open each editor,
select by mouse/keyboard, move the modeless window, drag the edited object on the
map, Cancel/X/Escape, Save and Add next, and missing/empty image sets. Image
details still needs runtime checks with small/large images and zoom/scroll.

### Additional specialized editors: numeric forms and pickup previews

The native object-editor base now renders direct labels, integer fields,
checkboxes and radios using their retained positions/models. Invalid/overflowing
integer drafts remain editable and block Save/Add next; disabled drafts do not
block saving. Numerical fields retain their negative-value policy. Valid edits
queue existing model actions before the next NewFrame. Shared pickup previews
handle missing/empty sprite sets and do not mutate shared sprites.

New native editors: breakable planks, special powerups (duration and sword
persistence), checkpoints/super checkpoints/boss stager, crumbling pegs (including
the level-three image variants), and treasures (including glitter). Special
powerups use local enabled state when saving because their retained root is
hidden, and their editor type is now enSpecialPowerup. Checkpoint destruction
releases all four radios; treasure destruction releases the glitter checkbox.
Runtime gates remain: each selection, live map previews, cancel/save/add next,
conditional controls, invalid numeric drafts, keyboard traversal and missing
image assets. These changes do not port the remaining inventory/rectangle-pick,
path, sound-player or other custom object-editor widgets.
Native object-editor sizing also includes the title bar and footer height, preserving the bottom anchor when expanding an undersized legacy form. Final x64 and Win32 builds pass; both existing migration regression tests pass. Full GUI flows remain unverified.

### Native area-picking object editors

The shared native object form now renders nested cProcPickRect controls and
additional icon buttons, preserving button highlights and per-field model
validation. Queued rectangle events go through the picker listener before the
owning editor. Starting/stopping Pick discards superseded numeric drafts. Native
window sizing includes nested controls, keeping Pick buttons inside the form.

New native forms: projectile shooters, lasers, lava hands, screen shake and
stalactites. Rectangle edits and map picking update their retained temporary
object models; Save/Add next remain blocked during picking. Shake/stalactite Save
availability now follows typed coordinate validity. The shared picker processes
mouse release before input-consumption checks, and dragging recalculates both
edges from the fixed start point when crossing the origin. IsEnabled uses local
button state with hidden retained roots. Projectile dragging is initialized,
normalization of INT_MIN speed avoids signed overflow and updates the selected
direction, and its four coordinate fields are released on destruction.

New geometry regressions cover reverse/cross-origin dragging, optional empty
rectangles/axes, reversed endpoints and overflowing/negative coordinates. These
are model checks; runtime gates still include Pick/Cancel/Escape, release over a
panel, direct coordinate correction, direction buttons, conditional level-14
shooter directions, Save/Add next, and switching documents/tools. Remaining
specialized editors with inventories, audio, paths and custom controls still
need their native implementations.
Validation: final x64 and Win32 builds pass; both regression executables pass with the new geometry checks. The legacy simulatePress/distributeActionEvent path does not gate callbacks on hidden parent visibility; full GUI behavior remains a runtime gate.

### Warp, springboard and timed-series editors

Warp and springboard editors now use native controls. Warp previews render via
ImGui with missing/empty-set guards. Destination picking moved from legacy Draw
to _Think so it works without the retained viewport callback; all three variant
radios are disabled during picking. Explicit Pick clears superseded integer
drafts. Springboard height picking checks the coordinate difference in 64 bits.

Toggle-peg and floor-spike/saw forms now include native image choices and their
separate series windows. Save remains in the main form; Add next is rendered only
in its owning series panel. Both forms share draft validity for Add next, and
its delay-plus-increment must fit an int. Invalid increments do not prevent saving
the existing object in the main panel. The companion window participates in
keyboard focus/Escape, has no close button like the original, and sizes to fit
its footer. Add-next application calculates the sum in 64 bits, avoiding signed
integer overflow. Missing assets fall back to the placeholder preview.

Runtime gates: each warp variant/destination/cancel, springboard height picking,
all level-dependent peg/spike/saw variants, duration/default encodings, Always on,
series focus/Tab/Escape, overflow feedback, and Save/Add next across successive
objects. Regression tests pass, including signed-boundary series increments;
compilation and code inspection do not prove those full GUI flows.
Series audit also found that saved SawBlade2/3/4 presets were not decoded on reopening (only FloorSaw aliases were recognized), resetting the base delay during Add next. Shared TimedSeries::Delay now decodes the saved engine names and retains old aliases; regression checks cover saw, spike and peg presets plus explicit delays.
Validation: final x64/Win32 builds pass after the preset-delay correction, both regression tests pass, and git diff --check reports no whitespace errors. Full GUI interactions remain unverified.

### Native inventory catalogue, slots, crab nests and crates

The inventory picker has an opt-in native path; remaining legacy editors keep
using their original catalogue until migrated. The native catalogue has a clipped
scrolling item grid, animated source-atlas thumbnails, effect/ID descriptions,
unsupported-warp tinting and mouse/keyboard selection. Native slots support
catalogue-to-slot and slot-to-slot drag/drop, removal by dragging out, right-click
or Delete, and keyboard pick/place. Payloads carry item IDs, never widget pointers;
model assignments validate slot membership, local visibility/enabled state and
run through the existing owner callback in Think. Drag previews use ImGui so they
travel with detached viewports. Native dragging does not depend on the legacy
clipboard or its HGE mouse state. Escape clears a carried selection/drag first.

Crab nests and crates now opt into the native catalogue and slots. Crab-count
validation enforces 0..9 and Save follows rectangle validity/picking state. Crates
also render durability dropdowns, stacking previews, alignment/custom Z,
randomization and conditional warp destinations. Their retained hidden-root
checks use local enabled/visible state, so stacking and durability decisions are
not corrupted by hiding the old window. Warp picking runs in Think in the native
path, Escape cancels picking first, and single-container warp picking guards null
inventory slots. Destruction releases the custom Z field.

A real ImGui frame regression verifies delivery when the source widget disappears
after taking its item (needed for crate compaction); both regression executables
pass. Runtime gates: catalogue scrolling/descriptions, mouse and keyboard pick/
place, dropping outside, delete/cancel, all stack sizes and compaction, crate
randomization/durability/warp options, crab count/activation area, Save/Add next,
missing assets, detached viewports, and tall crate forms on small screens.
Final validation: x64 and Win32 builds pass with the native crate path, both regression tests pass, and git diff --check reports no whitespace errors. Companion inventory/series windows do not steal focus on appearing; dropdown frames retain the original 20-pixel control height. Full application GUI flows remain unverified.

### Native enemies and statues; bounded tall forms

Enemy and statue editors now use native controls and the shared native inventory
catalogue/slots. Enemy type/behavior/patrol controls, optional damage, patrol-area
picking and warp/gem destinations preserve their retained model routes. Hidden
retained roots no longer erase visible loot or hide saved fields: model decisions
use local visibility/enabled state. Save only promotes a visible warp item to the
first slot. Enemy destruction releases its nine slots and treasure label; statue
destruction also releases randomization controls and the custom-Z field.

Enemy refresh no longer forces an optional patrol checkbox back on. Destination
validation uses map height for Y, refreshes after picking/typing and ignores hidden
destination fields. Save is blocked during either pick mode or an invalid enabled
patrol area; object dragging is disabled while picking. Clear/Pick discard numeric
drafts that have been superseded by model operations.

Tall enemy forms cap their root to the available main viewport height, keep Save/
Add next outside a scrolling body and flatten child navigation so Tab can leave
it. Generic bitmap labels now use cursor-screen positions so they scroll with
other controls. Existing short forms retain their fixed, non-scrolling roots.
A real ImGui test verifies automatic child scrolling, Tab reaching the fixed
footer and zero root ghost-scroll range. Destination geometry tests cover wide/
tall maps and zero/far edges. Both regression executables pass.

Runtime gates remain: each enemy variant and forced/optional patrol policies,
behavior flags, damage, conditional loot and warp/gem controls, both pick modes,
patrol-area clear/edit/cancel, short/tall layouts and detached views, statue
randomization/durability/Z, and Save/Add next across all supported game levels.
Final validation: x64 and Win32 builds pass after the layout refinements; both regression executables pass. Enemy body IDs remain stable across bounded/unbounded layout changes. Inventory keyboard carry clears after placement or starting a drag, so a stale selection does not intercept Escape during map picking. Full application GUI flows remain unverified.

### Native ambient-sound and sound/dialog-trigger editors

The native object form now supports editable text dropdowns with legacy/UTF-8
conversion, independent IDs for field/arrow/popup, model choices and queued owner
callbacks. Ambient sound and sound/dialog triggers opt into this path and use a
native embedded cSoundPlayer card: play/pause, stop, seek, volume and elapsed/
total time. It resolves assets by copied name on each native draw, stops playback
when the asset/effect changes, guards missing/unloaded assets, and retains the
name while a temporarily missing asset can return. cSoundPlayer destruction now
stops its owned channel and releases both sliders; Stop resets the cursor.

Ambient forms preserve global/specified/two-area modes and loop/interval policy.
Only enabled areas and interval fields gate saving. Interval endpoints are
validated as non-negative and increasing; map-pick release is handled before
consumed-pointer checks, and picking suppresses object dragging. An empty sound
bank no longer indexes element zero. Hidden-root checks use local enabled state.
Sound/dialog triggers preserve standard/custom sound selection, activation-area
size, exclamation/dialog behavior and activation counts; selecting custom mode
updates the actual animation/sound binding. Missing custom resources block Save,
and a newly available resource updates the preview and temporary object together.
The activation-area dropdown is now released on destruction.

Both regression executables pass, including new interval-bound checks. These are
not proof of audible playback or complete editor behavior. Runtime gates include
play/pause/resume/stop/seek/volume, close/switch-document during preview, file
remove/reload, custom dropdown Unicode/invalid IDs, both ambient areas, loop and
on/off intervals, all trigger modes/counts/area sizes, and Save/Add next.
Final validation: x64 and Win32 builds pass after the missing-resource recovery correction; both regression executables pass; git diff --check reports no whitespace errors. Audible playback and full GUI flows remain unverified.

### Native map-text and rope editors

Map text now uses a dynamic std::string-backed ImGui multiline input, preserving
legacy encoding conversion without imposing a fixed text-buffer limit. Enter
inserts a new line; Apply/alignment/Save reuse queued owner callbacks and local
enabled state. Alignment buttons expose their selected state. Empty or
unsupported-only text, invalid encoding and overflowing layouts cannot be saved.

A shared tested layout replaces the separate generation/alignment scans. The
first supported glyph always reuses the temporary object, fixing phantom/stale
objects for leading spaces/newlines/unsupported bytes. Left/center/right alignment
uses the same measured line widths, punctuation has a consistent baseline, and
layout/coordinate bounds are checked before deleting the previous preview.
Re-alignment preserves existing object IDs. Group dragging synchronizes after the
base movement (including release/cancel), updates spatial indices and rejects
overflowing group moves. The shared save dispatcher now respects disabled Save/
Add next and a virtual Save operation that discovers invalid data.

The rope editor also opts into native controls. It uses enRope and positions both
footer buttons in compact and animated modes, avoiding an off-window Add next
control forcing the compact form to expand.

Both regressions pass: glyph mapping, leading whitespace, line alignment,
punctuation, unsupported-only input and coordinate overflow, plus a real ImGui
multiline test typing 2000 characters and Enter without truncation/submission.
Runtime gates remain: real GAME_FONT assets, Apply/Save/cancel and selection of all
created glyphs, alignment before/after edits, group drag/release/Escape with Ctrl/
Shift, missing font frames, non-ASCII input, rope mode/speed/delay and Add next.
Final validation: x64/Win32 builds pass with both native editors and save-dispatch guards. Both regression tests pass; the multiline fixture includes a navigable Save control and verifies Enter does not activate it. Full application GUI and real GAME_FONT flows remain unverified.

### Native wall-cannon and Candy editors

Wall cannon now uses native controls with validated component/polar conversion,
overflow-safe map dragging and direction changes, and cancellable arrow picking.
Regression coverage includes both facings, angle normalization and extreme values.
Candy now uses native category tabs and a clipped, scrollable image grid with
checkerboard previews. Selection resolves current assets by name and frame ID;
empty sets are guarded, and switching to animation clears a stale frame ID.
Regression coverage checks animated, single-frame and explicit-frame selection.

Both regression executables pass. Application GUI flows remain unverified:
cannon direction/arrow picking/Save and Candy tabs, animation expansion, asset
reload, Save/Add next and keyboard navigation need testing with real maps.
Remaining specialized ports are Elevator and Elevator Path; image import,
loading progress and legacy main chrome also require completion or scope audit.

### Native Elevator and Elevator Path editors

Both elevator forms now use native object-editor windows. The shared renderer
supports segmented type selectors, nested lockable X/Y controls, and lists with
a background and a single Tab stop while retaining arrow-key selection.
Compound X/Y actions dispatch through their original owner before updating the
object. Selector tooltips, active direction buttons, manual rectangle picking,
movement/speed fields and path-step controls retain their model callbacks.

Lockable fields now accept zero, parse complete integers without throwing, and
reject non-finite/overflowing proportion calculations. Programmatic setters
respect their event-suppression argument. Elevator distance/direction changes
check endpoint bounds; automatic object moves validate the complete translated
range before applying it. Manual picking handles release over consumed UI and
blocks object dragging/Save while picking.

Path editing fixes stale controls after deleting a step, initializes the unused
label pointer, guards absent preview sprites and restores the original step on
Escape/right-click. Releasing over UI keeps the last valid drag. Declining the
open-route Save confirmation now blocks the shared save dispatcher. Route
closure builds a proposal before asking about removed steps, clamps selection
after shortening the list and respects eight-step storage. Intermediate route
coordinates and drag distances are validated before updating encoded rectangles.

x64 and Win32 builds pass; both migration regression executables pass. New tests
cover X/Y scaling, endpoint overflow, route closure/storage limits, invalid
intermediate points and drag snapping. Runtime gates remain: all elevator types,
one-way/automatic/manual transitions, locks/directions/speed warnings, rectangle
picking, path list keyboard navigation, move/wait edits, close/invert/generate
chain, Save/Add next/cancel and document transitions on real maps.
The remaining window ports include image import and map-loading progress;
dormant camera/statistics reachability and legacy main chrome still need audit.
Full application GUI behavior has not been interactively verified.

### Native image import and map-loading window

Image import now uses a modal ImGui state with a native multi-file picker,
queued OS drops, independent preview textures, a clipped file list, editable
IDs, batch ID generation and destination/result names. All IO and state changes
run from Think. Cancel/title-close/Escape close the popup before popping the
state; successful import also closes the modal explicitly. Browser import
buttons are enabled in the native path. Existing custom image sets accept new
frames; standard image sets retain their disabled Add frames button because
custom packages use a different mount prefix. Tile sets accept additions and
original-tile overrides. Unsaved maps must be saved before custom-file output.

The old import implementation had disabled file-selection/drop/save paths and
lost decoded image dimensions. The native replacement reads PID and paletted
PCX with bounded decoders, decodes truecolor formats through SFML, quantizes to
the current palette, preserves PID offsets/light data, and validates tile sizes.
It validates complete IDs, all duplicate rows, existing custom IDs, safe relative
destination folders and current bank membership. A staged PID batch refuses
existing output files and rolls back its committed files on failure. Feed file
notifications remain responsible for registering/reloading imported assets.

Map loading now draws a native fixed progress window with indeterminate parsing
progress. Its worker no longer changes GUI controls, pushes dialog states or
spins on an unsynchronized render flag. Think waits for the first main-thread
render, consumes the future once and displays loading errors on the main thread;
Destroy joins a pending task before releasing its input buffer/path. Resource
mounting remains synchronous and has no measured incremental progress yet.

Regression tests pass for filename/ID/path validation, palette conversion,
truncated PID/PCX and padded PCX rows, staged output/no-overwrite/rollback, and
new output read by the original application PID decoder. GUI flows, external
file dialogs, all truecolor formats, hot reload after import, failed writes and
real-map loading/errors still require runtime verification.

Reachability audit: MapStats is reachable from APPMEN_TOOLS_STATS and remains a
required native port. The old Camera window has no opening action in current
sources; legacy database and selection-area entry points are replaced in the
ImGui build. Main editor chrome remains on guichan. The migration is incomplete.
Final validation: x64 and Win32 builds pass after the modal-close and saved-map
guards; both regression executables pass. No interactive GUI verification was
performed. Review build: bin/x64/ImportReview/WapMap.exe.

### Native map statistics

MapStats now takes a value snapshot and displays a native modal with General,
Objects and per-plane tabs, responsive legend tables and ImGui-drawn pie charts.
It does not allocate legacy GUI/chart textures in the native path; its snapshot
is released in Destroy. Close/title-close/Escape close the popup and queue a
state pop. A child body handles genuine overflow without root-window ghost scroll.
Unique plane tab IDs allow duplicate plane names; empty distributions avoid
division by zero and a single category draws a complete circle.

The old tile classification treated ordinary tiles as empty, left the unknown
counter uninitialized and counted repeated double-tile attributes twice. Native
statistics distinguish invisible/filled/normal tiles, classify missing definitions
as unknown and count each distinct attribute once per tile. Mixed tiles can
contribute to multiple categories; the UI explains the percentage denominator.
Attribute summaries are cached per tile ID, avoiding repeated full mask scans.
Object/treasure counts and score arithmetic use int64. Treasure counts retain
the image-set-name method and explicitly exclude container/enemy loot; the
placed-treasure score is displayed only for Claw/Claw2.

Both regression executables pass, including tile classification, duplicate
attribute membership, treasure matching and scores beyond 32-bit range. x64
and Win32 builds pass. Runtime gates remain: real-map totals, empty maps,
all treasure categories, mixed/missing attributes, many/duplicate plane tabs,
chart layout at smaller window sizes, keyboard navigation and modal close.
No interactive GUI verification has been performed.

Current source audit finds native entry paths for the reachable auxiliary
windows and specialized object editors. The dormant Camera form remains as
legacy model/fallback code; main editor chrome and context menus still use
guichan. Do not treat retained source constructors as proof that a native
entry path is missing, or the source audit as proof of runtime completion.

### Native application menu

The application menu now uses an ImGui menu bar and native submenu navigation
while retaining the existing Context command models and action callbacks.
Read-only model access exposes ordered entries/separators. Icons, checked states,
enabled groups, recent files and dynamic plane options are rendered natively.
Plane submenus have an explicit visibility toggle alongside border/grid/object
options. App-menu guichan painting and mouse/key opening paths are bypassed in
the native build; the model objects remain for synchronization and dispatch.

Selections are queued and dispatched from EditingWW::Think before NewFrame.
Dispatch revalidates the document, enabled/reachable model entry, caption, recent
file path and plane selection. A busy guard prevents reentrant actions during
synchronous dialogs, and processing a command ends the editor's Think for that
frame. Legacy contexts requested by old callbacks are hidden after dispatch.
Alt/F10 focus, arrows/submenus and Escape use native navigation; menu closure
releases keyboard capture back to the editor.

Main-menu rendering is submitted before state rendering so it remains in the
background under a modal, with its input disabled when EditingWW is not on top.
Modeless native panels retain normal main-menu interaction. Startup fade gates
the new bar just as the application chrome becomes available.

Runtime gates remain: every menu command and enabled/checked state, file/MRU
changes, repeated plane submenu selection, Alt/F10/arrows/Escape, returning
keyboard input to the viewport, modeless hover/clicks, modal overlays and both
maximized/fullscreen modes. Existing regression tests pass but do not verify
these full application GUI paths. Toolbar, document tabs, status strip and
standalone context menus remain to migrate.
Final validation: x64 and Win32 builds pass, including disabled background menu
rendering during synchronous modal actions; both existing regression executables
pass. No interactive application-menu verification was performed. Review build:
bin/x64/MenuReview/WapMap.exe.


### Current migration audit (2026-10-24)

This audit supersedes earlier remaining-work statements where they conflict with
current source. The SFML backend is implemented and the main build definitions
no longer link HGE, BASS or DirectX 9. HGE-named helpers and the guichan adapter
run through the SFML facade; removing that facade is optional Step 13.

The ImGui implementation now extends beyond the application menu. The connected
`nativeChrome.cpp` path draws the tile/object toolbar, status strip, zoom controls,
map dockspace and tile/object context menus. `cMDI::DrawImGuiViews` supplies map
document windows and their docking/detaching behavior. Auxiliary windows and
specialized object editors have native entry paths. Retained guichan constructors
and action models are not evidence of missing native UI. The dormant Camera form
and ImGui-off fallback remain legacy code. Earlier notes saying toolbar, document
tabs, status strip and standalone context menus still await migration are stale.
Source integration does not establish complete interactive parity.

Dependency source: the SFML 3 multi-viewport backend is published as `8851118` on
`wapmap` in https://github.com/Zax37/imgui-sfml. The matching ImGui source is
`778992face67ecefaa5c2932db4b2101545ea4df` in the sibling checkout. README records
these revisions; the build does not automatically fetch or enforce them. CMake
still defaults ImGui to OFF while Visual Studio enables it in all configurations.
Validate these as separate configurations.

Validation in this audit: rebuilt the Release x64 standalone migration suite and
ran CTest successfully (2/2: migration_regression, bitmap_font_regression). No full
application rebuild, interactive GUI pass, audible playback check or performance
benchmark was performed in this audit.

Remaining completion gates:
- Run the outstanding Step 0/10 application checklist and reference-image
  comparisons, including save/reopen after tile/object edits, specialized forms,
  import and mapshot output, sound/ADPCM and simulator playback, non-ASCII text,
  hotkeys, drag-and-drop and resize/minimize/restore.
- Exercise native menus/toolbars/context menus, map docking and detached-window
  input/focus, modal interactions, keyboard navigation and multi-monitor/DPI.
- Resolve or explicitly accept BLEND_COLORADD's approximation: graphics.cpp uses
  framebuffer BlendAdd rather than HGE texture-stage colour addition. A legacy
  wComboButton use remains; verify the ImGui-off appearance against the baseline.
- Measure LEVEL13 cold-load time and zoomed-out panning/frame times after the
  atlas and lookup optimizations; source changes alone do not prove performance.
- Verify clean dependency setup and both ImGui-on/off builds. Audit remaining
  x64 narrowing warnings and third-party Lua brush behavior as needed.

The remaining work is primarily validation, exact blend parity and build
reproducibility. Removing the compatibility API is not a completion requirement
for the HGE-to-SFML backend migration.
