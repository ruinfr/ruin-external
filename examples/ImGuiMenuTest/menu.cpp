#include "imgui.h"
#include "imgui_internal.h"
#include "byte.h"
#include "menu.h"
#include "theme.h"
#include "texture_loader.h"
#include <cmath>
#include <map>
#include <cstdio>
#include <cstring>
#if defined(_WIN32)
#include <Windows.h>
#endif

using namespace MenuLayout;

// ---- Sidebar data + selection ----

static SidebarSelection s_sidebar = { 0, 0 };

static std::vector<NavSection> s_nav_sections;

const std::vector<NavSection>& GetNavSections()
{
    if (s_nav_sections.empty())
    {
        s_nav_sections.push_back({ "ENEMY", { {"ESP", "enemy_esp"}, {"Chams", "enemy_chams"}, {"Other", "enemy_other"} } });
        s_nav_sections.push_back({ "TEAM",  { {"ESP", "team_esp"},  {"Chams", "team_chams"},  {"Other", "team_other"} } });
        s_nav_sections.push_back({ "WORLD", { {"ESP", "world_esp"}, {"Chams", "world_chams"}, {"Other", "world_other"} } });
    }
    return s_nav_sections;
}

int GetSidebarFlatIndex(const std::vector<NavSection>& sections, int section_index, int item_index)
{
    int flat = 0;
    for (int s = 0; s < section_index && s < (int)sections.size(); s++)
        flat += (int)sections[s].items.size();
    return flat + item_index;
}

static int GetCurrentSidebarFlatIndex()
{
    const std::vector<NavSection>& sections = GetNavSections();
    return GetSidebarFlatIndex(sections, s_sidebar.section, s_sidebar.item);
}

// Animation: ~150ms hover fade, ~180ms selected slide with easing. No per-frame allocs.
static const float kHoverAnimDuration    = 0.15f;
static const float kSelectedAnimDuration = 0.18f;
static std::map<ImGuiID, float> s_hover_alpha;
static std::map<ImGuiID, float> s_selected_slide;

static float EaseOutCubic(float t)
{
    t = ImClamp(t, 0.f, 1.f);
    return 1.f - (1.f - t) * (1.f - t) * (1.f - t);
}

static ImU32 SetAlpha(ImU32 c, float alpha)
{
    unsigned int a = (unsigned int)(ImClamp(alpha, 0.f, 1.f) * 255.f) & 0xFF;
    return (c & 0x00FFFFFFu) | (a << 24);
}

// Theme-derived sidebar colors (no hardcoded yellow; from active ImGui style).
static ImU32 GetStyleColorU32(ImGuiCol idx) { return ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(idx)); }

float dpi_scale = 1.f;

ImFont* gilroy = nullptr;
ImFont* gilroy_mini = nullptr;
ImFont* iconfont = nullptr;
ImVec2 pos;
ImDrawList* draw = nullptr;

static int sliderint = 0;
static bool checkbox = false;
static int tabs = 1;
static int sett = 0;

// Dark mode: persistent state + animation scalar (0=light, 1=dark). No theme colors changed yet.
static bool  g_DarkMode = false;
static float g_ThemeT   = 0.0f;

// Moon icon for theme toggle. Loaded once; 0 if load failed (use fallback button).
static ImTextureID g_MoonTex = (ImTextureID)0;
static bool        g_MoonTexTried = false;

// ---- Path helpers (executable directory for reliable settings + icon paths) ----
static void GetExeDir(char* out, int size)
{
    if (!out || size <= 0) return;
    out[0] = '\0';
#if defined(_WIN32)
    if (GetModuleFileNameA(NULL, out, size) == 0)
        return;
    char* last = std::strrchr(out, '\\');
    if (last)
        last[1] = '\0';
    else
        out[0] = '\0';
#else
    (void)size;
    std::snprintf(out, 512, ".");
#endif
}

static void GetSettingsFilePath(char* out, int size)
{
    if (!out || size <= 0) return;
    GetExeDir(out, size);
    if (!out[0]) return;
    std::strncat(out, "settings.ini", size - (int)std::strlen(out) - 1);
}

