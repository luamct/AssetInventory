@echo off
echo Building Asset Inventory with Clang and Visual Studio...

REM Check if Clang is available in common locations
set CLANG_PATH=
if exist "C:\Program Files\LLVM\bin\clang++.exe" (
    set "CLANG_PATH=C:\Program Files\LLVM\bin"
) else if exist "C:\Program Files (x86)\LLVM\bin\clang++.exe" (
    set "CLANG_PATH=C:\Program Files (x86)\LLVM\bin"
) else (
    where clang++ >nul 2>&1
    if %errorlevel% neq 0 (
        echo Error: Clang++ not found
        echo Please install Clang from https://llvm.org/releases/ or add it to your PATH
        exit /b 1
    )
)

REM Create build directory
if not exist "build" mkdir build

REM Configure with CMake using Visual Studio generator
echo Configuring with CMake...
if defined CLANG_PATH (
    cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=debug -DCMAKE_C_COMPILER="%CLANG_PATH%\clang.exe" -DCMAKE_CXX_COMPILER="%CLANG_PATH%\clang++.exe"
) else (
    cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
)

if %errorlevel% neq 0 (
    echo Error: CMake configuration failed
    exit /b 1
)

REM Build the project
echo Building with Visual Studio...
cmake --build build --config debug

if %errorlevel% neq 0 (
    echo Error: Build failed
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executables are located in the build directory.
echo.
