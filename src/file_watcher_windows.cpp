#include <windows.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include "file_watcher.h"

class WindowsFileWatcher : public FileWatcherImpl {
 private:
  HANDLE h_directory;
  HANDLE h_event;
  std::thread watch_thread;
  std::atomic<bool> should_stop;
  std::atomic<bool> is_watching_flag;
  FileEventCallback callback;
  std::string watched_path;

  // Buffer for file change notifications
  char buffer[4096];
  DWORD bytes_returned;

  // Thread-safe event queue
  std::queue<FileEvent> event_queue;
  std::mutex queue_mutex;
  std::condition_variable queue_cv;

 public:
  WindowsFileWatcher()
      : h_directory(INVALID_HANDLE_VALUE),
        h_event(INVALID_HANDLE_VALUE),
        should_stop(false),
        is_watching_flag(false) {}

  ~WindowsFileWatcher() { stop_watching(); }

  bool start_watching(const std::string& path, FileEventCallback cb) override {
    if (is_watching_flag.load()) {
      std::cerr << "Already watching a directory\n";
      return false;
    }

    watched_path = path;
    callback = cb;

    // Create event for signaling
    h_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (h_event == nullptr) {
      std::cerr << "Failed to create event: " << GetLastError() << '\n';
      return false;
    }

    // Open directory handle
    h_directory = CreateFileW(
        std::wstring(path.begin(), path.end()).c_str(), FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr);

    if (h_directory == INVALID_HANDLE_VALUE) {
      std::cerr << "Failed to open directory: " << GetLastError() << '\n';
      CloseHandle(h_event);
      return false;
    }

    should_stop = false;
    is_watching_flag = true;

    // Start watching thread
    watch_thread = std::thread(&WindowsFileWatcher::watch_loop, this);

    std::cout << "Started watching directory: " << path << '\n';
    return true;
  }

  void stop_watching() override {
    if (!is_watching_flag.load()) return;

    should_stop = true;

    // Signal the event to wake up the thread
    if (h_event != INVALID_HANDLE_VALUE) {
      SetEvent(h_event);
    }

    // Wait for thread to finish
    if (watch_thread.joinable()) {
      watch_thread.join();
    }

    // Clean up handles
    if (h_directory != INVALID_HANDLE_VALUE) {
      CloseHandle(h_directory);
      h_directory = INVALID_HANDLE_VALUE;
    }

    if (h_event != INVALID_HANDLE_VALUE) {
      CloseHandle(h_event);
      h_event = INVALID_HANDLE_VALUE;
    }

    is_watching_flag = false;
    std::cout << "Stopped watching directory: " << watched_path << '\n';
  }

  bool is_watching() const override { return is_watching_flag.load(); }

 private:
  void perform_initial_scan(const std::string& directory_path) {
    try {
      std::filesystem::path path(directory_path);

      if (!std::filesystem::exists(path)) {
        std::cerr << "Directory does not exist: " << directory_path << '\n';
        return;
      }

      if (!std::filesystem::is_directory(path)) {
        std::cerr << "Path is not a directory: " << directory_path << '\n';
        return;
      }

      // Recursively scan the directory
      scan_directory_recursive(path, directory_path);

      std::cout << "Initial scan completed\n";
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Filesystem error during initial scan: " << e.what() << '\n';
    } catch (const std::exception& e) {
      std::cerr << "Error during initial scan: " << e.what() << '\n';
    }
  }

  void scan_directory_recursive(const std::filesystem::path& current_path,
                                const std::string& base_path) {
    try {
      for (const auto& entry :
           std::filesystem::recursive_directory_iterator(current_path)) {
        if (should_stop.load()) {
          return;  // Stop scanning if we're being shut down
        }

        std::string relative_path =
            std::filesystem::relative(entry.path(), base_path).string();
        std::string full_path = entry.path().string();

        // Convert backslashes to forward slashes for consistency
        std::replace(relative_path.begin(), relative_path.end(), '\\', '/');
        std::replace(full_path.begin(), full_path.end(), '\\', '/');

        if (std::filesystem::is_directory(entry)) {
          // Create event for directory
          FileEvent event(FileEventType::DirectoryCreated, full_path);
          if (callback) {
            callback(event);
          }
        } else if (std::filesystem::is_regular_file(entry)) {
          // Create event for file
          FileEvent event(FileEventType::Created, full_path);
          if (callback) {
            callback(event);
          }
        }
      }
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Error scanning directory " << current_path.string() << ": "
                << e.what() << '\n';
    }
  }