// Load theme from settings.ini next to exe; set g_DarkMode and g_ThemeT immediately to avoid flash. No crash if file missing.
static void LoadSettings()
{
    char path[512];
    GetSettingsFilePath(path, sizeof(path));
    if (!path[0]) return;
    FILE* f = std::fopen(path, "r");
    if (!f) return;
    char line[64];
    while (std::fgets(line, sizeof(line), f))
    {
        int dark = -1;
        if (std::sscanf(line, " dark_mode = %d", &dark) == 1 ||
            std::sscanf(line, "dark_mode=%d", &dark) == 1 ||
            std::sscanf(line, " dark = %d", &dark) == 1 ||
            std::sscanf(line, "dark=%d", &dark) == 1)
        {
            g_DarkMode = (dark != 0);
            g_ThemeT = g_DarkMode ? 1.0f : 0.0f;
            break;
        }
    }
    std::fclose(f);
}

// Persist theme choice to settings.ini next to executable. No crash if write fails.
static void SaveSettings()
{
    char path[512];
    GetSettingsFilePath(path, sizeof(path));
    if (!path[0]) return;
    FILE* f = std::fopen(path, "w");
    if (!f) return;
    std::fprintf(f, "[theme]\ndark_mode=%d\ndark=%d\n", g_DarkMode ? 1 : 0, g_DarkMode ? 1 : 0);
    std::fclose(f);
}

void LoadMenuSettings()
{
    LoadSettings();
}

void SaveMenuSettings()
{
    SaveSettings();
}

void ShutdownMenu()
{
    if (g_MoonTex != (ImTextureID)0)
    {
        ReleaseTextureOpenGL(g_MoonTex);
        g_MoonTex = (ImTextureID)0;
    }
}

// Load moon icon once; prefer exe-dir icons/ then fallbacks. Never crashes; fallback to "🌙" if missing.
static void EnsureMoonTexture()
{
    if (g_MoonTexTried)
        return;
    g_MoonTexTried = true;
    char buf[512];
    char exeDir[512];
    GetExeDir(exeDir, sizeof(exeDir));
    if (exeDir[0])
    {
        std::snprintf(buf, sizeof(buf), "%sicons\\moon.png", exeDir);
        g_MoonTex = LoadTextureFromFileOpenGL(buf);
        if (g_MoonTex == (ImTextureID)0)
        {
            std::snprintf(buf, sizeof(buf), "%sicons/moon.png", exeDir);
            g_MoonTex = LoadTextureFromFileOpenGL(buf);
        }
    }
    if (g_MoonTex == (ImTextureID)0)
    {
        const char* fallbacks[] = { "icons/moon.png", "../../../../icons/moon.png", "../../../icons/moon.png" };
        for (const char* p : fallbacks)
        {
            g_MoonTex = LoadTextureFromFileOpenGL(p);
            if (g_MoonTex != (ImTextureID)0) break;
        }
    }
}

// ---- Helpers ----

void DrawSectionHeader(const char* label)
{
    ImGui::Spacing();
    ImGui::Dummy(ImVec2(0, kSectionSpacingAbove));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::SetWindowFontScale(kSectionHeaderScale);
    ImGui::TextUnformatted(label);
    ImGui::SetWindowFontScale(1.f);
    ImGui::PopStyleColor();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    dl->AddLine(ImVec2(p.x, p.y), ImVec2(p.x + w, p.y), GetStyleColorU32(ImGuiCol_Separator), kDividerThickness);
    ImGui::Dummy(ImVec2(0, kDividerPaddingV));
}

bool DrawSidebarItem(const char* display_label, const char* unique_id, bool selected)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(unique_id);
    const ImVec2 label_size = ImGui::CalcTextSize(display_label, NULL, true);
    float avail_w = ImGui::GetContentRegionAvail().x;
    ImVec2 size(avail_w, kSidebarItemHeight);
    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, 0.f);
    if (!ImGui::ItemAdd(bb, id))
        return false;
    bool hovered = false, held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float rounding = Theme::Style::kFrameRounding;
    // Theme-derived: selected = HeaderActive, hover = FrameBgHovered, accent = CheckMark, text = Text (readable in both modes)
    if (selected)
        dl->AddRectFilled(bb.Min, bb.Max, GetStyleColorU32(ImGuiCol_HeaderActive), rounding);
    if (hovered && !selected)
        dl->AddRectFilled(bb.Min, bb.Max, GetStyleColorU32(ImGuiCol_FrameBgHovered), rounding);
    if (selected)
        dl->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x + kAccentBarWidth, bb.Max.y), GetStyleColorU32(ImGuiCol_CheckMark), 0.f);
    ImVec2 text_pos(bb.Min.x + style.FramePadding.x + (selected ? kAccentBarWidth : 0.f), bb.Min.y + (kSidebarItemHeight - label_size.y) * 0.5f);
    dl->AddText(text_pos, GetStyleColorU32(ImGuiCol_Text), display_label);
    return pressed;
}

