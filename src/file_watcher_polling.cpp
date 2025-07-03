#include <algorithm>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <map>
#include <thread>

#include "file_watcher.h"

namespace fs = std::filesystem;

class PollingFileWatcher : public FileWatcherImpl {
private:
  std::thread watch_thread;
  std::atomic<bool> should_stop;
  std::atomic<bool> is_watching_flag;
  FileEventCallback callback;
  std::string watched_path;
  int polling_interval;
  std::vector<std::string> file_extensions;

  // Store previous state for comparison
  std::map<std::string, fs::file_time_type> previous_files;
  std::map<std::string, uint64_t> previous_sizes;

public:
  PollingFileWatcher()
      : should_stop(false), is_watching_flag(false), polling_interval(1000) {}

  ~PollingFileWatcher() { stop_watching(); }

  bool start_watching(const std::string &path, FileEventCallback cb) override {
    if (is_watching_flag.load()) {
      std::cerr << "Already watching a directory\n";
      return false;
    }

    std::filesystem::path path_obj(path);
    if (!std::filesystem::exists(path_obj) ||
        !std::filesystem::is_directory(path_obj)) {
      std::cerr << "Path does not exist or is not a directory: " << path
                << '\n';
      return false;
    }

    watched_path = path;
    callback = cb;
    is_watching_flag = true;

    // Perform initial scan
    std::cout << "Performing initial scan of directory: " << path << '\n';
    try {
      scan_directory_recursive(path_obj, path);
    } catch (const std::exception &e) {
      std::cerr << "Error during initial scan: " << e.what() << '\n';
    }

    // Start polling thread
    watch_thread = std::thread(&PollingFileWatcher::watch_loop, this);
    std::cout << "Started polling directory: " << path
              << " (interval: " << polling_interval << "ms)\n";

    // Initialize previous state
    build_file_state();

    should_stop = false;

    return true;
  }

  void stop_watching() override {
    if (!is_watching_flag.load())
      return;

    should_stop = true;

    if (watch_thread.joinable()) {
      watch_thread.join();
    }

    is_watching_flag = false;
    std::cout << "Stopped polling directory: " << watched_path << '\n';
  }

  bool is_watching() const override { return is_watching_flag.load(); }

  void set_polling_interval(int interval) { polling_interval = interval; }

  void set_file_extensions(const std::vector<std::string> &extensions) {
    file_extensions = extensions;
  }

private:
  void perform_initial_scan(const std::string &directory_path) {
    try {
      fs::path path(directory_path);

      if (!fs::exists(path)) {
        std::cerr << "Directory does not exist: " << directory_path << '\n';
        return;
      }

      if (!fs::is_directory(path)) {
        std::cerr << "Path is not a directory: " << directory_path << '\n';
        return;
      }

      // Recursively scan the directory
      scan_directory_recursive(path, directory_path);

      std::cout << "Initial scan completed\n";
    } catch (const fs::filesystem_error &e) {
      std::cerr << "Filesystem error during initial scan: " << e.what() << '\n';
    } catch (const std::exception &e) {
      std::cerr << "Error during initial scan: " << e.what() << '\n';
    }
  }

  void scan_directory_recursive(const fs::path &current_path,
                                const std::string &base_path) {
    try {
      for (const auto &entry : fs::recursive_directory_iterator(current_path)) {
        if (should_stop.load()) {
          return; // Stop scanning if we're being shut down
        }

        std::string relative_path =
            fs::relative(entry.path(), base_path).string();
        std::string full_path = entry.path().string();

        // Convert backslashes to forward slashes for consistency
        std::replace(relative_path.begin(), relative_path.end(), '\\', '/');
        std::replace(full_path.begin(), full_path.end(), '\\', '/');

        if (fs::is_directory(entry)) {
          // Create event for directory
          FileEvent event(FileEventType::DirectoryCreated, full_path);
          if (callback) {
            callback(event);
          }
        } else if (fs::is_regular_file(entry)) {
          // Create event for file
          FileEvent event(FileEventType::Created, full_path);
          if (callback) {
            callback(event);
          }
        }
      }
    } catch (const fs::filesystem_error &e) {
      std::cerr << "Error scanning directory " << current_path.string() << ": "
                << e.what() << '\n';
    }
  }

  void watch_loop() {
    while (!should_stop.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(polling_interval));
      if (should_stop.load())
        break;
      check_for_changes();
    }
  }

  void build_file_state() {
    previous_files.clear();
    previous_sizes.clear();
    try {
      for (const auto &entry : fs::recursive_directory_iterator(watched_path)) {
        std::string relative_path =
            fs::relative(entry.path(), watched_path).string();
        if (entry.is_regular_file()) {
          previous_files[relative_path] = fs::last_write_time(entry.path());
          previous_sizes[relative_path] = fs::file_size(entry.path());
        }
      }
    } catch (const fs::filesystem_error &e) {
      std::cerr << "Error building file state: " << e.what() << '\n';
    }
  }

  void check_for_changes() {
    std::map<std::string, fs::file_time_type> current_files;
    std::map<std::string, uint64_t> current_sizes;
    try {
      for (const auto &entry : fs::recursive_directory_iterator(watched_path)) {
        std::string relative_path =
            fs::relative(entry.path(), watched_path).string();
        if (entry.is_regular_file()) {
          // Check if we should filter by extension
          if (!file_extensions.empty()) {
            std::string ext = entry.path().extension().string();
            std::transform(
                ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
                  return static_cast<char>(std::tolower(static_cast<int>(c)));
                });
            bool should_include = false;
            for (const auto &filter_ext : file_extensions) {
              if (ext == filter_ext) {
                should_include = true;
                break;
              }
            }
            if (!should_include)
              continue;
          }
          current_files[relative_path] = fs::last_write_time(entry.path());
          current_sizes[relative_path] = fs::file_size(entry.path());
        }
      }
    } catch (const fs::filesystem_error &e) {
      std::cerr << "Error checking for changes: " << e.what() << '\n';
      return;
    }
    // Check for new or modified files
    for (const auto &[path, time] : current_files) {
      if (previous_files.find(path) == previous_files.end()) {
        // New file created
        FileEvent event(FileEventType::Created, watched_path + "/" + path);
        if (callback)
          callback(event);
      } else {
        // Check if file was modified
        auto prev_time = previous_files[path];
        auto prev_size = previous_sizes[path];
        if (time != prev_time || current_sizes[path] != prev_size) {
          FileEvent event(FileEventType::Modified, watched_path + "/" + path);
          if (callback)
            callback(event);
        }
      }
    }
    // Check for deleted files
    for (const auto &[path, time] : previous_files) {
      (void)time;
      if (current_files.find(path) == current_files.end()) {
        // File was deleted
        FileEvent event(FileEventType::Deleted, watched_path + "/" + path);
        if (callback)
          callback(event);
      }
    }
    // Update previous state
    previous_files = current_files;
    previous_sizes = current_sizes;
  }
};

// Factory function for polling implementation
std::unique_ptr<FileWatcherImpl> create_polling_file_watcher_impl() {
  return std::make_unique<PollingFileWatcher>();
}
