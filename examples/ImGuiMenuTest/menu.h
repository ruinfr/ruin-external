#pragma once

struct ImGuiIO;
#include <vector>

// -----------------------------------------------------------------------------
// Sidebar navigation (data-driven)
// -----------------------------------------------------------------------------
enum class Section { Enemy, Team, World };

struct NavItem {
    const char* label;
    const char* id;
};

struct NavSection {
    const char* header;
    std::vector<NavItem> items;
};

struct SidebarSelection {
    int section;
    int item;
};

// Builds the sidebar nav data (Enemy/Team/World + items). No allocations after first call.
const std::vector<NavSection>& GetNavSections();
int GetSidebarFlatIndex(const std::vector<NavSection>& sections, int section_index, int item_index);

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
// display_label = text shown to user (e.g. "ESP"). unique_id = stable ID per section (e.g. "enemy_esp") — never shown.
bool DrawSidebarItem(const char* display_label, const char* unique_id, bool selected);
bool DrawSidebarItemAnimated(const char* display_label, const char* unique_id, bool selected);
void DrawTopHeader(const char* icon, const char* title);

// -----------------------------------------------------------------------------
// Init & render
// -----------------------------------------------------------------------------
void InitMenuStyle();
void LoadMenuFonts(ImGuiIO& io);
void RenderMenu();

// Settings: load before first frame (sets theme immediately = no flash). Save on theme toggle.
void LoadMenuSettings();
void SaveMenuSettings();
// Release menu resources (e.g. icon texture). Call before renderer shutdown.
void ShutdownMenu();