bool DrawSidebarItemAnimated(const char* display_label, const char* unique_id, bool selected)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(unique_id);
    const ImVec2 label_size = ImGui::CalcTextSize(display_label, NULL, true);
    float avail_w = ImGui::GetContentRegionAvail().x;
    ImVec2 size(avail_w, kSidebarItemHeight);
    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, 0.f);
    if (!ImGui::ItemAdd(bb, id))
        return false;
    bool hovered = false, held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

    float dt = ImGui::GetIO().DeltaTime;
    float hover_speed = 1.f / kHoverAnimDuration;
    float slide_speed = 1.f / kSelectedAnimDuration;
    float& ha = s_hover_alpha[id];
    float& sl = s_selected_slide[id];
    ha += (hovered ? 1.f : -1.f) * hover_speed * dt;
    ha = ImClamp(ha, 0.f, 1.f);
    sl += (selected ? 1.f : -1.f) * slide_speed * dt;
    sl = ImClamp(sl, 0.f, 1.f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float rounding = Theme::Style::kFrameRounding;

    // Single background per item (no stacked alpha): selected uses HeaderActive, hover uses FrameBgHovered.
    if (sl > 0.001f)
    {
        ImU32 selected_bg = SetAlpha(GetStyleColorU32(ImGuiCol_HeaderActive), sl);
        dl->AddRectFilled(bb.Min, bb.Max, selected_bg, rounding);
    }
    else if (ha > 0.001f)
    {
        ImU32 hover_bg = SetAlpha(GetStyleColorU32(ImGuiCol_FrameBgHovered), ha);
        dl->AddRectFilled(bb.Min, bb.Max, hover_bg, rounding);
    }

    if (sl > 0.001f)
    {
        float bar_w = kAccentBarWidth * EaseOutCubic(sl);
        dl->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x + bar_w, bb.Max.y), GetStyleColorU32(ImGuiCol_CheckMark), 0.f);
    }

    float bar_vis = EaseOutCubic(sl);
    float text_offset = bar_vis * kAccentBarWidth;
    ImVec2 text_pos(bb.Min.x + style.FramePadding.x + text_offset, bb.Min.y + (kSidebarItemHeight - label_size.y) * 0.5f);
    dl->AddText(text_pos, GetStyleColorU32(ImGuiCol_Text), display_label);
    return pressed;
}

void DrawTopHeader(const char* icon, const char* title)
{
    if (icon && iconfont)
    {
        ImGui::PushFont(iconfont);
        ImGui::TextUnformatted(icon);
        ImGui::PopFont();
    }
    if (title && *title)
    {
        ImGui::SameLine();
        ImGui::SetWindowFontScale(0.95f);
        ImGui::TextUnformatted(title);
        ImGui::SetWindowFontScale(1.f);
    }
}

// ---- Layout regions ----

// Theme-aware rail tab (replaces ImGui::tab which uses hardcoded light colors in core).
static bool DrawRailTab(const char* label, bool selected)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    const ImGuiStyle& style = ImGui::GetCurrentContext()->Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const ImVec2 size = ImGui::CalcItemSize(ImVec2(15, 15), label_size.x + style.FramePadding.x * 2.f, label_size.y + style.FramePadding.y * 2.f);
    ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;
    bool hovered = false, held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float rounding = 4.f;
    ImU32 fill = GetStyleColorU32(held && hovered ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImU32 border_col = GetStyleColorU32(ImGuiCol_Border);
    dl->AddRectFilled(bb.Min, bb.Max, fill, rounding);
    dl->AddRect(bb.Min, bb.Max, border_col, rounding);
    if (selected)
        dl->AddLine(ImVec2(bb.Max.x + 7, bb.Min.y + 3), ImVec2(bb.Max.x + 7, bb.Max.y - 3), GetStyleColorU32(ImGuiCol_CheckMark), 2.f);
    return pressed;
}

