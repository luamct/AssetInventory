#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "theme.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#include "asset_database.h"
#include "asset_index.h"
#include "file_watcher.h"

// Include stb_image for PNG loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Constants
constexpr int WINDOW_WIDTH = 1920;          // Increased from 1280
constexpr int WINDOW_HEIGHT = 1080;         // Increased from 720
constexpr float SEARCH_BOX_WIDTH = 375.0f;  // Increased from 250.0f
constexpr float SEARCH_BOX_HEIGHT = 60.0f;  // Increased from 40.0f
constexpr float THUMBNAIL_SIZE = 180.0f;    // Increased from 120.0f
constexpr float GRID_SPACING = 30.0f;       // Increased from 20.0f

// Color constants
constexpr ImU32 BACKGROUND_COLOR = IM_COL32(242, 247, 255, 255);          // Light blue-gray background
constexpr ImU32 FALLBACK_THUMBNAIL_COLOR = IM_COL32(242, 247, 255, 255);  // Same as background

// Global variables for search and UI state
static char search_buffer[256] = "";
// static bool show_search_results = false;  // Unused variable
// static unsigned int thumbnail_texture = 0; // Unused variable

// Texture cache for loaded images
struct TextureCacheEntry {
  unsigned int texture_id;
  std::string file_path;
  bool is_loaded;
  int width;
  int height;

  TextureCacheEntry() : texture_id(0), is_loaded(false), width(0), height(0) {}
};

// Global variables
std::vector<FileInfo> g_assets;
std::vector<FileInfo> g_filtered_assets;
std::atomic<bool> g_assets_updated(false);
AssetDatabase g_database;
FileWatcher g_file_watcher;
unsigned int g_default_texture = 0;

// Texture cache
std::unordered_map<std::string, TextureCacheEntry> g_texture_cache;

bool load_roboto_font(ImGuiIO &io) {
  // Load embedded Roboto font from external/fonts directory
  ImFont *font = io.Fonts->AddFontFromFileTTF("external/fonts/Roboto-Regular.ttf",
                                              24.0f);  // Increased from 16.0f
  if (font) {
    std::cout << "Roboto font loaded successfully!\n";
    return true;
  }

  // Fallback: try system fonts if embedded font not found
  // FIXME: Remove this, since we're using embedded font now
  const char *font_paths[] = {"C:/Windows/Fonts/Roboto-Regular.ttf", "C:/Windows/Fonts/roboto.ttf",
                              "C:/Windows/Fonts/Roboto-Medium.ttf"};

  for (const char *path : font_paths) {
    font = io.Fonts->AddFontFromFileTTF(path, 24.0f);  // Increased from 16.0f
    if (font) {
      std::cout << "Roboto font loaded from system: " << path << '\n';
      return true;
    }
  }

  // If Roboto is not found, use default font
  std::cout << "Roboto font not found, using default font\n";
  return false;
}

// Function to load texture from file
unsigned int load_texture(const char *filename) {
  int width, height, channels;
  unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);
  if (!data) {
    std::cerr << "Failed to load texture: " << filename << '\n';
    return 0;
  }

  unsigned int texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload texture data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  stbi_image_free(data);
  return texture_id;
}

// Function to calculate aspect-ratio-preserving dimensions
ImVec2 calculate_thumbnail_size(int original_width, int original_height, float max_size) {
  float aspect_ratio = static_cast<float>(original_width) / static_cast<float>(original_height);

  if (aspect_ratio > 1.0f) {
    // Landscape image
    return ImVec2(max_size, max_size / aspect_ratio);
  } else {
    // Portrait or square image
    return ImVec2(max_size * aspect_ratio, max_size);
  }
}

// Function to get or load texture for an asset
unsigned int get_asset_texture(const FileInfo &asset) {
  // For non-texture assets, return default texture
  if (asset.type != AssetType::Texture) {
    return g_default_texture;
  }

  // Check if texture is already cached
  auto it = g_texture_cache.find(asset.full_path);
  if (it != g_texture_cache.end()) {
    if (it->second.is_loaded) {
      return it->second.texture_id;
    }
  }

  // Load the texture and get dimensions
  int width, height, channels;
  unsigned char *data = stbi_load(asset.full_path.c_str(), &width, &height, &channels, 4);
  if (!data) {
    std::cerr << "Failed to load texture: " << asset.full_path << '\n';
    // Cache the failure
    TextureCacheEntry &entry = g_texture_cache[asset.full_path];
    entry.texture_id = 0;
    entry.file_path = asset.full_path;
    entry.is_loaded = false;
    entry.width = 0;
    entry.height = 0;
    return 0;
  }

  unsigned int texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload texture data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  stbi_image_free(data);

  // Cache the result
  TextureCacheEntry &entry = g_texture_cache[asset.full_path];
  entry.texture_id = texture_id;
  entry.file_path = asset.full_path;
  entry.is_loaded = true;
  entry.width = width;
  entry.height = height;

  return texture_id;
}

