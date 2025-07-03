# File Watcher Implementation for Asset Inventory

This document explains the file watching options implemented for the Asset Inventory project and their performance characteristics.

## Overview

The file watcher system provides real-time monitoring of asset directories to automatically detect when files are added, modified, or deleted. This enables the Asset Inventory to stay synchronized with the file system without manual refresh.

## Implementation Options

### 1. **Native Windows Implementation (Recommended for Windows)**

**File:** `src/FileWatcher_Windows.cpp`

**Technology:** `ReadDirectoryChangesW` API

**Performance Characteristics:**
- ✅ **Ultra-low latency** (< 1ms response time)
- ✅ **Kernel-level notifications** (no polling overhead)
- ✅ **Minimal CPU usage** (only active when changes occur)
- ✅ **Supports all file system events** (create, modify, delete, rename)
- ✅ **Recursive directory monitoring**

**How it works:**
- Uses Windows' native file system change notifications
- Creates an overlapped I/O operation that waits for file system events
- Events are delivered directly from the kernel to the application
- No polling or file system scanning required

**Best for:** Production Windows environments where performance is critical

### 2. **Cross-Platform Polling Implementation (Fallback)**

**File:** `src/FileWatcher_Polling.cpp`

**Technology:** `std::filesystem` with periodic scanning

**Performance Characteristics:**
- ⚠️ **Configurable latency** (depends on polling interval)
- ⚠️ **CPU usage scales with directory size**
- ✅ **Cross-platform compatibility**
- ✅ **Simple and reliable**
- ✅ **Customizable file filtering**

**How it works:**
- Periodically scans the directory tree for changes
- Compares file timestamps and sizes with previous state
- Configurable polling interval (default: 1000ms)
- Supports file extension filtering

**Best for:** Cross-platform development, testing, or when native APIs aren't available

## Performance Comparison

| Aspect | Native Windows | Polling |
|--------|----------------|---------|
| **Latency** | < 1ms | 100-5000ms |
| **CPU Usage** | Minimal | Moderate |
| **Memory Usage** | Low | Low |
| **Scalability** | Excellent | Good (with filtering) |
| **Cross-Platform** | Windows only | All platforms |

## Usage Examples

### Basic Usage

```cpp
#include "include/FileWatcher.h"

FileWatcher watcher;

// Start watching a directory
watcher.StartWatching("assets", [](const FileEvent& event) {
    switch (event.type) {
        case FileEventType::Created:
            std::cout << "New file: " << event.path << std::endl;
            break;
        case FileEventType::Modified:
            std::cout << "Modified: " << event.path << std::endl;
            break;
        case FileEventType::Deleted:
            std::cout << "Deleted: " << event.path << std::endl;
            break;
    }
});
```

### Advanced Usage with Filtering

```cpp
FileWatcher watcher;

// Filter by file extensions
std::vector<std::string> extensions = {".png", ".jpg", ".fbx", ".obj"};
watcher.SetFileExtensions(extensions);

// Set polling interval (for polling implementation)
watcher.SetPollingInterval(500); // 500ms

watcher.StartWatching("assets", OnFileEvent);
```

## Integration with Asset Inventory

The file watcher integrates seamlessly with the existing Asset Inventory system:

1. **Automatic Detection:** New assets are automatically detected and indexed
2. **Real-time Updates:** UI updates immediately when files change
3. **Performance Optimized:** Uses the most efficient method for each platform
4. **Thread-Safe:** Callbacks are executed in the main thread context

## Platform-Specific Considerations

### Windows
- Uses `ReadDirectoryChangesW` for optimal performance
- Supports all file system events
- Minimal resource usage
- Automatic fallback to polling if native API fails

### Linux/macOS
- Currently uses polling implementation
- Future: Can be extended with `inotify` (Linux) or `FSEvents` (macOS)
- Good performance with appropriate polling intervals

## Configuration Options

### Polling Interval
```cpp
watcher.SetPollingInterval(1000); // 1 second
```

**Recommendations:**
- **Development:** 500-1000ms (good balance of responsiveness and performance)
- **Production:** 2000-5000ms (reduces CPU usage for large directories)
- **Real-time:** 100-500ms (maximum responsiveness, higher CPU usage)

### File Extension Filtering
```cpp
std::vector<std::string> extensions = {".png", ".jpg", ".jpeg", ".fbx", ".obj"};
watcher.SetFileExtensions(extensions);
```

**Benefits:**
- Reduces unnecessary processing
- Improves performance for large directories
- Focuses on relevant asset types

## Testing

Run the file watcher test to verify functionality:

```bash
# Build the test
cmake --build build --target FileWatcherTest

# Run the test
./build/FileWatcherTest
```

The test will watch the `assets` directory and report all file system events.

## Future Enhancements

1. **Linux Support:** Add `inotify` implementation for native Linux performance
2. **macOS Support:** Add `FSEvents` implementation for native macOS performance
3. **Multiple Directories:** Support watching multiple directories simultaneously
4. **Event Batching:** Batch multiple events to reduce callback frequency
5. **Persistent State:** Save file state to disk for faster startup

## Troubleshooting

### Common Issues

1. **High CPU Usage (Polling Mode)**
   - Increase polling interval
   - Add file extension filtering
   - Reduce directory size being watched

2. **Missing Events (Windows)**
   - Check if directory handle has proper permissions
   - Verify antivirus software isn't interfering
   - Ensure sufficient buffer size for large directories

3. **Cross-Platform Compatibility**
   - Use polling implementation for maximum compatibility
   - Test on target platforms before deployment

### Debug Information

Enable debug output by setting environment variables:
```bash
# Windows
set ASSET_INVENTORY_DEBUG=1

# Linux/macOS
export ASSET_INVENTORY_DEBUG=1
```

## Conclusion

The file watcher system provides a robust, performant solution for real-time asset monitoring. The native Windows implementation offers optimal performance, while the polling implementation ensures cross-platform compatibility. Choose the appropriate implementation based on your platform requirements and performance needs.
