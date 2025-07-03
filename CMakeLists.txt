cmake_minimum_required(VERSION 3.16)
project(AssetInventory VERSION 1.0.0 LANGUAGES C CXX)

# Generate compilation database for Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set runtime library for MSVC to avoid linking conflicts
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    # Explicitly exclude the static runtime library that's causing conflicts
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
endif()

# MSVC-specific settings
if(MSVC)
    # Enable optimizations in release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
    endif()
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(OpenGL REQUIRED)

# SQLite setup - compile from source
set(SQLITE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite)

# Create SQLite library
add_library(sqlite3 STATIC ${SQLITE_DIR}/sqlite3.c)

# Set SQLite runtime library to match main application
if(MSVC)
    set_property(TARGET sqlite3 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()


# Set SQLite include directories
target_include_directories(sqlite3 PRIVATE ${SQLITE_DIR})

# Set SQLite compile definitions for better performance
target_compile_definitions(sqlite3 PRIVATE
    SQLITE_ENABLE_FTS5
    SQLITE_ENABLE_JSON1
    SQLITE_ENABLE_RTREE
    SQLITE_ENABLE_UNLOCK_NOTIFY
    SQLITE_ENABLE_DBSTAT_VTAB
    SQLITE_ENABLE_LOAD_EXTENSION=1
    SQLITE_THREADSAFE=1
    SQLITE_USE_URI=1
    SQLITE_ENABLE_COLUMN_METADATA
)

# Use precompiled GLFW binaries
set(GLFW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include")
set(GLFW_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib-vc2022/glfw3.lib")

# Create GLFW imported target (static library)
add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION ${GLFW_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${GLFW_INCLUDE_DIR}
)

message(STATUS "Using precompiled GLFW: include at ${GLFW_INCLUDE_DIR}, lib at ${GLFW_LIBRARY}")

# Set GLFW runtime library to match main application
if(MSVC)
    set_property(TARGET glfw PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Add Dear ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# File watcher sources
set(FILE_WATCHER_SOURCES
    src/file_watcher.cpp
    src/file_watcher_polling.cpp
)

# Platform-specific file watcher sources
list(APPEND FILE_WATCHER_SOURCES src/file_watcher_windows.cpp)
# Windows-specific definitions
add_definitions(
    -DWIN32_LEAN_AND_MEAN
    -D_UNICODE
    -DUNICODE
    -D_WIN32_WINNT=0x0601
    -DNOMINMAX
)

# Add executable
add_executable(AssetInventory
    src/main.cpp
    src/asset_index.cpp
    src/asset_database.cpp
    ${IMGUI_SOURCES}
    ${FILE_WATCHER_SOURCES}
)

# Set runtime library for all executables
if(MSVC)
    set_property(TARGET AssetInventory PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Add file watcher test executable
add_executable(FileWatcherTest
    tests/test_file_watcher.cpp
    ${FILE_WATCHER_SOURCES}
)

if(MSVC)
    set_property(TARGET FileWatcherTest PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Add database test executable
add_executable(DatabaseTest
    tests/test_database.cpp
    src/asset_index.cpp
    src/asset_database.cpp
)

if(MSVC)
    set_property(TARGET DatabaseTest PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Add database inspection executable
add_executable(InspectDB
    src/inspect_db.cpp
    src/asset_index.cpp
    src/asset_database.cpp
)

if(MSVC)
    set_property(TARGET InspectDB PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Set compiler flags for our own code
if(MSVC)
    target_compile_options(AssetInventory PRIVATE /W4)
    target_compile_options(FileWatcherTest PRIVATE /W4)
    target_compile_options(DatabaseTest PRIVATE /W4)
    target_compile_options(InspectDB PRIVATE /W4)
endif()

# Suppress warnings for external libraries
if(MSVC)
    set(EXTERNAL_WARNING_FLAGS /w)

    # SQLite (C library)
    target_compile_options(sqlite3 PRIVATE ${EXTERNAL_WARNING_FLAGS})

    # GLFW is pre-built binary, no warning suppression needed

    # ImGui sources (C++ but external)
    foreach(imgui_source ${IMGUI_SOURCES})
        set_source_files_properties(${imgui_source} PROPERTIES COMPILE_FLAGS ${EXTERNAL_WARNING_FLAGS})
    endforeach()

    # Suppress warnings for stb_image.h specifically
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/stb_image.h PROPERTIES COMPILE_FLAGS ${EXTERNAL_WARNING_FLAGS})
endif()

# Include directories
target_include_directories(AssetInventory PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${SQLITE_DIR}
    ${GLFW_INCLUDE_DIR}
)

target_include_directories(FileWatcherTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${SQLITE_DIR}
    ${GLFW_INCLUDE_DIR}
)

target_include_directories(DatabaseTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${SQLITE_DIR}
    ${GLFW_INCLUDE_DIR}
)

target_include_directories(InspectDB PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${SQLITE_DIR}
    ${GLFW_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(AssetInventory PRIVATE
    glfw
    OpenGL::GL
    sqlite3
)

# Add filesystem library for MSVC
if(MSVC)
    target_link_libraries(AssetInventory PRIVATE legacy_stdio_definitions)
endif()



target_link_libraries(FileWatcherTest PRIVATE
    glfw
    OpenGL::GL
)



target_link_libraries(DatabaseTest PRIVATE
    sqlite3
)



target_link_libraries(InspectDB PRIVATE
    sqlite3
)



# Copy font file to build directory
add_custom_command(TARGET AssetInventory POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/external/fonts/Roboto-Regular.ttf
    ${CMAKE_BINARY_DIR}/external/fonts/Roboto-Regular.ttf
    COMMENT "Copying Roboto font to build directory"
)
