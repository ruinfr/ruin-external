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
#include <chrono>
#if defined(_WIN32)
#include <Windows.h>
#endif

using namespace MenuLayout;

// #region agent log
void DebugLog(const char* location, const char* message, const char* step)
{
    FILE* f = std::fopen("c:\\Users\\cobra\\Downloads\\RUIN EXTERNAL\\debug-20d588.log", "a");
    if (!f) return;
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::fprintf(f, "{\"sessionId\":\"20d588\",\"location\":\"%s\",\"message\":\"%s\",\"data\":{\"step\":\"%s\"},\"timestamp\":%lld}\n", location, message, step, (long long)ms);
    std::fclose(f);
}
// #endregion

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

// EaseInOutCubic: smooth start and end for theme transition (no linear feel).
static float EaseInOutCubic(float t)
{
    t = ImClamp(t, 0.f, 1.f);
    return t <= 0.5f ? 4.f * t * t * t : 1.f - (-2.f * t + 2.f) * (-2.f * t + 2.f) * (-2.f * t + 2.f) * 0.5f;
}

// Move towards target at constant rate; stable at any FPS, no overshoot.
static float MoveTowards(float current, float target, float max_delta)
{
    if (current < target) return ImMin(current + max_delta, target);
    if (current > target) return ImMax(current - max_delta, target);
    return target;
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

// Tab state: Visual / Aim / Whitelist (one active at a time).
enum class Tab { Visual, Aim, Whitelist };
static Tab currentTab = Tab::Visual;

// Sidebar icon textures (loaded once at init, released in ShutdownMenu).
static ImTextureID iconVisual = (ImTextureID)0;
static ImTextureID iconAim = (ImTextureID)0;
static ImTextureID iconWhitelist = (ImTextureID)0;
static bool g_SidebarIconsTried = false;

// Persistent slider value (not reinitialized per frame); used by General panel.
static int sliderValue = 0;
static bool checkbox = false;
static int sett = 0;

// Clean slider using SliderBehavior + theme colors; no dots (avoids custom SliderScalar in imgui_widgets).
// Must activate the slider on click/focus (SetActiveID) so SliderBehavior can update value — same as SliderScalar.
static bool ThemedSliderInt(const char* label, int* v, int v_min, int v_max)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const ImVec2 cursor = window->DC.CursorPos;
    const ImRect frame_bb(cursor, ImVec2(cursor.x + 220.f, cursor.y + 30.f));
    const ImRect total_bb(ImVec2(frame_bb.Min.x + 70, frame_bb.Min.y), ImVec2(frame_bb.Max.x, frame_bb.Max.y));

    const bool hovered = ImGui::ItemHoverable(frame_bb, id);
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id, &frame_bb))
        return false;

    const bool focus_requested = ImGui::FocusableItemRegister(window, id);
    const bool clicked = (hovered && g.IO.MouseClicked[0]);
    if (focus_requested || clicked || g.NavActivateId == id || g.NavInputId == id)
    {
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    ImRect grab_bb;
    const bool value_changed = ImGui::SliderBehavior(total_bb, id, ImGuiDataType_S32, v, &v_min, &v_max, "%d", 0, &grab_bb);
    if (value_changed)
        ImGui::MarkItemEdited(id);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float frame_r = style.FrameRounding;
    const float grab_h = grab_bb.Max.y - grab_bb.Min.y;
    const float grab_r = ImMin(style.GrabRounding, grab_h * 0.5f);  // pill: rounding up to half height
    const ImRect track_rect(ImVec2(total_bb.Min.x, frame_bb.Min.y + 11), ImVec2(total_bb.Max.x, frame_bb.Max.y - 11));
    dl->AddRectFilled(track_rect.Min, track_rect.Max, GetStyleColorU32(ImGuiCol_FrameBg), frame_r);

    const bool grab_active = (g.ActiveId == id);
    ImU32 grab_col = GetStyleColorU32(grab_active ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab);
    dl->AddRectFilled(grab_bb.Min, grab_bb.Max, grab_col, grab_r);

    char value_buf[32];
    std::snprintf(value_buf, sizeof(value_buf), "%d", *v);
    const ImRect value_rect(frame_bb.Min, ImVec2(frame_bb.Min.x + 45, frame_bb.Max.y));
    ImGui::RenderTextClipped(value_rect.Min, value_rect.Max, value_buf, value_buf + std::strlen(value_buf), NULL, ImVec2(0.5f, 0.5f));
    if (label_size.x > 0.0f)
        ImGui::RenderText(ImVec2(frame_bb.Min.x + 25, frame_bb.Min.y - 20), label);

    return value_changed;
}

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
    static bool g_ShutdownDone = false;
    if (g_ShutdownDone)
        return;
    g_ShutdownDone = true;

    if (g_MoonTex != (ImTextureID)0)
    {
        ReleaseTextureOpenGL(g_MoonTex);
        g_MoonTex = (ImTextureID)0;
    }
    if (iconVisual) { ReleaseTextureOpenGL(iconVisual); iconVisual = (ImTextureID)0; }
    if (iconAim) { ReleaseTextureOpenGL(iconAim); iconAim = (ImTextureID)0; }
    if (iconWhitelist) { ReleaseTextureOpenGL(iconWhitelist); iconWhitelist = (ImTextureID)0; }
}

