@echo off
echo Downloading dependencies for Asset Inventory...

REM Create directories if they don't exist
if not exist "external\imgui" mkdir external\imgui
if not exist "external\glfw" mkdir external\glfw
if not exist "external\fonts" mkdir external\fonts
if not exist "external\sqlite" mkdir external\sqlite

echo.
echo Downloading SQLite amalgamation...
powershell -Command "& {Invoke-WebRequest -Uri 'https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip' -OutFile 'sqlite-amalgamation.zip'}"
powershell -Command "& {Expand-Archive -Path 'sqlite-amalgamation.zip' -DestinationPath 'temp' -Force}"
xcopy "temp\sqlite-amalgamation-3450100\*" "external\sqlite\" /E /I /Y
rmdir /S /Q temp
del sqlite-amalgamation.zip

echo.
echo Downloading Dear ImGui...
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/imgui.h' -OutFile 'external\imgui\imgui.h'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/imgui.cpp' -OutFile 'external\imgui\imgui.cpp'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/imgui_demo.cpp' -OutFile 'external\imgui\imgui_demo.cpp'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/imgui_draw.cpp' -OutFile 'external\imgui\imgui_draw.cpp'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/imgui_tables.cpp' -OutFile 'external\imgui\imgui_tables.cpp'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/imgui_widgets.cpp' -OutFile 'external\imgui\imgui_widgets.cpp'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/imconfig.h' -OutFile 'external\imgui\imconfig.h'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/imgui_internal.h' -OutFile 'external\imgui\imgui_internal.h'}"

REM Download stb_image.h for image loading
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/nothings/stb/master/stb_image.h' -OutFile 'external\imgui\stb_image.h'}"

echo Downloading STB library files...
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/imstb_rectpack.h' -OutFile 'external\imgui\imstb_rectpack.h'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/imstb_textedit.h' -OutFile 'external\imgui\imstb_textedit.h'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/imstb_truetype.h' -OutFile 'external\imgui\imstb_truetype.h'}"

REM Create backends directory
if not exist "external\imgui\backends" mkdir external\imgui\backends

echo Downloading Dear ImGui backends...
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/backends/imgui_impl_glfw.cpp' -OutFile 'external\imgui\backends\imgui_impl_glfw.cpp'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/backends/imgui_impl_glfw.h' -OutFile 'external\imgui\backends\imgui_impl_glfw.h'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/backends/imgui_impl_opengl3.cpp' -OutFile 'external\imgui\backends\imgui_impl_opengl3.cpp'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/backends/imgui_impl_opengl3.h' -OutFile 'external\imgui\backends\imgui_impl_opengl3.h'}"
powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/ocornut/imgui/master/backends/imgui_impl_opengl3_loader.h' -OutFile 'external\imgui\backends\imgui_impl_opengl3_loader.h'}"

echo.
echo Downloading Roboto Font...
powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/google/fonts/raw/main/apache/roboto/Roboto-Regular.ttf' -OutFile 'external\fonts\Roboto-Regular.ttf'}"

echo.
echo Downloading GLFW...
powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.zip' -OutFile 'glfw.zip'}"
powershell -Command "& {Expand-Archive -Path 'glfw.zip' -DestinationPath 'temp' -Force}"
xcopy "temp\glfw-3.3.8\*" "external\glfw\" /E /I /Y
rmdir /S /Q temp
del glfw.zip

echo.
echo Dependencies downloaded successfully!
echo You can now build the project with: cmake -B build -S . -G "Visual Studio 17 2022" -A x64
echo.
pause
