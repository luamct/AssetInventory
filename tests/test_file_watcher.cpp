#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

#include "src/file_watcher.h"


void on_file_event(const FileEvent& event) {
  std::string event_type;
  switch (event.type) {
    case FileEventType::Created:
      event_type = "CREATED";
      break;
    case FileEventType::Modified:
      event_type = "MODIFIED";
      break;
    case FileEventType::Deleted:
      event_type = "DELETED";
      break;
    case FileEventType::Renamed:
      event_type = "RENAMED";
      break;
    case FileEventType::DirectoryCreated:
      event_type = "DIR_CREATED";
      break;
    case FileEventType::DirectoryDeleted:
      event_type = "DIR_DELETED";
      break;
  }

  auto time = std::chrono::system_clock::to_time_t(event.timestamp);
  std::tm timeinfo;
#ifdef _WIN32
  localtime_s(&timeinfo, &time);
#else
  localtime_r(&time, &timeinfo);
#endif

  char time_str[26];
  asctime_s(time_str, sizeof(time_str), &timeinfo);

  // Remove newline if present
  std::string time_string(time_str);
  if (!time_string.empty() && time_string.back() == '\n') {
    time_string.pop_back();
  }

  std::cout << "[" << time_string << "] " << event_type << ": " << event.path;
  if (!event.old_path.empty()) {
    std::cout << " (from: " << event.old_path << ")";
  }
  std::cout << '\n';
}

int main() {
  std::cout << '\n';
  std::cout << "Asset Inventory File Watcher Test\n";
  std::cout << "==================================\n";

  FileWatcher watcher;
  std::string watch_path = "assets";

  // Check if directory exists
  if (!std::filesystem::exists(watch_path)) {
    std::cerr << "Failed to start watching directory: " << watch_path << '\n';
    return 1;
  }

  std::cout << "Watching directory: " << watch_path << '\n';

  // Set up callback
  auto callback = [](const FileEvent& event) {
    std::string event_type;
    switch (event.type) {
      case FileEventType::Created:
        event_type = "Created";
        break;
      case FileEventType::Modified:
        event_type = "Modified";
        break;
      case FileEventType::Deleted:
        event_type = "Deleted";
        break;
      case FileEventType::Renamed:
        event_type = "Renamed";
        break;
      case FileEventType::DirectoryCreated:
        event_type = "DirectoryCreated";
        break;
      case FileEventType::DirectoryDeleted:
        event_type = "DirectoryDeleted";
        break;
    }
    std::cout << "File event: " << event_type << " - " << event.path << '\n';
  };

  // Start watching
  if (!watcher.start_watching(watch_path, callback)) {
    std::cerr << "Failed to start file watcher\n";
    return 1;
  }

  std::cout << '\n';
  std::cout << "Press Ctrl+C to stop...\n";

  // Keep running until interrupted
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::cout << "File watcher stopped.\n";
  return 0;
}