// Load moon icon once; prefer "moon (1).png" then "moon.png". Exe-dir then fallbacks. No crash if missing.
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
        std::snprintf(buf, sizeof(buf), "%sicons\\moon (1).png", exeDir);
        g_MoonTex = LoadTextureFromFileOpenGL(buf);
        if (g_MoonTex == (ImTextureID)0)
        {
            std::snprintf(buf, sizeof(buf), "%sicons/moon (1).png", exeDir);
            g_MoonTex = LoadTextureFromFileOpenGL(buf);
        }
        if (g_MoonTex == (ImTextureID)0)
        {
            std::snprintf(buf, sizeof(buf), "%sicons\\moon.png", exeDir);
            g_MoonTex = LoadTextureFromFileOpenGL(buf);
        }
        if (g_MoonTex == (ImTextureID)0)
        {
            std::snprintf(buf, sizeof(buf), "%sicons/moon.png", exeDir);
            g_MoonTex = LoadTextureFromFileOpenGL(buf);
        }
    }
    if (g_MoonTex == (ImTextureID)0)
    {
        const char* fallbacks[] = { "icons/moon (1).png", "icons/moon.png", "../../../../icons/moon (1).png", "../../../../icons/moon.png", "../../../icons/moon.png" };
        for (const char* p : fallbacks)
        {
            g_MoonTex = LoadTextureFromFileOpenGL(p);
            if (g_MoonTex != (ImTextureID)0) break;
        }
    }
}

// Load sidebar icons once (Visual, Aim, Whitelist). Prefer exe-dir icons/ then fallbacks. PNG only; no per-frame load.
static void EnsureSidebarIcons()
{
    if (g_SidebarIconsTried)
        return;
    g_SidebarIconsTried = true;
    char buf[512];
    char exeDir[512];
    GetExeDir(exeDir, sizeof(exeDir));
    const char* names[] = { "visuual.png", "aim.png", "whitelist.png" };
    ImTextureID* targets[] = { &iconVisual, &iconAim, &iconWhitelist };
    for (int i = 0; i < 3; i++)
    {
        if (exeDir[0])
        {
            std::snprintf(buf, sizeof(buf), "%sicons\\%s", exeDir, names[i]);
            *targets[i] = LoadTextureFromFile(buf, nullptr, nullptr);
            if (*targets[i] == (ImTextureID)0)
            {
                std::snprintf(buf, sizeof(buf), "%sicons/%s", exeDir, names[i]);
                *targets[i] = LoadTextureFromFile(buf, nullptr, nullptr);
            }
        }
        if (*targets[i] == (ImTextureID)0)
        {
            char fallback[64];
            std::snprintf(fallback, sizeof(fallback), "icons/%s", names[i]);
            *targets[i] = LoadTextureFromFile(fallback, nullptr, nullptr);
        }
    }
}

