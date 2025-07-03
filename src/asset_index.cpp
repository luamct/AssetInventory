#include "asset_index.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Asset type mapping based on file extensions - O(1) lookup using map
AssetType get_asset_type(const std::string& extension) {
  static const std::map<std::string, AssetType> type_map = {
      // Textures
      {".png", AssetType::Texture},
      {".jpg", AssetType::Texture},
      {".jpeg", AssetType::Texture},
      {".bmp", AssetType::Texture},
      {".tga", AssetType::Texture},
      {".dds", AssetType::Texture},
      {".hdr", AssetType::Texture},
      {".exr", AssetType::Texture},
      {".ktx", AssetType::Texture},

      // Models
      {".fbx", AssetType::Model},
      {".obj", AssetType::Model},
      {".dae", AssetType::Model},
      {".3ds", AssetType::Model},
      {".blend", AssetType::Model},
      {".max", AssetType::Model},
      {".ma", AssetType::Model},
      {".mb", AssetType::Model},
      {".c4d", AssetType::Model},

      // Audio
      {".wav", AssetType::Sound},
      {".mp3", AssetType::Sound},
      {".ogg", AssetType::Sound},
      {".flac", AssetType::Sound},
      {".aac", AssetType::Sound},
      {".m4a", AssetType::Sound},

      // Fonts
      {".ttf", AssetType::Font},
      {".otf", AssetType::Font},
      {".woff", AssetType::Font},
      {".woff2", AssetType::Font},
      {".eot", AssetType::Font},

      // Shaders
      {".vert", AssetType::Shader},
      {".frag", AssetType::Shader},
      {".geom", AssetType::Shader},
      {".tesc", AssetType::Shader},
      {".tese", AssetType::Shader},
      {".comp", AssetType::Shader},
      {".glsl", AssetType::Shader},
      {".hlsl", AssetType::Shader},

      // Documents
      {".txt", AssetType::Document},
      {".md", AssetType::Document},
      {".pdf", AssetType::Document},
      {".doc", AssetType::Document},
      {".docx", AssetType::Document},

      // Archives
      {".zip", AssetType::Archive},
      {".rar", AssetType::Archive},
      {".7z", AssetType::Archive},
      {".tar", AssetType::Archive},
      {".gz", AssetType::Archive}};

  std::string ext = extension;
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(static_cast<int>(c)));
  });

  auto it = type_map.find(ext);
  return (it != type_map.end()) ? it->second : AssetType::Unknown;
}

// Convert AssetType enum to string for display
std::string get_asset_type_string(AssetType type) {
  switch (type) {
    case AssetType::Texture:
      return "Texture";
    case AssetType::Model:
      return "Model";
    case AssetType::Sound:
      return "Sound";
    case AssetType::Font:
      return "Font";
    case AssetType::Shader:
      return "Shader";
    case AssetType::Document:
      return "Document";
    case AssetType::Archive:
      return "Archive";
    case AssetType::Directory:
      return "Directory";
    case AssetType::Unknown:
      return "Unknown";
    default:
      return "Unknown";
  }
}

// Recursively scan directory and collect file information
std::vector<FileInfo> scan_directory(const std::string& root_path) {
  std::vector<FileInfo> files;

  try {
    fs::path root(root_path);
    if (!fs::exists(root) || !fs::is_directory(root)) {
      std::cerr << "Error: Path does not exist or is not a directory: "
                << root_path << '\n';
      return files;
    }

    std::cout << "Scanning directory: " << root_path << '\n';

    for (const auto& entry : fs::recursive_directory_iterator(root)) {
      FileInfo file_info;

      // Basic file information
      file_info.full_path = entry.path().string();
      file_info.name = entry.path().filename().string();
      file_info.is_directory = entry.is_directory();

      // Relative path from root
      file_info.relative_path = fs::relative(entry.path(), root).string();

      if (!file_info.is_directory) {
        // File-specific information
        file_info.extension = entry.path().extension().string();
        file_info.type = get_asset_type(file_info.extension);

        try {
          file_info.size = fs::file_size(entry.path());
          // Convert file_time_type to system_clock::time_point (portable way)
          auto ftime = fs::last_write_time(entry.path());
          auto sctp =
              std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                  ftime - fs::file_time_type::clock::now() +
                  std::chrono::system_clock::now());
          file_info.last_modified = sctp;
        } catch (const fs::filesystem_error& e) {
          std::cerr << "Warning: Could not get file info for "
                    << file_info.full_path << ": " << e.what() << '\n';
          file_info.size = 0;
        }
      } else {
        file_info.type = AssetType::Directory;
        file_info.extension = "";
        file_info.size = 0;
      }

      files.push_back(file_info);
    }

    std::cout << "Found " << files.size() << " files and directories\n";

  } catch (const fs::filesystem_error& e) {
    std::cerr << "Error scanning directory: " << e.what() << '\n';
  }

  return files;
}

// Print file information for debugging
void print_file_info(const FileInfo& file) {
  std::cout << "Name: " << file.name << '\n';
  std::cout << "  Path: " << file.relative_path << '\n';
  std::cout << "  Type: " << get_asset_type_string(file.type) << '\n';
  std::cout << "  Size: " << file.size << " bytes" << '\n';
  std::cout << "  Extension: " << file.extension << '\n';
  std::cout << "  Is Directory: " << (file.is_directory ? "Yes" : "No") << '\n';
  std::cout << "---" << '\n';
}

// Test function to demonstrate the indexing
void test_indexing() {
  // Hardcoded path for testing - change this to your assets folder
  std::string scan_path = "assets";

  std::cout << "Starting file indexing...\n";
  auto start_time = std::chrono::high_resolution_clock::now();

  std::vector<FileInfo> files = scan_directory(scan_path);

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);

  std::cout << "\nIndexing completed in " << duration.count() << "ms\n";
  std::cout << "Total files found: " << files.size() << '\n';

  // Print first 10 files as a sample
  std::cout << "\nSample files:" << '\n';
  int count = 0;
  for (const auto& file : files) {
    if (count++ >= 10) break;
    print_file_info(file);
  }

  // Count files by type
  std::map<std::string, int> type_count;
  for (const auto& file : files) {
    type_count[get_asset_type_string(file.type)]++;
  }

  std::cout << "\nFiles by type:" << '\n';
  for (const auto& pair : type_count) {
    std::cout << "  " << pair.first << ": " << pair.second << '\n';
  }
}
