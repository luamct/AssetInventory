#include "file_watcher.h"

#include <iostream>

// Platform-specific factory functions
#ifdef _WIN32
std::unique_ptr<FileWatcherImpl> create_windows_file_watcher_impl();
#else
std::unique_ptr<FileWatcherImpl> create_polling_file_watcher_impl();
#endif

// Declare the polling factory function for Windows fallback
std::unique_ptr<FileWatcherImpl> create_polling_file_watcher_impl();

std::unique_ptr<FileWatcherImpl> create_file_watcher_impl() {
#ifdef _WIN32
  std::cout << "Using native Windows file watcher\n";
  return create_windows_file_watcher_impl();
#else
  std::cout << "Using polling-based file watcher\n";
  return create_polling_file_watcher_impl();
#endif
}

std::unique_ptr<FileWatcherImpl> create_fallback_file_watcher_impl() {
  std::cout << "Falling back to polling-based file watcher\n";
  return create_polling_file_watcher_impl();
}

FileWatcher::FileWatcher() : p_impl(nullptr) {
  p_impl = create_file_watcher_impl();
  if (!p_impl) {
    p_impl = create_fallback_file_watcher_impl();
  }
  if (!p_impl) {
    std::cerr << "No file watcher implementation available\n";
  }
}

FileWatcher::~FileWatcher() { stop_watching(); }

bool FileWatcher::start_watching(const std::string& path,
                                 FileEventCallback callback) {
  if (!p_impl) {
    std::cerr << "No file watcher implementation available\n";
    return false;
  }

  watched_path = path;
  is_watching_flag = true;

  return p_impl->start_watching(path, callback);
}

void FileWatcher::stop_watching() {
  if (p_impl) {
    p_impl->stop_watching();
  }
  is_watching_flag = false;
}

bool FileWatcher::is_watching() const {
  return is_watching_flag && p_impl && p_impl->is_watching();
}

std::string FileWatcher::get_watched_path() const { return watched_path; }

void FileWatcher::set_file_extensions(
    const std::vector<std::string>& extensions) {
  file_extensions = extensions;
}

void FileWatcher::set_polling_interval(int milliseconds) {
  polling_interval = milliseconds;
}