  void watch_loop() {
    OVERLAPPED overlapped = {0};
    overlapped.hEvent = h_event;

    while (!should_stop.load()) {
      // Start monitoring
      if (!ReadDirectoryChangesW(
              h_directory, buffer, sizeof(buffer),
              TRUE,  // Watch subtree
              FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                  FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                  FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
              nullptr, &overlapped, nullptr)) {
        std::cerr << "ReadDirectoryChangesW failed: " << GetLastError() << '\n';
        break;
      }

      // Wait for either a change notification or stop signal
      DWORD wait_result = WaitForSingleObject(h_event, INFINITE);

      if (should_stop.load()) {
        break;
      }

      if (wait_result == WAIT_OBJECT_0) {
        // Get the overlapped result
        DWORD bytes_transferred;
        if (GetOverlappedResult(h_directory, &overlapped, &bytes_transferred,
                                FALSE)) {
          process_file_changes(buffer, bytes_transferred);
        }

        // Reset event for next iteration
        ResetEvent(h_event);
      }
    }
  }

  void process_file_changes(const char* change_buffer,
                            DWORD bytes_transferred) {
    (void)bytes_transferred;  // Suppress unused parameter warning

    const FILE_NOTIFY_INFORMATION* p_notify =
        reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(change_buffer);

    while (p_notify) {
      // Convert wide string to UTF-8 properly
      std::wstring wide_name(p_notify->FileName,
                             p_notify->FileNameLength / sizeof(WCHAR));
      std::string file_name = wide_string_to_utf8(wide_name);

      // Determine event type
      FileEventType event_type = FileEventType::Modified;  // Default
      switch (p_notify->Action) {
        case FILE_ACTION_ADDED:
          event_type = FileEventType::Created;
          break;
        case FILE_ACTION_REMOVED:
          event_type = FileEventType::Deleted;
          break;
        case FILE_ACTION_MODIFIED:
          event_type = FileEventType::Modified;
          break;
        case FILE_ACTION_RENAMED_OLD_NAME:
          // We'll handle rename in the next iteration
          break;
        case FILE_ACTION_RENAMED_NEW_NAME:
          event_type = FileEventType::Renamed;
          break;
        default:
          event_type = FileEventType::Modified;
          break;
      }

      // Create full path
      std::string full_path = watched_path + "\\" + file_name;

      // Create and queue event
      FileEvent event(event_type, full_path);

      // Call callback in main thread context
      if (callback) {
        callback(event);
      }

      // Move to next notification
      if (p_notify->NextEntryOffset == 0) {
        break;
      }
      p_notify = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(
          reinterpret_cast<const char*>(p_notify) + p_notify->NextEntryOffset);
    }
  }

  // Helper function to convert wide string to UTF-8
  std::string wide_string_to_utf8(const std::wstring& wide_str) {
    if (wide_str.empty()) return std::string();

    // Convert wide string to UTF-8
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide_str.data(), -1,
                                          nullptr, 0, nullptr, nullptr);
    if (size_needed > 0) {
      std::string utf8_str(size_needed, 0);
      WideCharToMultiByte(CP_UTF8, 0, wide_str.data(), -1, &utf8_str[0],
                          size_needed, nullptr, nullptr);
      return utf8_str;
    }
    return std::string();
  }
};

// Factory function for Windows implementation
std::unique_ptr<FileWatcherImpl> create_windows_file_watcher_impl() {
  return std::make_unique<WindowsFileWatcher>();
}
