#pragma once

// -----------------------------------------------------------------------------
// Premium light theme: contrast, spacing, rounding, subtle depth.
// Call ApplyTheme() after ImGui::CreateContext() (and when DPI changes).
// -----------------------------------------------------------------------------

namespace Theme {

// Style constants (base values; scaled by ApplyTheme(dpi_scale))
namespace Style {
    const float kWindowRounding   = 8.f;
    const float kFrameRounding   = 6.f;   // sidebar item rounding 6-8px
    const float kPopupRounding   = 6.f;
    const float kScrollbarRounding = 6.f;
    const float kGrabRounding    = 4.f;
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

// Colors (light modern: neutral base, subtle hover, single soft blue accent, no yellow)
namespace Color {
    // Backgrounds (neutral, cohesive)
    const unsigned int kWindowBg     = 0xFFF0EDED;  // base (0.93, 0.93, 0.94)
    const unsigned int kSidebarBg    = 0xFFE8E6E6;  // sidebar (0.90, 0.90, 0.91)
    const unsigned int kPanelBg     = 0xFFECEAED;  // panels (between base and sidebar)
    // Text
    const unsigned int kText        = 0xFF2B2626;  // primary (0.15, 0.15, 0.17)
    const unsigned int kTextDisabled = 0xFF7F7373; // secondary / disabled (0.45, 0.45, 0.50)
    const unsigned int kSectionHeader = 0xFF7F7373; // same as secondary
    // Sidebar: subtle hover, stronger selected, single accent
    const unsigned int kSidebarItemText         = 0xFF2B2626;  // primary text
    const unsigned int kSidebarItemTextSelected = 0xFF1F1C26;  // stronger when selected
    const unsigned int kSidebarHoverBg          = 0xFFE6DED9;  // hover (0.85, 0.87, 0.90) — slightly darken
    const unsigned int kSidebarSelectedBg      = 0xFFF2D9CC;  // selected (0.80, 0.85, 0.95) — soft blue tint
    const unsigned int kSidebarAccentBar       = 0xFFD97340;  // modern soft blue (0.25, 0.45, 0.85) — 3px left bar
    // Dividers and borders
    const unsigned int kDivider   = 0xFFE5E0DE;
    const unsigned int kBorder    = 0xFFE8E4E2;
    // Buttons (use same accent)
    const unsigned int kButton        = 0xFFD97340;
    const unsigned int kButtonHovered = 0xFFE08050;
    const unsigned int kButtonActive  = 0xFFC96838;
    // Frames (inputs)
    const unsigned int kFrameBg        = 0xFFFFFFFF;
    const unsigned int kFrameBgHovered = 0xFFF5F4F2;
    const unsigned int kFrameBgActive  = 0xFFEEEDEC;
}

// Apply the full theme and scale by dpi_scale (call after CreateContext / when DPI changes).
void ApplyTheme(float dpi_scale = 1.f);

// Optional: push style for sidebar item (selected = true for selected state).
// Use before drawing custom sidebar item; pop after.
void PushSidebarItemStyle(bool selected);
void PopSidebarItemStyle();

} // namespace Theme