// ---- Helpers ----

// Section header color: TextDisabled-like but readable in dark mode (lerp toward Text).
static ImVec4 GetSectionHeaderColor()
{
    ImVec4 td = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
    ImVec4 t = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    const float blend = 0.45f;
    return ImVec4(
        td.x + (t.x - td.x) * blend,
        td.y + (t.y - td.y) * blend,
        td.z + (t.z - td.z) * blend,
        1.0f);
}

void DrawSectionHeader(const char* label)
{
    ImGui::Spacing();
    ImGui::Dummy(ImVec2(0, kSectionSpacingAbove));
    ImGui::PushStyleColor(ImGuiCol_Text, GetSectionHeaderColor());
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
    // Selected: subtle tinted bg + accent bar. Hover: slight lift only (never when selected).
    if (selected)
        dl->AddRectFilled(bb.Min, bb.Max, GetStyleColorU32(ImGuiCol_HeaderActive), rounding);
    if (hovered && !selected)
        dl->AddRectFilled(bb.Min, bb.Max, GetStyleColorU32(ImGuiCol_FrameBgHovered), rounding);
    if (selected)
        dl->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x + kAccentBarWidth, bb.Max.y), GetStyleColorU32(ImGuiCol_CheckMark), 0.f);
    float text_offset = selected ? kAccentBarWidth : 0.f;
    ImVec2 text_pos(bb.Min.x + style.FramePadding.x + text_offset, bb.Min.y + (kSidebarItemHeight - label_size.y) * 0.5f);
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

    // Selected: subtle tinted bg + accent bar. Hover: slight lift only (drawn only when not selected).
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

// Theme colors for icon rail (no yellow, no white on hover): dark and light palettes, blended by g_ThemeT.
static void GetIconRailColors(ImVec4* out_button, ImVec4* out_hover, ImVec4* out_active, ImVec4* out_tint)
{
    const ImVec4 dark_btn(0.16f, 0.17f, 0.19f, 1.0f);
    const ImVec4 dark_hover(0.19f, 0.20f, 0.22f, 1.0f);
    const ImVec4 dark_active(0.20f, 0.21f, 0.26f, 1.0f);
    const ImVec4 light_btn(0.92f, 0.93f, 0.95f, 1.0f);
    const ImVec4 light_hover(0.88f, 0.89f, 0.91f, 1.0f);
    const ImVec4 light_active(0.82f, 0.85f, 0.92f, 1.0f);
    const ImVec4 dark_tint(1.0f, 1.0f, 1.0f, 1.0f);
    const ImVec4 light_tint(0.15f, 0.15f, 0.18f, 1.0f);
    float t = g_ThemeT;
    if (out_button)
        *out_button = ImVec4(dark_btn.x + (light_btn.x - dark_btn.x) * (1.f - t), dark_btn.y + (light_btn.y - dark_btn.y) * (1.f - t), dark_btn.z + (light_btn.z - dark_btn.z) * (1.f - t), 1.f);
    if (out_hover)
        *out_hover = ImVec4(dark_hover.x + (light_hover.x - dark_hover.x) * (1.f - t), dark_hover.y + (light_hover.y - dark_hover.y) * (1.f - t), dark_hover.z + (light_hover.z - dark_hover.z) * (1.f - t), 1.f);
    if (out_active)
        *out_active = ImVec4(dark_active.x + (light_active.x - dark_active.x) * (1.f - t), dark_active.y + (light_active.y - dark_active.y) * (1.f - t), dark_active.z + (light_active.z - dark_active.z) * (1.f - t), 1.f);
    if (out_tint)
        *out_tint = ImVec4(dark_tint.x + (light_tint.x - dark_tint.x) * (1.f - t), dark_tint.y + (light_tint.y - dark_tint.y) * (1.f - t), dark_tint.z + (light_tint.z - dark_tint.z) * (1.f - t), 1.f);
}

