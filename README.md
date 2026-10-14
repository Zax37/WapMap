![WapMap](logo.png)

# WapMap

WapMap is a WAP32 file editor, originally developed by kijanek6 in 2010–2013. It is primarily known as a Claw level editor and also supports Gruntz and Get Medieval maps to some extent.

The renderer and platform layer are being migrated from HGE to SFML 3. The application retains its existing editor UI and uses the in-tree HGE compatibility facade; building WapMap no longer requires the HGE library or the DirectX SDK.

## Building on Windows

The default build target is x86. An optional x64 target is also available when all dependencies are built for x64. Install Visual Studio 2019 or newer with the C++ desktop workload and CMake 3.22 or newer.

### Dependencies

- SFML 3.0.2, built and installed for x86. The default install prefix is `../SFML-3.0.2-install`; set `SFML_INSTALL_DIR` to override it.
- zlib for x86. It can be installed with vcpkg (`vcpkg install zlib:x86-windows-static-md`) and discovered through the vcpkg CMake toolchain, or supplied through CMake's `ZLIB_ROOT`.
- curl, built as a static x86 library with Schannel, in `../curl`. WapMap expects its `include` directory and `build/lib/{Debug,Release}` outputs.
- Lua, built for x86, in `../lua`. WapMap expects its headers there and `Debug`/`Release` library outputs.

For x64, use x64 builds of SFML, zlib, curl and Lua. CMake keeps Lua headers (`LUA_DIR`) separate from its library output (`LUA_LIBRARY_DIR`), and curl's source/header root (`CURL_DIR`) separate from its build output (`CURL_BUILD_DIR`). The Visual Studio project defaults to sibling x64 dependency folders (`SFML-3.0.2-x64-install`, `zlib-x64\install`, `curl-x64-install`, and `lua-x64`); override its `SFMLRoot`, `ZlibRoot`, `CurlRoot`, or `LuaRoot` properties if needed. HGE and the DirectX 9 SDK are not required.

### Configure and build

Run these commands from an x86 Visual Studio Developer Command Prompt, with the vcpkg toolchain path adjusted to your installation if using vcpkg:

```powershell
cmake -S . -B ..\WapMap-sfml3-build -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE=D:\Projects\vcpkg\scripts\buildsystems\vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x86-windows-static-md
cmake --build ..\WapMap-sfml3-build --parallel 8
```

If zlib is installed outside vcpkg, omit `CMAKE_TOOLCHAIN_FILE` and set `ZLIB_ROOT` to its x86 installation prefix.

For an x64 build, configure a separate build directory and point CMake to x64 dependency outputs:

```powershell
cmake -S . -B ..\WapMap-sfml3-x64-build -A x64 `
  -DSFML_INSTALL_DIR=D:\Projects\SFML-3.0.2-x64-install `
  -DLUA_DIR=D:\Projects\lua `
  -DLUA_LIBRARY_DIR=D:\Projects\lua-x64 `
  -DCURL_DIR=D:\Projects\curl `
  -DCURL_BUILD_DIR=D:\Projects\curl-x64-build `
  -DZLIB_ROOT=D:\Projects\zlib-x64\install
cmake --build ..\WapMap-sfml3-x64-build --config Release --parallel 8
```

### Building the Visual Studio project directly

`WapMap\WapMap.vcxproj` defaults `ZlibRoot` to `..\..\zlib` relative to the project directory. To use a vcpkg installation instead, set the MSBuild property to its x86 triplet prefix, for example:

```powershell
msbuild WapMap\WapMap.vcxproj /p:Configuration=Release /p:Platform=Win32 `
  /p:ZlibRoot=D:\Projects\vcpkg\installed\x86-windows-static-md
```

The selected prefix must contain `include\zlib.h`, `lib\zlib.lib`, and `debug\lib\zlibd.lib` for Debug builds. Set `ZlibRoot` to another x86 zlib prefix if your installation uses a different location.

The solution also provides `x64` configurations. Build them with `/p:Platform=x64`; the project uses separate x64 dependency defaults and writes outputs to `bin\x64` and `obj\x64`.
