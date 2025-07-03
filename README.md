# Asset Inventory - Game Asset Management Tool

A C++ desktop application for managing game assets efficiently, built with Dear ImGui for a lightweight and fast GUI.

## Features

- **Lightweight GUI** - Built with Dear ImGui for fast, responsive interface
- **Cross-platform** - Works on Windows, Linux, and macOS
- **Modern C++** - Uses C++17 standard
- **Fast development** - Immediate mode GUI for rapid prototyping
- **File Watching** - Real-time file system monitoring for asset changes

## Documentation

- **[File Watcher System](readmes/FILE_WATCHER_README.md)** - Documentation for the real-time file monitoring system

## Prerequisites

Before building this project, ensure you have the following installed:

1. **Visual Studio Build Tools 2022** (Recommended)
   - Download from: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
   - Install the "C++ build tools" workload
   - Much smaller download (~2-3 GB) compared to full Visual Studio

2. **CMake** (version 3.16 or higher)
   - Download from: https://cmake.org/download/
   - Or install via Visual Studio Installer

3. **Git** (for version control)
   - Download from: https://git-scm.com/download/win

## Setup Instructions

### 1. Clone the Repository
```bash
git clone <your-repository-url>
cd AssetInventory
```

### 2. Download Dependencies
Run the automatic download script:
```bash
download_dependencies.bat
```

This will download:
- **Dear ImGui** - Lightweight immediate mode GUI library
- **GLFW** - Window management and OpenGL context creation

### 3. Open in Cursor/VS Code
- Open the project folder in Cursor
- Install the following extensions if prompted:
  - C/C++ (ms-vscode.cpptools)
  - CMake Tools (ms-vscode.cmake-tools)
  - CMake (twxs.cmake)

### 4. Configure and Build

#### Option A: Using VS Code Tasks (Recommended)
1. Press `Ctrl+Shift+P` to open the command palette
2. Type "Tasks: Run Task" and select it
3. Choose "CMake Configure (Visual Studio)" to set up the build
4. Choose "CMake Build" to compile the project

#### Option B: Using Command Line
```bash
# Configure the project
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

# Build the project
cmake --build build --config Debug
```

### 5. Run the Application
- The executable will be located at: `build/Debug/AssetInventory.exe`
- You can run it directly or use the debug configuration in VS Code
- Press `F5` to run with debugging

## Building with Clang and Ninja (Alternative)

For faster builds and better Clang integration, you can use Clang with Ninja:

### Prerequisites for Clang Build
1. **Clang** - Install LLVM/Clang from https://llvm.org/releases/
2. **Ninja** - Install Ninja build system from https://ninja-build.org/
3. **CMake** - Same as above

### Quick Build with Clang
```bash
# Debug build
./build_clang.bat

# Or using PowerShell
./build_clang.ps1

# Release build
./build_clang_release.bat
```

### Manual Clang Build
```bash
# Set environment variables
set CC=clang
set CXX=clang++

# Configure with Ninja
cmake -B build -S . -G "Ninja" -DCMAKE_BUILD_TYPE=debug

# Build
cmake --build build
```

### Clang Benefits
- **Faster builds** - Ninja is significantly faster than MSBuild
- **Better diagnostics** - Clang provides excellent error messages and warnings
- **Address Sanitizer** - Built-in memory error detection in debug builds
- **Cross-platform** - Same toolchain across Windows, Linux, and macOS
- **Modern C++** - Excellent C++17/20 support

### Clang-specific Features
- **Color diagnostics** - Colored compiler output for better readability
- **Enhanced warnings** - More comprehensive warning system
- **Sanitizers** - Address and Undefined Behavior sanitizers in debug builds
- **Optimizations** - Advanced optimization passes in release builds

## Project Structure

```
AssetInventory/
├── src/                   # Source files
│   ├── main.cpp           # Main application entry point
│   ├── asset_index.cpp    # Asset indexing functionality
│   ├── asset_index.h      # Asset index header
│   ├── file_watcher.cpp   # File watcher implementation
│   ├── file_watcher.h     # File watcher header
│   ├── file_watcher_polling.cpp    # Cross-platform polling watcher
│   ├── file_watcher_windows.cpp    # Windows-specific watcher
│   └── theme.h            # UI theme definitions
├── tests/                 # Test files
│   └── test_file_watcher.cpp       # File watcher test
├── readmes/               # Documentation
│   └── FILE_WATCHER_README.md      # File watcher documentation
├── CMakeLists.txt         # CMake build configuration
├── download_dependencies.bat  # Automatic dependency downloader
├── .vscode/               # VS Code configuration files
│   ├── c_cpp_properties.json  # C++ IntelliSense settings
│   ├── tasks.json             # Build tasks
│   ├── launch.json            # Debug configuration
│   └── settings.json          # Workspace settings
├── external/               # External libraries
│   ├── imgui/             # Dear ImGui library
│   └── glfw/              # GLFW window management
├── assets/                # Game assets
└── README.md              # This file
```

## GUI Framework

This project uses **Dear ImGui** for the user interface:

### Why Dear ImGui?
- ✅ **Lightweight** - Single header file, minimal dependencies
- ✅ **Fast** - Immediate mode GUI, perfect for real-time applications
- ✅ **Simple** - Easy to learn and use
- ✅ **Great for tools** - Perfect for asset management, debugging, etc.
- ✅ **Cross-platform** - Works on Windows, Linux, Mac

### Features
- **Immediate mode** - No complex state management
- **Real-time updates** - UI updates every frame
- **Keyboard navigation** - Full keyboard support
- **Docking** - Resizable, dockable windows
- **Dark theme** - Modern, easy-on-the-eyes interface

## Development

### Adding New Source Files
1. Create your `.cpp` files in the `src/` directory
2. Create your `.h` files in the `include/` directory
3. Update `CMakeLists.txt` to include new source files

### Adding GUI Elements
```cpp
// Example: Adding a button
if (ImGui::Button("My Button")) {
    // Handle button click
}

// Example: Adding text input
char buffer[256] = "";
ImGui::InputText("Name", buffer, sizeof(buffer));
```

### Debugging
- Use the "Debug AssetInventory" configuration in VS Code
- Set breakpoints by clicking in the gutter next to line numbers
- Press F5 to start debugging

### Building for Release
```bash
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## Why Visual Studio Build Tools?

- **Smaller footprint**: ~2-3 GB vs 10+ GB for full Visual Studio
- **Faster installation**: Only essential compiler tools
- **Perfect for Cursor**: You're already using Cursor as your IDE
- **Same compiler**: Uses the same MSVC compiler as full Visual Studio

## Troubleshooting

### Common Issues

1. **CMake not found**: Make sure CMake is installed and in your PATH
2. **Compiler not found**: Ensure Visual Studio Build Tools are installed with C++ workload
3. **Build errors**: Check that all prerequisites are properly installed
4. **Missing dependencies**: Run `download_dependencies.bat` to download required libraries

### Getting Help
- Check the CMake documentation: https://cmake.org/documentation/
- Visual Studio Build Tools documentation: https://docs.microsoft.com/en-us/cpp/build/
- CMake Tools extension documentation: https://github.com/microsoft/vscode-cmake-tools
- Dear ImGui documentation: https://github.com/ocornut/imgui

## License

[Add your license information here]