// Icon tab: InvisibleButton + custom draw. Rounded hover/active only (theme FrameBgHovered/FrameBgActive), no permanent box.
// UVs (0,1)-(1,0) so icons are not upside down with OpenGL texture coords.
static bool DrawIconTab(const char* id, ImTextureID tex, bool selected, ImVec4 tint, const char* fallbackLabel)
{
    const float hitSize = 32.f;
    const float iconSize = 24.f;
    const float rounding = 8.f;

    ImGui::InvisibleButton(id, ImVec2(hitSize, hitSize));
    const bool pressed = ImGui::IsItemClicked(0);
    const bool hovered = ImGui::IsItemHovered();
    const bool active = ImGui::IsItemActive();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 mn = ImGui::GetItemRectMin();
    ImVec2 mx = ImGui::GetItemRectMax();

    if (selected || active)
        dl->AddRectFilled(mn, mx, GetStyleColorU32(ImGuiCol_FrameBgActive), rounding);
    else if (hovered)
        dl->AddRectFilled(mn, mx, GetStyleColorU32(ImGuiCol_FrameBgHovered), rounding);

    ImVec2 iconMin(mn.x + (hitSize - iconSize) * 0.5f, mn.y + (hitSize - iconSize) * 0.5f);
    ImVec2 iconMax(iconMin.x + iconSize, iconMin.y + iconSize);
    ImU32 tintU32 = ImGui::ColorConvertFloat4ToU32(tint);

    if (tex)
        dl->AddImage(tex, iconMin, iconMax, ImVec2(0, 1), ImVec2(1, 0), tintU32);
    else if (fallbackLabel && fallbackLabel[0])
    {
        ImVec2 labelSize = ImGui::CalcTextSize(fallbackLabel);
        ImVec2 textPos(iconMin.x + (iconSize - labelSize.x) * 0.5f, iconMin.y + (iconSize - labelSize.y) * 0.5f);
        dl->AddText(textPos, tintU32, fallbackLabel);
    }

    return pressed;
}

// Settings gear: InvisibleButton + theme bg only on hover/active (like moon button). No gray square when idle.
static bool DrawSettingsGearButton()
{
    const ImVec2 size(22.f, 22.f);
    ImGui::InvisibleButton("##settings", size);
    const bool pressed = ImGui::IsItemClicked(0);
    const bool hovered = ImGui::IsItemHovered();
    const bool active = ImGui::IsItemActive();

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetItemRectMin();
    ImVec2 p2 = ImGui::GetItemRectMax();

    // Like moon: no bg when idle; subtle hover/active only (theme-driven).
    if (active)
        draw->AddRectFilled(p, p2, GetStyleColorU32(ImGuiCol_FrameBgActive), 6.0f);
    else if (hovered)
        draw->AddRectFilled(p, p2, GetStyleColorU32(ImGuiCol_FrameBgHovered), 6.0f);

    // Gear icon centered (icon font "L")
    if (iconfont)
    {
        const char* gear = "L";
        ImGui::PushFont(iconfont);
        ImVec2 label_size = ImGui::CalcTextSize(gear, NULL, true);
        ImGui::PopFont();
        ImVec2 text_pos(p.x + (size.x - label_size.x) * 0.5f, p.y + (size.y - label_size.y) * 0.5f);
        draw->AddText(iconfont, iconfont->FontSize, text_pos, GetStyleColorU32(ImGuiCol_Text), gear);
    }

    return pressed;
}

