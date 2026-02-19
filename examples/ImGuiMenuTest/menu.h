#pragma once

struct ImGuiIO;

// -----------------------------------------------------------------------------
// Layout constants (use these instead of magic numbers)
// -----------------------------------------------------------------------------
namespace MenuLayout {
    const float kMenuWidth           = 805.f;
    const float kMenuHeight          = 575.f;
    const float kIconRailWidth       = 50.f;
    const float kSidebarWidth        = 180.f;
    const float kPreviewWidth        = 280.f;
    const float kPanelPaddingH       = 15.f;
    const float kPanelPaddingV       = 50.f;
    const float kSectionSpacingAbove = 12.f;
    const float kSectionHeaderScale  = 0.87f;
    const float kSidebarItemHeight   = 28.f;
    const float kSidebarItemSpacing  = 4.f;
    const float kAccentBarWidth      = 3.f;
    const float kDividerThickness    = 1.f;
    const float kDividerPaddingV     = 8.f;
}

// -----------------------------------------------------------------------------
// Helpers (non-clickable section header, clickable sidebar item, top icon+title)
// -----------------------------------------------------------------------------
void DrawSectionHeader(const char* label);
bool DrawSidebarItem(const char* label, bool selected);
void DrawTopHeader(const char* icon, const char* title);

// -----------------------------------------------------------------------------
// Init & render
// -----------------------------------------------------------------------------
void InitMenuStyle();
void LoadMenuFonts(ImGuiIO& io);
void RenderMenu();
