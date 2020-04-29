# WapMap

## Info

WapMap is a fanmade WAP32 file editor, originally developped by kijanek6 in 2010-2013.
It's mostly known as Claw level editor, but it used to support Gruntz and Get Medieval maps too at some point(and to some extent).

For the upcoming, 10th birthday of the editor, I decided to bring it back, and finally managed to build the thing myself.

WapMap, as most of kijanek's projects at the time, depended on HGE(Haaf's Game Engine), which he edited when needed.
Sadly, in the package, which I got from him, there was no source files of that version, only linking files.

He planned on rewriting the rendering to SFML, but he failed to get it working and abandoned the project.

Since I don't have source code for some of the changes in HGE, which kijanek made, I've cloned the HGE github repository,
merged with the header files from his version and re-implemented most of the needed new functionalities.

## Building

1. Get Visual Studio 2017 (v141) toolset to build HGE and 2019 (v142) for WapMap.

2. Create some directory for the project and its dependencies (e.g. WapMap).

3. Clone them:

```
git clone https://github.com/Zax37/WapMap
git clone https://github.com/Zax37/hge
git clone https://github.com/curl/curl
git clone https://github.com/lua/lua
```

4. Build the dependencies (I used CMake). You'll need to download DirectX 9 SDK (June 2010), as HGE's readme says.

5. Download [SFML 2.5.1](https://www.sfml-dev.org/files/SFML-2.5.1-windows-vc15-32-bit.zip), unzip it next to other dependencies and rename from `SFML-2.5.1` to `SFML-2.5.1-2017`.

6. Open WapMap/WapMap/WapMap.sln in Visual Studio 2019 and run it!

7. I've also created CMakeLists for the project, if you'd like to use different toolset. I'll try to keep them up to date. :)