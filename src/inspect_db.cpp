#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

#include "asset_database.h"
#include "asset_index.h"

int main() {
  AssetDatabase db;
  if (!db.initialize("db/assets.db")) {
    std::cerr << "Failed to open database!\n";
    return 1;
  }

  std::cout << "=== Database Contents ===\n";
  std::cout << "Total assets: " << db.get_total_asset_count() << '\n';
  std::cout << "Total size: " << db.get_total_size() << " bytes\n";
  std::cout << '\n';

  // Get all assets
  std::vector<FileInfo> all_assets = db.get_all_assets();

  // Print all assets in a table format
  std::cout << std::left << std::setw(40) << "Name" << std::setw(15) << "Type"
            << std::setw(12) << "Size" << std::setw(20) << "Modified"
            << "Path\n";
  std::cout << std::string(95, '-') << '\n';

  for (const auto &asset : all_assets) {
    // Format the last modified time
    auto time_t = std::chrono::system_clock::to_time_t(asset.last_modified);
    std::stringstream ss;

    // Use thread-safe localtime_s on Windows, fallback to localtime on other
    // platforms
#ifdef _WIN32
    struct tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
#else
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
#endif

    std::cout << std::left << std::setw(40) << asset.name << std::setw(15)
              << get_asset_type_string(asset.type) << std::setw(12)
              << asset.size << std::setw(20) << ss.str() << asset.relative_path
              << '\n';
  }

  std::cout << '\n';
  std::cout << "=== Statistics by Type ===\n";

  // Get statistics by type
  std::map<AssetType, int> type_count;
  std::map<AssetType, uint64_t> type_size;

  for (const auto &asset : all_assets) {
    type_count[asset.type]++;
    if (!asset.is_directory) {
      type_size[asset.type] += asset.size;
    }
  }

  for (const auto &pair : type_count) {
    std::string type_name = get_asset_type_string(pair.first);
    int count = pair.second;
    uint64_t size = type_size[pair.first];
    std::cout << type_name << ": " << count << " files, " << size << " bytes\n";
  }

  return 0;
}