// Theme-aware rail icon button (replaces ImGui::settingsbutton; uses FrameBg + Text from theme).
static bool DrawRailIconButton(const char* label)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    const ImGuiStyle& style = ImGui::GetCurrentContext()->Style;
    const ImGuiID id = window->GetID(label);
    if (!iconfont) return false;
    ImGui::PushFont(iconfont);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImGui::PopFont();
    const ImVec2 size = ImGui::CalcItemSize(ImVec2(15, 15), label_size.x + style.FramePadding.x * 2.f, label_size.y + style.FramePadding.y * 2.f);
    ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;
    bool hovered = false, held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float rounding = 4.f;
    ImU32 fill = GetStyleColorU32(held && hovered ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    dl->AddRectFilled(bb.Min, bb.Max, fill, rounding);
    ImVec2 text_pos(bb.Min.x + size.x * 0.5f - label_size.x * 0.5f, bb.Min.y + size.y * 0.5f - label_size.y * 0.5f);
    dl->AddText(iconfont, iconfont->FontSize, text_pos, GetStyleColorU32(ImGuiCol_Text), label);
    return pressed;
}

static void DrawIconRail()
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
    ImGui::BeginChild("##icon_rail", ImVec2(kIconRailWidth, -1), false, 0);
    ImGui::SetCursorPos(ImVec2(kPanelPaddingH - 2.f, 7.f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));
    DrawTopHeader("K", nullptr);
    ImGui::PopStyleColor();
    ImGui::SetCursorPos(ImVec2(17.f, 75.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 16));
    ImGui::BeginGroup();
    if (DrawRailTab("A", 0 == tabs)) tabs = 0;
    if (DrawRailTab("B", 1 == tabs)) tabs = 1;
    if (DrawRailTab("C", 2 == tabs)) tabs = 2;
    if (DrawRailTab("D", 3 == tabs)) tabs = 3;
    if (DrawRailTab("E", 4 == tabs)) tabs = 4;
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40.f);
    if (DrawRailIconButton("L")) sett = 1;
    ImGui::EndGroup();
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

static void DrawSidebar()
{
    if (tabs != 1)
        return;
    const std::vector<NavSection>& sections = GetNavSections();
    ImGui::SameLine(0, 0);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
    ImGui::BeginChild("Visuals", ImVec2(kSidebarWidth, -1), false, 0);
    ImGui::SetCursorPos(ImVec2(kPanelPaddingH, kPanelPaddingV));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, kSidebarItemSpacing));
    for (int si = 0; si < (int)sections.size(); si++)
    {
        const NavSection& sec = sections[si];
        DrawSectionHeader(sec.header);
        for (int ii = 0; ii < (int)sec.items.size(); ii++)
        {
            const NavItem& it = sec.items[ii];
            bool selected = (s_sidebar.section == si && s_sidebar.item == ii);
            if (DrawSidebarItemAnimated(it.label, it.id, selected))
            {
                s_sidebar.section = si;
                s_sidebar.item = ii;
            }
        }
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

static void DrawVerticalDivider()
{
    ImGui::SameLine(0, 0);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float h = ImGui::GetWindowHeight();
    dl->AddLine(ImVec2(p.x, p.y), ImVec2(p.x, p.y + h), GetStyleColorU32(ImGuiCol_Separator), kDividerThickness);
    ImGui::Dummy(ImVec2(kDividerThickness + 2.f, 0));
}

static void DrawGeneralPanel()
{
    if (tabs != 1)
        return;
    DrawVerticalDivider();
    ImGui::SameLine(0, 0);
    float general_w = ImGui::GetContentRegionAvail().x - kPreviewWidth - 4.f;
    if (general_w < 200.f)
        general_w = 200.f;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));
    ImGui::BeginChild("General", ImVec2(general_w, -1), false, 0);
    ImGui::SetCursorPos(ImVec2(kPanelPaddingH, kPanelPaddingV));
    ImGui::BeginGroup();
    ImGui::Checkbox("Checkbox", &checkbox);
    ImGui::SliderInt("SliderInt", &sliderint, 0, 100);
    ImGui::Button("Button", ImVec2(220, 30));
    ImGui::EndGroup();
    ImGui::EndChild();
}

static void DrawPreviewPanel()
{
    if (tabs != 1)
        return;
    DrawVerticalDivider();
    ImGui::SameLine(0, 0);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));
    ImGui::BeginChild("Preview", ImVec2(kPreviewWidth, -1), false, 0);
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