// Function to get cached texture dimensions
bool get_texture_dimensions(const std::string &file_path, int &width, int &height) {
  auto it = g_texture_cache.find(file_path);
  if (it != g_texture_cache.end() && it->second.is_loaded) {
    width = it->second.width;
    height = it->second.height;
    return true;
  }
  return false;
}

// Function to truncate filename to specified length with ellipsis
std::string truncate_filename(const std::string &filename, size_t max_length = 20) {
  if (filename.length() <= max_length) {
    return filename;
  }
  return filename.substr(0, max_length - 3) + "...";
}

// Function to convert string to lowercase for case-insensitive search
std::string to_lowercase(const std::string &str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

// Function to check if asset matches search terms
bool asset_matches_search(const FileInfo &asset, const std::string &search_query) {
  if (search_query.empty()) {
    return true;  // Show all assets when search is empty
  }

  std::string query_lower = to_lowercase(search_query);
  std::string name_lower = to_lowercase(asset.name);
  std::string extension_lower = to_lowercase(asset.extension);
  std::string path_lower = to_lowercase(asset.full_path);

  // Split search query into terms (space-separated)
  std::vector<std::string> search_terms;
  std::stringstream ss(query_lower);
  std::string search_term;
  while (ss >> search_term) {
    if (!search_term.empty()) {
      search_terms.push_back(search_term);
    }
  }

  // All terms must match (AND logic)
  for (const auto &term : search_terms) {
    bool term_matches = name_lower.find(term) != std::string::npos || extension_lower.find(term) != std::string::npos ||
                        path_lower.find(term) != std::string::npos;

    if (!term_matches) {
      return false;
    }
  }

  return true;
}

// Function to filter assets based on search query
void filter_assets(const std::string &search_query) {
  g_filtered_assets.clear();

  for (const auto &asset : g_assets) {
    if (asset_matches_search(asset, search_query)) {
      g_filtered_assets.push_back(asset);
    }
  }
}

// File event callback function
void on_file_event(const FileEvent &event) {
  switch (event.type) {
    case FileEventType::Created:
      // Fall through to Modified case
    case FileEventType::Modified: {
      // Check if it's a file (not directory)
      if (std::filesystem::is_regular_file(event.path)) {
        // Clear texture cache entry for this file so it can be reloaded
        auto cache_it = g_texture_cache.find(event.path);
        if (cache_it != g_texture_cache.end()) {
          // If there's an existing texture, delete it from OpenGL
          if (cache_it->second.is_loaded && cache_it->second.texture_id != 0) {
            glDeleteTextures(1, &cache_it->second.texture_id);
          }
          // Remove from cache
          g_texture_cache.erase(cache_it);
        }

        FileInfo file_info;
        std::filesystem::path path(event.path);

        file_info.name = path.filename().string();
        file_info.extension = path.extension().string();
        file_info.full_path = event.path;
        file_info.relative_path = event.path;  // For now, use full path
        file_info.size = std::filesystem::file_size(event.path);
        file_info.last_modified = event.timestamp;
        file_info.is_directory = false;
        file_info.type = get_asset_type(file_info.extension);

        // Insert or update in database
        auto existing_asset = g_database.get_asset_by_path(event.path);
        if (existing_asset.full_path.empty()) {
          g_database.insert_asset(file_info);
        } else {
          g_database.update_asset(file_info);
        }
        g_assets_updated = true;
      }
      break;
    }
    case FileEventType::Deleted: {
      // Clear texture cache entry for deleted file
      auto cache_it = g_texture_cache.find(event.path);
      if (cache_it != g_texture_cache.end()) {
        if (cache_it->second.is_loaded && cache_it->second.texture_id != 0) {
          glDeleteTextures(1, &cache_it->second.texture_id);
        }
        g_texture_cache.erase(cache_it);
      }

      g_database.delete_asset(event.path);
      g_assets_updated = true;
      break;
    }
    case FileEventType::Renamed: {
      // Clear texture cache entry for old path
      auto cache_it = g_texture_cache.find(event.old_path);
      if (cache_it != g_texture_cache.end()) {
        if (cache_it->second.is_loaded && cache_it->second.texture_id != 0) {
          glDeleteTextures(1, &cache_it->second.texture_id);
        }
        g_texture_cache.erase(cache_it);
      }

      // Delete old entry and create new one
      g_database.delete_asset(event.old_path);

      if (std::filesystem::is_regular_file(event.path)) {
        FileInfo file_info;
        std::filesystem::path path(event.path);

        file_info.name = path.filename().string();
        file_info.extension = path.extension().string();
        file_info.full_path = event.path;
        file_info.relative_path = event.path;
        file_info.size = std::filesystem::file_size(event.path);
        file_info.last_modified = event.timestamp;
        file_info.is_directory = false;
        file_info.type = get_asset_type(file_info.extension);

        g_database.insert_asset(file_info);
        g_assets_updated = true;
      }
      break;
    }
    default:
      break;
  }
}

int main() {
  // Initialize database
  std::cout << "Initializing database...\n";
  if (!g_database.initialize("db/assets.db")) {
    std::cerr << "Failed to initialize database\n";
    return -1;
  }

  // Clean database before starting (as requested)
  std::cout << "Cleaning database...\n";
  g_database.clear_all_assets();

  // Create initial scan of assets directory
  std::cout << "Performing initial asset scan...\n";
  std::vector<FileInfo> initial_assets = scan_directory("assets");
  if (!initial_assets.empty()) {
    g_database.insert_assets_batch(initial_assets);
    g_assets = g_database.get_all_assets();
    g_filtered_assets = g_assets;  // Initialize filtered assets
  }

  // Start file watching
  std::cout << "Starting file watcher...\n";
  if (!g_file_watcher.start_watching("assets", on_file_event)) {
    std::cerr << "Failed to start file watcher\n";
    return -1;
  }

  std::cout << "File watcher started successfully\n";

  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }

  // Create window
  GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Asset Inventory", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync

  // Initialize Dear ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  // Disable imgui.ini file - we'll handle window positioning in code
  io.IniFilename = nullptr;

  // Ensure proper input handling for cursor blinking
  io.ConfigInputTextCursorBlink = true;

  // Load Roboto font
  load_roboto_font(io);

  // Setup light and fun theme
  Theme::setup_light_fun_theme();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  // Load default texture (generic icon)
  g_default_texture = load_texture("images/texture.png");
  if (g_default_texture == 0) {
    std::cerr << "Warning: Could not load default texture\n";
  }

  // Main loop
  double last_time = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {
    double current_time = glfwGetTime();
    io.DeltaTime = (float)(current_time - last_time);
    last_time = current_time;

    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Check if assets were updated and refresh the list
    if (g_assets_updated.exchange(false)) {
      g_assets = g_database.get_all_assets();
      // Re-apply current search filter to include new assets
      filter_assets(search_buffer);
    }

    // Create main window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Asset Inventory", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // Header
    ImGui::PushFont(io.Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.20f, 0.70f, 0.90f, 1.0f), "Asset Inventory");
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.60f, 0.60f, 0.60f, 1.0f), "v1.0.0");
    ImGui::Separator();
    ImGui::Spacing();

    // Search box - create capsule background first
    float search_x = (ImGui::GetWindowSize().x - SEARCH_BOX_WIDTH) * 0.5f;
    float search_y = ImGui::GetCursorPosY();

    // Draw capsule background
    ImVec2 capsule_min(search_x, search_y);
    ImVec2 capsule_max(search_x + SEARCH_BOX_WIDTH, search_y + SEARCH_BOX_HEIGHT);
    ImGui::GetWindowDrawList()->AddRectFilled(capsule_min, capsule_max,
                                              IM_COL32(255, 255, 255, 255),  // White background
                                              25.0f                          // Rounded corners
    );

    // Position text input inside the capsule
    ImGui::SetCursorPos(ImVec2(search_x + 20,
                               search_y + (SEARCH_BOX_HEIGHT - ImGui::GetFontSize()) * 0.5f - 4));  // Move up 2 pixels
    ImGui::PushItemWidth(SEARCH_BOX_WIDTH - 40);  // Leave space for padding

    // Remove borders from text input
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));  // Transparent background

    ImGui::InputText("##Search", search_buffer, sizeof(search_buffer), ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::PopItemWidth();

    // Filter assets in real-time as user types
    filter_assets(search_buffer);

    ImGui::Spacing();
    ImGui::Spacing();

    // Asset grid
    ImGui::BeginChild("AssetGrid", ImVec2(0, 0), true);

    // Calculate grid layout upfront since all items have the same size
    float available_width = ImGui::GetContentRegionAvail().x;
    // Add GRID_SPACING to available width since we don't need spacing after the
    // last item
    int columns = static_cast<int>((available_width + GRID_SPACING) / (THUMBNAIL_SIZE + GRID_SPACING));
    if (columns < 1) columns = 1;

    // Display filtered assets in a proper grid
    for (size_t i = 0; i < g_filtered_assets.size(); i++) {
      // Calculate grid position
      int row = static_cast<int>(i) / columns;
      int col = static_cast<int>(i) % columns;

      // Calculate absolute position for this grid item
      float x_pos = col * (THUMBNAIL_SIZE + GRID_SPACING);
      float y_pos = row * (THUMBNAIL_SIZE + GRID_SPACING);

      // Set cursor to the calculated position
      ImGui::SetCursorPos(ImVec2(x_pos, y_pos));

      ImGui::BeginGroup();

      // Get texture for this asset
      unsigned int asset_texture = get_asset_texture(g_filtered_assets[i]);

      // Calculate display size based on asset type
      ImVec2 display_size(THUMBNAIL_SIZE, THUMBNAIL_SIZE);
      if (g_filtered_assets[i].type == AssetType::Texture && asset_texture != 0) {
        // Get texture dimensions and calculate aspect-ratio-preserving size
        int width, height;
        if (get_texture_dimensions(g_filtered_assets[i].full_path, width, height)) {
          display_size = calculate_thumbnail_size(width, height, THUMBNAIL_SIZE);
        }
      }

      // Create a fixed-size container for consistent layout
      ImVec2 container_size(THUMBNAIL_SIZE,
                            THUMBNAIL_SIZE + 40.0f);  // Extra space for text
      ImVec2 container_pos = ImGui::GetCursorScreenPos();

      // Draw background for the container (same as app background)
      ImGui::GetWindowDrawList()->AddRectFilled(
          container_pos, ImVec2(container_pos.x + container_size.x, container_pos.y + container_size.y),
          BACKGROUND_COLOR);

      // Calculate position to center the image vertically in the container
      float image_y_offset = (container_size.y - display_size.y) * 0.5f;
      ImVec2 image_pos(container_pos.x, container_pos.y + image_y_offset);

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.3f));

      // Display thumbnail image
      if (asset_texture != 0) {
        ImGui::SetCursorScreenPos(image_pos);
        if (ImGui::ImageButton(("##Thumbnail" + std::to_string(i)).c_str(), (ImTextureID)(intptr_t)asset_texture,
                               display_size)) {
          std::cout << "Selected: " << g_filtered_assets[i].name << '\n';
        }
      } else {
        // Fallback: colored button if texture failed to load
        ImGui::SetCursorScreenPos(image_pos);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        if (ImGui::Button(("##Thumbnail" + std::to_string(i)).c_str(), display_size)) {
          std::cout << "Selected: " << g_filtered_assets[i].name << '\n';
        }
        ImGui::PopStyleVar();

        // Add a background to simulate thumbnail (same as app background)
        ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                                                  FALLBACK_THUMBNAIL_COLOR);
      }

      ImGui::PopStyleColor(3);

      // Position text at the bottom of the container
      ImGui::SetCursorScreenPos(ImVec2(container_pos.x, container_pos.y + THUMBNAIL_SIZE + 5.0f));

      // Asset name below thumbnail
      std::string truncated_name = truncate_filename(g_filtered_assets[i].name);
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                           (THUMBNAIL_SIZE - ImGui::CalcTextSize(truncated_name.c_str()).x) * 0.5f);
      ImGui::TextWrapped("%s", truncated_name.c_str());

      ImGui::EndGroup();
    }

    // Show message if no assets found
    if (g_filtered_assets.empty()) {
      if (g_assets.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No assets found. Add files to the 'assets' directory.");
      } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No assets match your search.");
      }
    }

    ImGui::EndChild();

    ImGui::End();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.95f, 0.97f, 1.00f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup textures
  for (auto &entry : g_texture_cache) {
    if (entry.second.is_loaded && entry.second.texture_id != 0) {
      glDeleteTextures(1, &entry.second.texture_id);
    }
  }
  g_texture_cache.clear();

  if (g_default_texture != 0) {
    glDeleteTextures(1, &g_default_texture);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  // Stop file watcher and close database
  g_file_watcher.stop_watching();
  g_database.close();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
