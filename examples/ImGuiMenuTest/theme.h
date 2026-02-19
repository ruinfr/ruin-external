#pragma once

// -----------------------------------------------------------------------------
// Premium light theme: contrast, spacing, rounding, subtle depth.
// Call ApplyTheme() after ImGui::CreateContext() (and when DPI changes).
// -----------------------------------------------------------------------------

namespace Theme {

// Style constants (base values; scaled by ApplyTheme(dpi_scale))
namespace Style {
    const float kWindowRounding   = 8.f;
    const float kFrameRounding   = 6.f;
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

// Colors (light theme, high contrast for text; low alpha only for disabled)
namespace Color {
    // Backgrounds (subtle depth: window lightest, then panel, then sidebar)
    const unsigned int kWindowBg     = 0xFFF8F9FC;  // main window
    const unsigned int kSidebarBg   = 0xFFF2F3F6;  // sidebar
    const unsigned int kPanelBg     = 0xFFF5F6F9;  // general/preview panels
    // Text: strong contrast
    const unsigned int kText        = 0xFF1C1E24;   // normal
    const unsigned int kTextDisabled = 0xFF9CA0A8; // disabled only (reduced contrast)
    const unsigned int kSectionHeader = 0xFF6B6F78; // structural labels
    // Sidebar items
    const unsigned int kSidebarItemText       = 0xFF2C2E36;
    const unsigned int kSidebarItemTextSelected = 0xFF1A1C22;
    const unsigned int kSidebarHoverBg        = 0xFFE8EAEF;
    const unsigned int kSidebarSelectedBg     = 0xFFE2E4EB;
    const unsigned int kSidebarAccentBar      = 0xFFC62828;  // strong accent
    // Dividers and borders
    const unsigned int kDivider   = 0xFFDEE0E5;
    const unsigned int kBorder    = 0xFFE2E4E8;
    // Buttons (accent)
    const unsigned int kButton        = 0xFFC62828;
    const unsigned int kButtonHovered = 0xFFD32F2F;
    const unsigned int kButtonActive  = 0xFFB71C1C;
    // Frames (inputs)
    const unsigned int kFrameBg        = 0xFFFFFFFF;
    const unsigned int kFrameBgHovered = 0xFFF5F6F9;
    const unsigned int kFrameBgActive  = 0xFFEEF0F4;
}

// Apply the full theme and scale by dpi_scale (call after CreateContext / when DPI changes).
void ApplyTheme(float dpi_scale = 1.f);

// Optional: push style for sidebar item (selected = true for selected state).
// Use before drawing custom sidebar item; pop after.
void PushSidebarItemStyle(bool selected);
void PopSidebarItemStyle();

} // namespace Theme
