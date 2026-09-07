# CLAUDE.md

Guidance for Claude Code when working in this repository. `AGENTS.md` covers code style and commit conventions; `README.md` is the user-facing overview; `readmes/` has component deep-dives.

## Project

AssetVault is a C++17 desktop app for browsing game asset libraries: search, thumbnails, 3D/audio preview, drag-out to other apps, and a live file watcher. Built with Dear ImGui, OpenGL 3.3, SQLite, Assimp, and miniaudio; dependencies via vcpkg; CMake presets for Windows, macOS, and Linux. MIT licensed.

## Working with the user

- Be candid and critical; skip praise. If a suggestion is problematic, say so and propose an alternative.
- Questions ("how does X work?", "what's the best way to…?") get answers, not code changes. Only change code on explicit requests ("implement", "fix", "add"). Ask when unsure.
- **Never run git commands unless explicitly asked.** Never assume the user wants a commit.
- **UI changes need user confirmation** — you can't see the result. Never claim a visual fix worked.
- Always stay in the repo root; use relative paths.
- Don't add comments that only describe the latest change (`// reduced from 3 to 1`).
- When the user gives new guidelines, add them here.

## Build, run, test

Set `VCPKG_ROOT` first (`$HOME/vcpkg` on macOS/Linux, `/c/vcpkg` on Windows; use Git Bash on Windows). Use the **Debug** config during development.

```bash
cmake --preset macos            # or: windows, windows-static, linux-static (plus *-asan variants)
cmake --build build --config Debug
./build/AssetVault.app/Contents/MacOS/AssetVault   # macOS
./build/Debug/AssetVault.exe                       # Windows
ctest --test-dir build --output-on-failure         # all tests
```

Test executables (Catch2, registered with CTest): `SearchTest`, `DbTest`, `EventProcessorTest`, `UtilsTest`, `UITest`, `IntegrationTest`, and `FileWatcherMacOSTest` / `FileWatcherWindowsTest` per platform. Tests set `TESTING=1`, which makes the app run headless and use `build/data` for its data directory. Prefer adding unit tests over ad-hoc scripts or manual checks. Clean build: `rm -rf build/`.

## Architecture

**Event-driven pipeline.** All asset changes — the initial scan and runtime file-watcher events — flow through `EventProcessor` as `FileEvent`s (Created / Modified / Deleted), processed in batches into the database, the in-memory asset map, and the search index. Never write to the database or asset map directly. `FileEvent` has no default constructor; use `FileEvent(type, path)`.

**Services.** `src/services.h` is a static service locator (`Services::provide(...)` at startup, `Services::database()` etc. afterwards). Tests inject mocks the same way. The shared asset map is a mutex-guarded `SafeAssets`; lock it for any cross-thread access.

**Threads.** Main thread runs ImGui; `EventProcessor` and `FileWatcher` each run their own background thread. Progress is reported via atomics.

**UI.** `src/ui/` is panel-based: `search_panel`, `results_panel`, `preview_panel`, `tree_panel`, with shared widgets in `components.h/.cpp` — add reusable widgets there. All colors come from `Theme::` in `src/theme.h`; never hardcode color values.

**Data.** Database, thumbnails, and settings (including the user-chosen assets directory) live in a per-user data directory — `%LOCALAPPDATA%\AssetVault` on Windows, `~/Library/Application Support/AssetVault` on macOS — resolved by `Config::get_data_directory()`. The assets folder is chosen in-app, not by convention.

**Asset types.** `AssetType` in `src/asset.h`: `_2D, _3D, Audio, Font, Shader, Document, Archive, Directory, Auxiliary, Unknown`. Extension mapping and string conversion live in `src/asset.cpp`; the database stores lowercase type names. To add a type, update the enum, both string conversion functions, the extension map, and the icon mapping in `texture_manager.cpp`. Always convert strings with `get_asset_type_from_string()`.

**Platform code.** `file_watcher_{macos,windows}.cpp` (FSEvents / ReadDirectoryChangesW), `drag_drop_{macos.mm,windows.cpp}`. macOS atomic saves (Preview, TextEdit) arrive as rename events and must be treated as Modified — see `readmes/FILE_WATCHER.md`.

**3D.** Assimp loading with multi-material support; models that fail to load are cached in `TextureManager` so they aren't retried every frame. Animation-only FBX files are accepted without geometry.

## Code quality

- Fix all compiler warnings.
- Avoid globals except for performance or threading; they hurt testability.
- Keep methods that don't touch instance state `static`.
- Follow DRY; extract shared helpers rather than duplicating patterns.
- Update `readmes/` when behavior changes.

## Releases

`project()` in `CMakeLists.txt` is the single source of truth for the version; it feeds the Windows `VERSIONINFO` resource and the macOS bundle. Release tags are `vX.Y.Z` (optionally `-alpha` etc.) and both CI workflows fail if the tag doesn't match `project()`. Windows code signing is planned via SignPath Foundation; macOS builds are signed and notarized in CI when the Apple secrets are present.
