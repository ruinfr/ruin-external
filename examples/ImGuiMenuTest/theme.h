#pragma once

#include "imgui.h"

// -----------------------------------------------------------------------------
// Premium light theme: contrast, spacing, rounding, subtle depth.
// Call ApplyTheme() after ImGui::CreateContext() (and when DPI changes).
//
// UI element locations (for reference):
//   Window title "ImGui Menu Test"  -> main.cpp, main() [glfwCreateWindow]
//   Sidebar "Visuals / ENEMY / TEAM / WORLD" -> menu.cpp GetNavSections(), DrawSidebar()
//   Theme toggle (moon icon)        -> menu.cpp DrawThemeToggleButton(), EnsureMoonTexture()
//   Settings/gear button            -> menu.cpp DrawRailIconButton("L") from DrawIconRail()
// -----------------------------------------------------------------------------

namespace Theme {

// Central palette: one struct per mode. All custom draw colors route through this
// (ThemeLight/ThemeDark fill ImGui::GetStyle().Colors from UITheme; menu.cpp uses
// GetStyleColorU32(ImGuiCol_*) so it stays theme-driven).
struct UITheme {
    ImVec4 Text;
    ImVec4 TextDisabled;
    ImVec4 WindowBg;
    ImVec4 ChildBg;
    ImVec4 PopupBg;
    ImVec4 Border;
    ImVec4 BorderShadow;
    ImVec4 FrameBg;
    ImVec4 FrameBgHovered;
    ImVec4 FrameBgActive;
    ImVec4 MenuBarBg;           // sidebar background
    ImVec4 ScrollbarBg;
    ImVec4 ScrollbarGrab;
    ImVec4 ScrollbarGrabHovered;
    ImVec4 ScrollbarGrabActive;
    ImVec4 CheckMark;           // accent bar / sidebar selected bar
    ImVec4 SliderGrab;
    ImVec4 SliderGrabActive;
    ImVec4 Button;
    ImVec4 ButtonHovered;
    ImVec4 ButtonActive;
    ImVec4 Header;
    ImVec4 HeaderHovered;
    ImVec4 HeaderActive;
    ImVec4 Separator;
    ImVec4 SeparatorHovered;
    ImVec4 SeparatorActive;
    ImVec4 ResizeGrip;
    ImVec4 ResizeGripHovered;
    ImVec4 ResizeGripActive;
    ImVec4 Tab;
    ImVec4 TabHovered;
    ImVec4 TabActive;
    ImVec4 TabUnfocused;
    ImVec4 TabUnfocusedActive;
    ImVec4 TableHeaderBg;
    ImVec4 TableBorderStrong;
    ImVec4 TableBorderLight;
    ImVec4 TableRowBg;
    ImVec4 TableRowBgAlt;
    ImVec4 TextSelectedBg;
    ImVec4 DragDropTarget;
    ImVec4 NavHighlight;
    ImVec4 NavWindowingHighlight;
    ImVec4 NavWindowingDimBg;
    ImVec4 ModalWindowDimBg;
    ImVec4 SliderText;
};

const UITheme& GetUIThemeLight();
const UITheme& GetUIThemeDark();
void FillStyleFromTheme(const UITheme& t, ImVec4* c);

// Style constants (base values; scaled by ApplyTheme(dpi_scale))
namespace Style {
    const float kWindowRounding   = 8.f;
    const float kFrameRounding   = 6.f;   // sidebar item rounding 6-8px
    const float kPopupRounding   = 6.f;
    const float kScrollbarRounding = 6.f;
    const float kGrabRounding    = 8.f;
    const float kFramePaddingX   = 8.f;
    const float kFramePaddingY   = 6.f;
    const float kItemSpacingX    = 8.f;
    const float kItemSpacingY    = 6.f;
    const float kItemInnerSpacingX = 6.f;
    const float kItemInnerSpacingY = 4.f;
    const float kWindowPaddingX  = 0.f;
    const float kWindowPaddingY  = 0.f;
    const float kBorderSize     = 1.f;
    const float kScrollbarSize  = 10.f;
}

// Colors (light: neutral grayscale only — no warm/yellow bias; used where constants are referenced)
namespace Color {
    // Backgrounds (R=G=B or nearly)
    const unsigned int kWindowBg     = 0xFFF7F7F9;  // 0.97, 0.97, 0.98
    const unsigned int kSidebarBg    = 0xFFF0F0F2;  // 0.94, 0.94, 0.95
    const unsigned int kPanelBg      = 0xFFF2F2F3;  // 0.95, 0.95, 0.96
    const unsigned int kText         = 0xFF262629;  // 0.15, 0.15, 0.17
    const unsigned int kTextDisabled = 0xFF808085;  // 0.50, 0.50, 0.52
    const unsigned int kSectionHeader = kTextDisabled;
    const unsigned int kSidebarItemText         = kText;
    const unsigned int kSidebarItemTextSelected = kText;
    const unsigned int kSidebarHoverBg          = 0xFFE0E0E6;  // 0.88, 0.88, 0.90
    const unsigned int kSidebarSelectedBg       = 0xFFE0E6F2;  // 0.88, 0.90, 0.95 cool tint
    const unsigned int kSidebarAccentBar       = 0xFFCCCCD1;  // neutral gray accent bar
    const unsigned int kDivider                = 0xFFE0E0E6;
    const unsigned int kBorder                 = 0xFFD9D9DE;
    const unsigned int kButton                 = 0xFFE6E6EB;
    const unsigned int kButtonHovered          = 0xFFDBDBE3;
    const unsigned int kButtonActive           = 0xFFD1D1D9;
    const unsigned int kFrameBg                = 0xFFEDEDF0;
    const unsigned int kFrameBgHovered         = 0xFFE3E3E8;
    const unsigned int kFrameBgActive          = 0xFFDBDBE3;
}

// Apply the full theme and scale by dpi_scale (call after CreateContext / when DPI changes).
void ApplyTheme(float dpi_scale = 1.f);

// Two palettes + blending: fill arrays of ImGuiCol_COUNT colors (no scaling).
void ThemeLight(ImVec4* colors);
void ThemeDark(ImVec4* colors);
// Lerp light -> dark by t (t eased in caller), write into ImGui::GetStyle().Colors. Call once per frame before drawing menu.
void ApplyBlendedTheme(float t);

// Blended color at t (0=light, 1=dark). Use same t as ApplyBlendedTheme (e.g. eased) so custom draw matches.
ImVec4 GetBlendedColor(ImGuiCol_ col, float t);
// Semantic helpers so custom draw (accent bar, sidebar) matches full palette.
ImVec4 GetAccentColor(float t);
struct SidebarColors { ImVec4 header, headerHovered, headerActive, checkMark; };
SidebarColors GetSidebarColors(float t);

// Optional: push style for sidebar item (selected = true for selected state).
// Use before drawing custom sidebar item; pop after.
void PushSidebarItemStyle(bool selected);
void PopSidebarItemStyle();

} // namespace Theme