static void DrawIconRail()
{
    // Sidebar BG: dark (0.12,0.13,0.15) / light (0.94,0.95,0.97)
    float t = g_ThemeT;
    ImVec4 sidebarBg(
        0.12f + (0.94f - 0.12f) * (1.f - t),
        0.13f + (0.95f - 0.13f) * (1.f - t),
        0.15f + (0.97f - 0.15f) * (1.f - t),
        1.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, sidebarBg);
    ImGui::BeginChild("##icon_rail", ImVec2(kIconRailWidth, -1), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPos(ImVec2((kIconRailWidth - 24.f) * 0.5f, 7.f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));
    DrawTopHeader("K", nullptr);
    ImGui::PopStyleColor();

    EnsureSidebarIcons();
    ImVec4 tintCol;
    GetIconRailColors(nullptr, nullptr, nullptr, &tintCol);

    const float tabHitSize = 32.f;
    const float spacing = 10.f;
    const float railPad = (kIconRailWidth - tabHitSize) * 0.5f;
    ImGui::SetCursorPos(ImVec2(railPad, 55.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, spacing));
    ImGui::BeginGroup();

    if (DrawIconTab("##tab_visual", iconVisual, currentTab == Tab::Visual, tintCol, "V"))
        currentTab = Tab::Visual;
    if (DrawIconTab("##tab_aim", iconAim, currentTab == Tab::Aim, tintCol, "A"))
        currentTab = Tab::Aim;
    if (DrawIconTab("##tab_whitelist", iconWhitelist, currentTab == Tab::Whitelist, tintCol, "W"))
        currentTab = Tab::Whitelist;

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40.f);
    if (DrawSettingsGearButton())
        sett = 1;
    if (sett == 1)
    {
        ImGui::OpenPopup("##settings_popup");
        ImVec2 gear_min = ImGui::GetItemRectMin();
        ImVec2 gear_max = ImGui::GetItemRectMax();
        ImGui::SetNextWindowPos(ImVec2(gear_min.x, gear_max.y + 2.f), ImGuiCond_Appearing, ImVec2(0.f, 0.f));
        sett = 2;
    }
    if (ImGui::BeginPopup("##settings_popup"))
    {
        if (sett == 2)
            sett = 0;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.f, 10.f));
        ImGui::TextUnformatted("Settings");
        ImGui::Separator();
        ImGui::TextUnformatted("Theme and options here.");
        ImGui::PopStyleVar();
        ImGui::EndPopup();
    }
    else if (sett == 2)
        sett = 0;
    ImGui::EndGroup();
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

