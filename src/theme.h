#pragma once
#include "imgui.h"

namespace Theme {
// Light and fun theme for game asset management
inline void setup_light_fun_theme() {
  ImGuiStyle &style = ImGui::GetStyle();

  // Light and vibrant color scheme
  ImVec4 *colors = style.Colors;
  colors[ImGuiCol_Text] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.97f, 1.00f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.95f, 0.97f, 1.00f, 1.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.95f, 0.97f, 1.00f, 1.00f);
  colors[ImGuiCol_Border] = ImVec4(0.80f, 0.85f, 0.90f, 0.50f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.95f, 1.00f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.85f, 0.90f, 0.95f, 1.00f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.85f, 0.90f, 0.95f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.75f, 0.85f, 0.95f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.85f, 0.90f, 0.95f, 0.75f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.90f, 0.95f, 1.00f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.90f, 0.95f, 1.00f, 0.60f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.70f, 0.80f, 0.90f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.60f, 0.75f, 0.85f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.65f, 0.80f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.15f, 0.55f, 0.75f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.15f, 0.55f, 0.75f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.10f, 0.45f, 0.65f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.15f, 0.55f, 0.75f, 0.80f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.10f, 0.45f, 0.65f, 0.95f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.05f, 0.35f, 0.55f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.85f, 0.90f, 0.95f, 0.80f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.80f, 0.85f, 0.90f, 0.90f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.75f, 0.80f, 0.85f, 1.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.80f, 0.85f, 0.90f, 0.50f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.70f, 0.80f, 0.90f, 0.75f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.15f, 0.55f, 0.75f, 0.95f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.55f, 0.75f, 0.30f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.10f, 0.45f, 0.65f, 0.60f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.05f, 0.35f, 0.55f, 0.90f);
  colors[ImGuiCol_Tab] = ImVec4(0.85f, 0.90f, 0.95f, 0.80f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.80f, 0.85f, 0.90f, 0.90f);
  colors[ImGuiCol_TabActive] = ImVec4(0.75f, 0.80f, 0.85f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.90f, 0.95f, 1.00f, 0.80f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.85f, 0.90f, 0.95f, 0.90f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.15f, 0.55f, 0.75f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.10f, 0.45f, 0.65f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.15f, 0.55f, 0.75f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.10f, 0.45f, 0.65f, 1.00f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.15f, 0.55f, 0.75f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(0.15f, 0.55f, 0.75f, 0.90f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.15f, 0.55f, 0.75f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.15f, 0.55f, 0.75f, 1.00f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.15f, 0.55f, 0.75f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.15f, 0.55f, 0.75f, 1.00f);
  colors[ImGuiCol_InputTextCursor] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

  // Modern spacing and sizing
  style.WindowPadding = ImVec2(15, 15);
  style.WindowRounding = 8.0f;
  style.FramePadding = ImVec2(8, 6);
  style.FrameRounding = 6.0f;
  style.ItemSpacing = ImVec2(12, 8);
  style.ItemInnerSpacing = ImVec2(8, 6);
  style.IndentSpacing = 25.0f;
  style.ScrollbarSize = 16.0f;
  style.ScrollbarRounding = 8.0f;
  style.GrabMinSize = 6.0f;
  style.GrabRounding = 4.0f;
  style.TabRounding = 6.0f;
  style.ChildRounding = 6.0f;
  style.PopupRounding = 6.0f;
  style.FrameBorderSize = 1.0f;
  style.WindowBorderSize = 1.0f;
}
} // namespace Theme