static void DrawMenuBackground()
{
    pos = ImGui::GetWindowPos();
    draw = ImGui::GetWindowDrawList();
    float w = kMenuWidth * dpi_scale;
    float h = kMenuHeight * dpi_scale;
    float rounding = Theme::Style::kWindowRounding;
    draw->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), GetStyleColorU32(ImGuiCol_WindowBg), rounding);
}

// Theme toggle: InvisibleButton + custom draw. Default transparent, hover/active use theme (no blue block). Icon tint = Text (dark in light, light in dark).
static void DrawThemeToggleButton()
{
    const float btn_size = 24.f;
    const float margin = 8.f;
    float win_w = ImGui::GetWindowWidth();
    ImGui::SetCursorPos(ImVec2(win_w - btn_size - margin, margin));

    EnsureMoonTexture();

    ImGui::PushID("##theme_toggle");
    ImGui::InvisibleButton("##theme_toggle_btn", ImVec2(btn_size, btn_size));
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    bool clicked = ImGui::IsItemClicked(0);

    ImVec2 mn = ImGui::GetItemRectMin();
    ImVec2 mx = ImGui::GetItemRectMax();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float rounding = Theme::Style::kFrameRounding;

    if (active)
        dl->AddRectFilled(mn, mx, GetStyleColorU32(ImGuiCol_FrameBgActive), rounding);
    else if (hovered)
        dl->AddRectFilled(mn, mx, GetStyleColorU32(ImGuiCol_FrameBgHovered), rounding);

    ImU32 icon_tint = GetStyleColorU32(ImGuiCol_Text);
    if (g_MoonTex != (ImTextureID)0)
        dl->AddImage(g_MoonTex, mn, mx, ImVec2(0, 0), ImVec2(1, 1), icon_tint);
    else
    {
        const char* fallback = "🌙";
        ImVec2 ts = ImGui::CalcTextSize(fallback);
        ImVec2 pos(mn.x + (btn_size - ts.x) * 0.5f, mn.y + (btn_size - ts.y) * 0.5f);
        dl->AddText(pos, icon_tint, fallback);
    }

    ImGui::PopID();

    if (hovered)
        ImGui::SetTooltip("Toggle theme");
    if (clicked)
    {
        g_DarkMode = !g_DarkMode;
        SaveMenuSettings();
    }
}

// ---- Public API ----

void InitMenuStyle()
{
    Theme::ApplyTheme(dpi_scale);
}

void LoadMenuFonts(ImGuiIO& io)
{
    ImFontConfig font_config;
    font_config.OversampleH = 1;
    font_config.OversampleV = 1;
    font_config.PixelSnapH = 1;
    static const ImWchar ranges[] = { 0x0020, 0x00FF, 0x0400, 0x044F, 0 };
    gilroy_mini = io.Fonts->AddFontFromMemoryTTF((void*)gilroyfont, sizeof(gilroyfont), 13, &font_config, ranges);
    iconfont = io.Fonts->AddFontFromMemoryTTF((void*)icon, sizeof(icon), 30, &font_config, ranges);
    gilroy = io.Fonts->AddFontFromMemoryTTF((void*)gilroyfont, sizeof(gilroyfont), 17, &font_config, ranges);
    io.Fonts->Build();
}

// Single theme apply per frame: update g_ThemeT from g_DarkMode, then ApplyBlendedTheme. Call after NewFrame(), before RenderMenu().
void ApplyMenuTheme()
{
    const float kThemeSpeed = 10.0f;
    float dt = ImGui::GetIO().DeltaTime;
    float target = g_DarkMode ? 1.0f : 0.0f;
    float step = kThemeSpeed * dt;
    if (g_ThemeT < target) { g_ThemeT = ImMin(g_ThemeT + step, target); }
    else if (g_ThemeT > target) { g_ThemeT = ImMax(g_ThemeT - step, target); }
    g_ThemeT = ImClamp(g_ThemeT, 0.0f, 1.0f);
    Theme::ApplyBlendedTheme(g_ThemeT);
}

void RenderMenu()
{
    float w = kMenuWidth * dpi_scale;
    float h = kMenuHeight * dpi_scale;
    ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::SetWindowSize(ImVec2(ImFloor(w), ImFloor(h)));
    DrawMenuBackground();
    DrawThemeToggleButton();
    DrawIconRail();
    DrawVerticalDivider();
    DrawSidebar();
    DrawGeneralPanel();
    DrawPreviewPanel();
    ImGui::End();
}
