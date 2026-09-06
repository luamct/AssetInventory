# AssetVault

A fast desktop browser for game asset libraries. Point it at a folder full of textures, models, sounds and fonts, and get a searchable, thumbnailed grid with live previews — no importing, no project files, no cloud.

Built in C++17 with Dear ImGui, OpenGL and SQLite. Runs on Windows and macOS.

## Features

- **Instant search** across your whole library, with filters by asset type and folder path
- **Thumbnails for everything visual** — PNG, JPG, TGA, HDR, EXR, DDS, KTX, SVG and more, generated once and cached
- **3D preview** for FBX, OBJ, glTF/GLB, DAE, PLY, STL and 3DS, with multi-material support and animation playback
- **Audio playback** for WAV, MP3, OGG, FLAC, AAC and M4A straight from the grid
- **Folder tree** alongside the results, so you can browse by structure or by search
- **Drag and drop out** — drag an asset from AssetVault into your engine, editor or file manager
- **Live updates** — a file watcher keeps the index in sync as you add, rename or delete files; no rescans
- **Incremental indexing** — only changes since the last run are processed, so startup on a large library is fast

## Download

Prebuilt binaries for each release are on the [Releases](https://github.com/luamct/AssetVault/releases) page:

- **Windows:** `AssetVault-windows.zip` — extract and run `AssetVault.exe`
- **macOS:** `AssetVault-macos.zip` — extract and drag `AssetVault.app` to Applications

> **First-launch warnings.** AssetVault is a small project and its binaries don't yet have a download history with Windows SmartScreen or macOS Gatekeeper. On Windows you may see *"Windows protected your PC"* — click **More info → Run anyway**. On macOS, right-click the app and choose **Open** the first time. Only do this for binaries downloaded from this repository's Releases page.

## Getting started

1. Launch AssetVault.
2. When prompted, choose the root folder of your asset library. You can change it later from the search panel.
3. AssetVault indexes the folder and generates thumbnails in the background — the progress bar shows how far along it is.
4. Search, filter by type, click an asset to preview it, or drag it into another application.

The index, thumbnails and settings live outside your asset folder, in `%LOCALAPPDATA%\AssetVault` on Windows and `~/Library/Application Support/AssetVault` on macOS. Your asset files are never modified.

## Supported formats

| Category | Extensions |
|---|---|
| 2D / textures | png, jpg, jpeg, gif, bmp, tga, dds, hdr, exr, ktx, svg |
| 3D models | fbx, obj, dae, gltf, glb, ply, stl, 3ds |
| Audio | wav, mp3, ogg, flac, aac, m4a |
| Fonts | ttf, otf, woff, woff2, eot |
| Shaders | vert, frag, geom, tesc, tese, comp, glsl, hlsl |
| Documents | txt, md, pdf, doc, docx |
| Archives | zip, rar, 7z, tar, gz |

Anything else is indexed as *unknown*; `.mtl`, logs, caches and backups are tracked as auxiliary files and hidden from search.

## Building from source

Dependencies are managed with [vcpkg](https://github.com/microsoft/vcpkg). Install it once and point `VCPKG_ROOT` at it, then run everything from the repository root.

### Windows (Git Bash)

```bash
export VCPKG_ROOT="/c/vcpkg"
cmake --preset windows
cmake --build build --config Release
./build/Release/AssetVault.exe
```

### macOS

```bash
export VCPKG_ROOT="$HOME/vcpkg"
cmake --preset macos
cmake --build build --config Release
./build/AssetVault.app/Contents/MacOS/AssetVault
```

### Linux

A `linux-static` preset exists and builds an AppImage-ready static binary, but Linux is not yet covered by CI or prebuilt releases.

### Tests

```bash
ctest --test-dir build --output-on-failure
```

Unit tests use Catch2 and cover the search/query engine and the cross-platform file watcher.

## Project layout

```
src/            application source (indexing, database, rendering, UI)
src/ui/         panel-based ImGui interface
tests/          Catch2 unit tests
external/       vendored dependencies not available in vcpkg (ImGui, GLAD, NanoSVG, miniaudio, stb)
images/         UI icons
resources/      platform resources (Windows icon and version info)
.github/        CI workflows that build and publish the Windows and macOS releases
```

## License

MIT — see [LICENSE](LICENSE).