static void DrawSidebar()
{
    if (currentTab != Tab::Visual)
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
    if (currentTab != Tab::Visual)
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
    {
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ThemedSliderInt("SliderInt", &sliderValue, 0, 100);
        ImGui::PopStyleVar(2);
    }
    ImGui::Button("Button", ImVec2(220, 30));
    ImGui::EndGroup();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

static void DrawPreviewPanel()
{
    if (currentTab != Tab::Visual)
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

// Theme toggle: InvisibleButton + draw soft rounded bg (hover/active only) + centered moon icon. Tint = Text (dark in light, light in dark).
static void DrawThemeToggleButton()
{
    const float btn_size = 24.f;
    const float margin = 8.f;
    const float icon_pad = 2.f;  // inset so icon is centered and not clipped
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

    // Default: no background. Hover/active: soft rounded bg from theme (no hardcoded blue).
    if (active)
        dl->AddRectFilled(mn, mx, GetStyleColorU32(ImGuiCol_FrameBgActive), rounding);
    else if (hovered)
        dl->AddRectFilled(mn, mx, GetStyleColorU32(ImGuiCol_FrameBgHovered), rounding);

    // Tint animates with theme: light = dark gray, dark = near-white (blended in ApplyBlendedTheme).
    ImU32 icon_tint = GetStyleColorU32(ImGuiCol_Text);
    ImVec2 imn(mn.x + icon_pad, mn.y + icon_pad);
    ImVec2 imx(mx.x - icon_pad, mx.y - icon_pad);
    if (g_MoonTex != (ImTextureID)0)
        dl->AddImage(g_MoonTex, imn, imx, ImVec2(0, 0), ImVec2(1, 1), icon_tint);
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

// Theme transition: ~220ms duration, eased, stable at any FPS. No allocation per frame.
static const float kThemeTransitionDuration = 0.22f;  // 180–260ms range

// Single theme apply per frame: update g_ThemeT from g_DarkMode, ease, then ApplyBlendedTheme. Call after NewFrame(), before RenderMenu().
void ApplyMenuTheme()
{
    // #region agent log
    DebugLog("menu.cpp:ApplyMenuTheme", "enter", "ApplyMenuTheme");
    // #endregion
    float dt = ImGui::GetIO().DeltaTime;
    float target = g_DarkMode ? 1.0f : 0.0f;
    float max_delta = (kThemeTransitionDuration > 0.f) ? (1.0f / kThemeTransitionDuration) * dt : 1.0f;
    g_ThemeT = MoveTowards(g_ThemeT, target, max_delta);
    g_ThemeT = ImClamp(g_ThemeT, 0.0f, 1.0f);

    float t_eased = EaseInOutCubic(g_ThemeT);
    Theme::ApplyBlendedTheme(t_eased);
}

void RenderMenu()
{
    // #region agent log
    DebugLog("menu.cpp:RenderMenu", "enter", "RenderMenu");
    // #endregion
    float w = kMenuWidth * dpi_scale;
    float h = kMenuHeight * dpi_scale;
    ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::SetWindowSize(ImVec2(ImFloor(w), ImFloor(h)));
    // #region agent log
    DebugLog("menu.cpp:RenderMenu", "before DrawMenuBackground", "DrawMenuBackground");
    // #endregion
    DrawMenuBackground();
    // #region agent log
    DebugLog("menu.cpp:RenderMenu", "before DrawThemeToggleButton", "DrawThemeToggleButton");
    // #endregion
    DrawThemeToggleButton();
    // #region agent log
    DebugLog("menu.cpp:RenderMenu", "before DrawIconRail", "DrawIconRail");
    // #endregion
    DrawIconRail();
    // #region agent log
    DebugLog("menu.cpp:RenderMenu", "before DrawVerticalDivider", "DrawVerticalDivider");
    // #endregion
    DrawVerticalDivider();
    // #region agent log
    DebugLog("menu.cpp:RenderMenu", "before DrawSidebar", "DrawSidebar");
    // #endregion
    DrawSidebar();
    // #region agent log
    DebugLog("menu.cpp:RenderMenu", "before DrawGeneralPanel", "DrawGeneralPanel");
    // #endregion
    DrawGeneralPanel();
    // #region agent log
    DebugLog("menu.cpp:RenderMenu", "before DrawPreviewPanel", "DrawPreviewPanel");
    // #endregion
    DrawPreviewPanel();
    // #region agent log
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiWindow* w = g.CurrentWindow;
        char buf[128];
        std::snprintf(buf, sizeof(buf), "id=%d:%d grp=%d:%d pop=%d:%d col=%d:%d style=%d:%d",
            (int)w->IDStack.Size, (int)w->DC.StackSizesOnBegin.SizeOfIDStack,
            (int)g.GroupStack.Size, (int)w->DC.StackSizesOnBegin.SizeOfGroupStack,
            (int)g.BeginPopupStack.Size, (int)w->DC.StackSizesOnBegin.SizeOfBeginPopupStack,
            (int)g.ColorStack.Size, (int)w->DC.StackSizesOnBegin.SizeOfColorStack,
            (int)g.StyleVarStack.Size, (int)w->DC.StackSizesOnBegin.SizeOfStyleVarStack);
        DebugLog("menu.cpp:RenderMenu", buf, "stacks_before_End");
    }
    DebugLog("menu.cpp:RenderMenu", "before ImGui::End", "RenderMenu_end");
    // #endregion
    ImGui::End();
    // #region agent log
    DebugLog("menu.cpp:RenderMenu", "after ImGui::End", "RenderMenu_done");
    // #endregion
}
