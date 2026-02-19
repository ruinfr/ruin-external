#include "imgui.h"
#include "imgui_internal.h"
#include "byte.h"
#include "menu.h"
#include "theme.h"
#include <cmath>
#include <map>

using namespace MenuLayout;

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

float dpi_scale = 1.f;

ImFont* gilroy = nullptr;
ImFont* gilroy_mini = nullptr;
ImFont* iconfont = nullptr;
ImVec2 pos;
ImDrawList* draw = nullptr;

static int sliderint = 0;
static bool checkbox = false;
static int tabs = 1;
static int subtabs = 0;
static int sett = 0;

// ---- Helpers ----

void DrawSectionHeader(const char* label)
{
    ImGui::Spacing();
    ImGui::Dummy(ImVec2(0, kSectionSpacingAbove));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(Theme::Color::kSectionHeader));
    ImGui::SetWindowFontScale(kSectionHeaderScale);
    ImGui::TextUnformatted(label);
    ImGui::SetWindowFontScale(1.f);
    ImGui::PopStyleColor();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    dl->AddLine(ImVec2(p.x, p.y), ImVec2(p.x + w, p.y), Theme::Color::kDivider, kDividerThickness);
    ImGui::Dummy(ImVec2(0, kDividerPaddingV));
}

bool DrawSidebarItem(const char* label, bool selected)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
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
    if (selected)
        dl->AddRectFilled(bb.Min, bb.Max, Theme::Color::kSidebarSelectedBg, rounding);
    if (hovered && !selected)
        dl->AddRectFilled(bb.Min, bb.Max, Theme::Color::kSidebarHoverBg, rounding);
    if (selected)
        dl->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x + kAccentBarWidth, bb.Max.y), Theme::Color::kSidebarAccentBar, 0.f);
    ImVec2 text_pos(bb.Min.x + style.FramePadding.x + (selected ? kAccentBarWidth : 0.f), bb.Min.y + (kSidebarItemHeight - label_size.y) * 0.5f);
    ImU32 text_color = selected ? Theme::Color::kSidebarItemTextSelected : Theme::Color::kSidebarItemText;
    dl->AddText(text_pos, text_color, label);
    return pressed;
}

bool DrawSidebarItemAnimated(const char* label, bool selected)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
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

    if (sl > 0.001f)
    {
        ImU32 selected_bg = SetAlpha(Theme::Color::kSidebarSelectedBg, sl);
        dl->AddRectFilled(bb.Min, bb.Max, selected_bg, rounding);
    }
    if (ha > 0.001f && sl < 0.999f)
    {
        ImU32 hover_bg = SetAlpha(Theme::Color::kSidebarHoverBg, ha);
        dl->AddRectFilled(bb.Min, bb.Max, hover_bg, rounding);
    }
    if (sl > 0.001f)
    {
        float bar_w = kAccentBarWidth * EaseOutCubic(sl);
        dl->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x + bar_w, bb.Max.y), Theme::Color::kSidebarAccentBar, 0.f);
    }

    float bar_vis = EaseOutCubic(sl);
    float text_offset = bar_vis * kAccentBarWidth;
    ImVec2 text_pos(bb.Min.x + style.FramePadding.x + text_offset, bb.Min.y + (kSidebarItemHeight - label_size.y) * 0.5f);
    ImU32 text_color = selected ? Theme::Color::kSidebarItemTextSelected : Theme::Color::kSidebarItemText;
    dl->AddText(text_pos, text_color, label);
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

static void DrawIconRail()
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(Theme::Color::kSidebarBg));
    ImGui::BeginChild("##icon_rail", ImVec2(kIconRailWidth, -1), false, 0);
    ImGui::SetCursorPos(ImVec2(kPanelPaddingH - 2.f, 7.f));
    DrawTopHeader("K", nullptr);
    ImGui::SetCursorPos(ImVec2(17.f, 75.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 16));
    ImGui::BeginGroup();
    if (ImGui::tab("A", 0 == tabs)) tabs = 0;
    if (ImGui::tab("B", 1 == tabs)) tabs = 1;
    if (ImGui::tab("C", 2 == tabs)) tabs = 2;
    if (ImGui::tab("D", 3 == tabs)) tabs = 3;
    if (ImGui::tab("E", 4 == tabs)) tabs = 4;
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40.f);
    if (ImGui::settingsbutton("L")) sett = 1;
    ImGui::EndGroup();
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

static void DrawSidebar()
{
    if (tabs != 1)
        return;
    ImGui::SameLine(0, 0);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(Theme::Color::kSidebarBg));
    ImGui::BeginChild("Visuals", ImVec2(kSidebarWidth, -1), false, 0);
    ImGui::SetCursorPos(ImVec2(kPanelPaddingH, kPanelPaddingV));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, kSidebarItemSpacing));
    DrawSectionHeader("ENEMY");
    if (DrawSidebarItemAnimated("ESP", subtabs == 0)) subtabs = 0;
    if (DrawSidebarItemAnimated("Chams", subtabs == 1)) subtabs = 1;
    if (DrawSidebarItemAnimated("Other", subtabs == 2)) subtabs = 2;
    DrawSectionHeader("TEAM");
    if (DrawSidebarItemAnimated("ESP##1", subtabs == 3)) subtabs = 3;
    if (DrawSidebarItemAnimated("Chams##1", subtabs == 4)) subtabs = 4;
    if (DrawSidebarItemAnimated("Other##1", subtabs == 5)) subtabs = 5;
    DrawSectionHeader("WORLD");
    if (DrawSidebarItemAnimated("ESP##2", subtabs == 6)) subtabs = 6;
    if (DrawSidebarItemAnimated("Chams##2", subtabs == 7)) subtabs = 7;
    if (DrawSidebarItemAnimated("Other##2", subtabs == 8)) subtabs = 8;
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
    dl->AddLine(ImVec2(p.x, p.y), ImVec2(p.x, p.y + h), Theme::Color::kDivider, kDividerThickness);
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
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(Theme::Color::kPanelBg));
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
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(Theme::Color::kPanelBg));
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
    draw->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), Theme::Color::kWindowBg, rounding);
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

void RenderMenu()
{
    float w = kMenuWidth * dpi_scale;
    float h = kMenuHeight * dpi_scale;
    ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::SetWindowSize(ImVec2(ImFloor(w), ImFloor(h)));
    DrawMenuBackground();
    DrawIconRail();
    DrawVerticalDivider();
    DrawSidebar();
    DrawGeneralPanel();
    DrawPreviewPanel();
    ImGui::End();
}
