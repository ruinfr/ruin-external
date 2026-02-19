#include "theme.h"
#include "imgui.h"

namespace Theme {

using namespace Style;
using namespace Color;

static int s_sidebar_style_push_count = 0;

static ImVec4 U32ToVec4(ImU32 c)
{
    return ImGui::ColorConvertU32ToFloat4(c);
}

static inline ImVec4 Lerp(const ImVec4& a, const ImVec4& b, float t)
{
    return ImVec4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t);
}

void ThemeLight(ImVec4* colors)
{
    ImVec4* c = colors;
    c[ImGuiCol_Text]                  = U32ToVec4(kText);
    c[ImGuiCol_All]                   = U32ToVec4(kText);
    c[ImGuiCol_TextDisabled]          = U32ToVec4(kTextDisabled);
    c[ImGuiCol_WindowBg]              = U32ToVec4(kWindowBg);
    c[ImGuiCol_ChildBg]               = U32ToVec4(kPanelBg);
    c[ImGuiCol_main]                  = U32ToVec4(kPanelBg);
    c[ImGuiCol_PopupBg]               = U32ToVec4(kPanelBg);
    c[ImGuiCol_Border]                = U32ToVec4(kBorder);
    c[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_FrameBg]               = U32ToVec4(kFrameBg);
    c[ImGuiCol_FrameBgHovered]        = U32ToVec4(kFrameBgHovered);
    c[ImGuiCol_FrameBgActive]         = U32ToVec4(kFrameBgActive);
    c[ImGuiCol_TitleBg]               = U32ToVec4(kWindowBg);
    c[ImGuiCol_TitleBgActive]         = U32ToVec4(kWindowBg);
    c[ImGuiCol_TitleBgCollapsed]      = U32ToVec4(kWindowBg);
    c[ImGuiCol_MenuBarBg]             = U32ToVec4(kSidebarBg);
    c[ImGuiCol_ScrollbarBg]           = ImVec4(0.98f, 0.98f, 0.99f, 0.53f);
    c[ImGuiCol_ScrollbarGrab]         = U32ToVec4(0xCFD0D5FF);
    c[ImGuiCol_ScrollbarGrabHovered]  = U32ToVec4(0xB8BAC0FF);
    c[ImGuiCol_ScrollbarGrabActive]   = U32ToVec4(0xA0A2A8FF);
    c[ImGuiCol_CheckMark]             = U32ToVec4(kSidebarAccentBar);
    c[ImGuiCol_Tab]                   = U32ToVec4(kSidebarHoverBg);
    c[ImGuiCol_SliderGrab]            = U32ToVec4(kSidebarAccentBar);
    c[ImGuiCol_SliderGrabActive]      = U32ToVec4(kButtonActive);
    c[ImGuiCol_Button]                = U32ToVec4(kButton);
    c[ImGuiCol_ButtonHovered]         = U32ToVec4(kButtonHovered);
    c[ImGuiCol_ButtonActive]          = U32ToVec4(kButtonActive);
    c[ImGuiCol_Header]                = U32ToVec4(kSidebarHoverBg);
    c[ImGuiCol_HeaderHovered]         = U32ToVec4(kSidebarHoverBg);
    c[ImGuiCol_HeaderActive]          = U32ToVec4(kSidebarSelectedBg);
    c[ImGuiCol_Separator]             = U32ToVec4(kDivider);
    c[ImGuiCol_SeparatorHovered]      = U32ToVec4(kSidebarAccentBar);
    c[ImGuiCol_SeparatorActive]       = U32ToVec4(kSidebarAccentBar);
    c[ImGuiCol_ResizeGrip]            = ImVec4(0.80f, 0.80f, 0.82f, 0.50f);
    c[ImGuiCol_ResizeGripHovered]     = ImVec4(0.55f, 0.60f, 0.78f, 0.70f);
    c[ImGuiCol_ResizeGripActive]      = ImVec4(0.45f, 0.52f, 0.72f, 1.0f);
    c[ImGuiCol_TabColor]              = U32ToVec4(kSidebarHoverBg);
    c[ImGuiCol_TabHovered]            = U32ToVec4(kSidebarSelectedBg);
    c[ImGuiCol_TabActive]             = U32ToVec4(kSidebarSelectedBg);
    c[ImGuiCol_TabUnfocused]          = U32ToVec4(kSidebarBg);
    c[ImGuiCol_TabUnfocusedActive]     = U32ToVec4(kSidebarHoverBg);
    c[ImGuiCol_PlotLines]             = U32ToVec4(kSidebarAccentBar);
    c[ImGuiCol_PlotLinesHovered]      = U32ToVec4(kButtonHovered);
    c[ImGuiCol_PlotHistogram]         = U32ToVec4(kSidebarAccentBar);
    c[ImGuiCol_PlotHistogramHovered]  = U32ToVec4(kButtonHovered);
    c[ImGuiCol_TableHeaderBg]         = U32ToVec4(kSidebarBg);
    c[ImGuiCol_TableBorderStrong]     = U32ToVec4(kBorder);
    c[ImGuiCol_TableBorderLight]      = U32ToVec4(kDivider);
    c[ImGuiCol_TableRowBg]            = U32ToVec4(kWindowBg);
    c[ImGuiCol_TableRowBgAlt]         = U32ToVec4(kPanelBg);
    c[ImGuiCol_TextSelectedBg]        = ImVec4(0.25f, 0.45f, 0.85f, 0.35f);
    c[ImGuiCol_DragDropTarget]        = ImVec4(0.25f, 0.45f, 0.85f, 0.60f);
    c[ImGuiCol_NavHighlight]          = ImVec4(0.25f, 0.45f, 0.85f, 0.80f);
    c[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.70f);
    c[ImGuiCol_NavWindowingDimBg]     = ImVec4(0, 0, 0, 0.40f);
    c[ImGuiCol_ModalWindowDimBg]      = ImVec4(0, 0, 0, 0.60f);
    c[ImGuiCol_SliderText]            = U32ToVec4(kText);
}

void ThemeDark(ImVec4* colors)
{
    // Gray-based dark mode: no pure black, soft text (not pure white), subtle borders, no neon.
    ImVec4* c = colors;
    const ImVec4 softText     = ImVec4(0.90f, 0.90f, 0.92f, 1.0f);
    const ImVec4 textDisabled = ImVec4(0.50f, 0.50f, 0.55f, 1.0f);
    const ImVec4 windowBg     = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
    const ImVec4 childBg     = ImVec4(0.14f, 0.14f, 0.17f, 1.0f);
    const ImVec4 border      = ImVec4(0.28f, 0.28f, 0.32f, 1.0f);
    const ImVec4 frameBg     = ImVec4(0.18f, 0.18f, 0.20f, 1.0f);
    const ImVec4 frameHover  = ImVec4(0.22f, 0.22f, 0.25f, 1.0f);
    const ImVec4 frameActive = ImVec4(0.25f, 0.25f, 0.28f, 1.0f);
    const ImVec4 accentMuted = ImVec4(0.45f, 0.55f, 0.75f, 1.0f);  // muted blue, no neon
    const ImVec4 headerBg    = ImVec4(0.22f, 0.22f, 0.26f, 1.0f);
    const ImVec4 headerHover = ImVec4(0.28f, 0.28f, 0.33f, 1.0f);
    const ImVec4 headerActive= ImVec4(0.25f, 0.30f, 0.38f, 1.0f);
    const ImVec4 button      = ImVec4(0.28f, 0.32f, 0.42f, 1.0f);
    const ImVec4 buttonHover = ImVec4(0.35f, 0.38f, 0.50f, 1.0f);
    const ImVec4 buttonActive= ImVec4(0.32f, 0.36f, 0.48f, 1.0f);

    c[ImGuiCol_Text]                  = softText;
    c[ImGuiCol_All]                   = softText;
    c[ImGuiCol_TextDisabled]          = textDisabled;
    c[ImGuiCol_WindowBg]              = windowBg;
    c[ImGuiCol_ChildBg]               = childBg;
    c[ImGuiCol_main]                  = childBg;
    c[ImGuiCol_PopupBg]               = childBg;
    c[ImGuiCol_Border]                = border;
    c[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0.5f);
    c[ImGuiCol_FrameBg]               = frameBg;
    c[ImGuiCol_FrameBgHovered]        = frameHover;
    c[ImGuiCol_FrameBgActive]         = frameActive;
    c[ImGuiCol_TitleBg]               = windowBg;
    c[ImGuiCol_TitleBgActive]         = windowBg;
    c[ImGuiCol_TitleBgCollapsed]      = windowBg;
    c[ImGuiCol_MenuBarBg]             = childBg;
    c[ImGuiCol_ScrollbarBg]           = ImVec4(0.10f, 0.10f, 0.12f, 0.53f);
    c[ImGuiCol_ScrollbarGrab]         = ImVec4(0.35f, 0.35f, 0.40f, 1.0f);
    c[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.45f, 0.45f, 0.50f, 1.0f);
    c[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.55f, 0.55f, 0.60f, 1.0f);
    c[ImGuiCol_CheckMark]             = accentMuted;
    c[ImGuiCol_Tab]                   = headerBg;
    c[ImGuiCol_SliderGrab]            = accentMuted;
    c[ImGuiCol_SliderGrabActive]      = ImVec4(0.50f, 0.60f, 0.80f, 1.0f);
    c[ImGuiCol_Button]                = button;
    c[ImGuiCol_ButtonHovered]         = buttonHover;
    c[ImGuiCol_ButtonActive]         = buttonActive;
    c[ImGuiCol_Header]                = headerBg;
    c[ImGuiCol_HeaderHovered]         = headerHover;
    c[ImGuiCol_HeaderActive]          = headerActive;
    c[ImGuiCol_Separator]             = border;
    c[ImGuiCol_SeparatorHovered]      = accentMuted;
    c[ImGuiCol_SeparatorActive]       = accentMuted;
    c[ImGuiCol_ResizeGrip]            = ImVec4(0.35f, 0.35f, 0.40f, 0.50f);
    c[ImGuiCol_ResizeGripHovered]     = ImVec4(0.45f, 0.45f, 0.52f, 0.70f);
    c[ImGuiCol_ResizeGripActive]      = ImVec4(0.50f, 0.50f, 0.58f, 1.0f);
    c[ImGuiCol_TabColor]              = headerBg;
    c[ImGuiCol_TabHovered]            = headerActive;
    c[ImGuiCol_TabActive]             = headerActive;
    c[ImGuiCol_TabUnfocused]          = childBg;
    c[ImGuiCol_TabUnfocusedActive]    = headerBg;
    c[ImGuiCol_PlotLines]             = accentMuted;
    c[ImGuiCol_PlotLinesHovered]      = buttonHover;
    c[ImGuiCol_PlotHistogram]         = accentMuted;
    c[ImGuiCol_PlotHistogramHovered]  = buttonHover;
    c[ImGuiCol_TableHeaderBg]         = headerBg;
    c[ImGuiCol_TableBorderStrong]    = ImVec4(0.30f, 0.30f, 0.35f, 1.0f);
    c[ImGuiCol_TableBorderLight]      = ImVec4(0.24f, 0.24f, 0.28f, 1.0f);
    c[ImGuiCol_TableRowBg]            = windowBg;
    c[ImGuiCol_TableRowBgAlt]         = childBg;
    c[ImGuiCol_TextSelectedBg]        = ImVec4(0.30f, 0.38f, 0.55f, 0.35f);
    c[ImGuiCol_DragDropTarget]        = ImVec4(0.35f, 0.45f, 0.65f, 0.60f);
    c[ImGuiCol_NavHighlight]          = ImVec4(0.45f, 0.55f, 0.75f, 0.80f);
    c[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.70f);
    c[ImGuiCol_NavWindowingDimBg]     = ImVec4(0, 0, 0, 0.40f);
    c[ImGuiCol_ModalWindowDimBg]      = ImVec4(0, 0, 0, 0.60f);
    c[ImGuiCol_SliderText]            = softText;
}

void ApplyBlendedTheme(float t)
{
    t = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
    static ImVec4 s_light[ImGuiCol_COUNT];
    static ImVec4 s_dark[ImGuiCol_COUNT];
    static bool s_init = false;
    if (!s_init)
    {
        ThemeLight(s_light);
        ThemeDark(s_dark);
        s_init = true;
    }
    ImVec4* dst = ImGui::GetStyle().Colors;
    for (int i = 0; i < ImGuiCol_COUNT; i++)
        dst[i] = Lerp(s_light[i], s_dark[i], t);
}

void ApplyTheme(float dpi_scale)
{
    ImGuiStyle& s = ImGui::GetStyle();
    s.Alpha = 1.f;
    s.WindowRounding = kWindowRounding;
    s.ChildRounding = kFrameRounding;
    s.FrameRounding = kFrameRounding;
    s.PopupRounding = kPopupRounding;
    s.ScrollbarRounding = kScrollbarRounding;
    s.GrabRounding = kGrabRounding;
    s.FramePadding = ImVec2(kFramePaddingX, kFramePaddingY);
    s.ItemSpacing = ImVec2(kItemSpacingX, kItemSpacingY);
    s.ItemInnerSpacing = ImVec2(kItemInnerSpacingX, kItemInnerSpacingY);
    s.WindowPadding = ImVec2(kWindowPaddingX, kWindowPaddingY);
    s.WindowBorderSize = kBorderSize;
    s.ChildBorderSize = kBorderSize;
    s.FrameBorderSize = 0.f;
    s.PopupBorderSize = kBorderSize;
    s.ScrollbarSize = kScrollbarSize;

    ImVec4* c = s.Colors;
    c[ImGuiCol_Text]                  = U32ToVec4(kText);
    c[ImGuiCol_TextDisabled]          = U32ToVec4(kTextDisabled);
    c[ImGuiCol_WindowBg]              = U32ToVec4(kWindowBg);
    c[ImGuiCol_ChildBg]               = U32ToVec4(kPanelBg);
    c[ImGuiCol_PopupBg]               = U32ToVec4(kPanelBg);
    c[ImGuiCol_Border]                = U32ToVec4(kBorder);
    c[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_FrameBg]               = U32ToVec4(kFrameBg);
    c[ImGuiCol_FrameBgHovered]        = U32ToVec4(kFrameBgHovered);
    c[ImGuiCol_FrameBgActive]         = U32ToVec4(kFrameBgActive);
    c[ImGuiCol_TitleBg]               = U32ToVec4(kWindowBg);
    c[ImGuiCol_TitleBgActive]         = U32ToVec4(kWindowBg);
    c[ImGuiCol_TitleBgCollapsed]       = U32ToVec4(kWindowBg);
    c[ImGuiCol_MenuBarBg]             = U32ToVec4(kSidebarBg);
    c[ImGuiCol_ScrollbarBg]           = ImVec4(0.98f, 0.98f, 0.99f, 0.53f);
    c[ImGuiCol_ScrollbarGrab]         = U32ToVec4(0xCFD0D5FF);
    c[ImGuiCol_ScrollbarGrabHovered]   = U32ToVec4(0xB8BAC0FF);
    c[ImGuiCol_ScrollbarGrabActive]    = U32ToVec4(0xA0A2A8FF);
    c[ImGuiCol_CheckMark]             = U32ToVec4(kSidebarAccentBar);
    c[ImGuiCol_SliderGrab]            = U32ToVec4(kSidebarAccentBar);
    c[ImGuiCol_SliderGrabActive]       = U32ToVec4(kButtonActive);
    c[ImGuiCol_Button]                = U32ToVec4(kButton);
    c[ImGuiCol_ButtonHovered]          = U32ToVec4(kButtonHovered);
    c[ImGuiCol_ButtonActive]          = U32ToVec4(kButtonActive);
    c[ImGuiCol_Header]                = U32ToVec4(kSidebarHoverBg);
    c[ImGuiCol_HeaderHovered]         = U32ToVec4(kSidebarHoverBg);
    c[ImGuiCol_HeaderActive]          = U32ToVec4(kSidebarSelectedBg);
    c[ImGuiCol_Separator]             = U32ToVec4(kDivider);
    c[ImGuiCol_SeparatorHovered]      = U32ToVec4(kSidebarAccentBar);
    c[ImGuiCol_SeparatorActive]       = U32ToVec4(kSidebarAccentBar);

    s.ScaleAllSizes(dpi_scale);
}

void PushSidebarItemStyle(bool selected)
{
    const ImVec4* c = ImGui::GetStyle().Colors;
    if (selected)
    {
        ImGui::PushStyleColor(ImGuiCol_Header, c[ImGuiCol_HeaderActive]);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, c[ImGuiCol_HeaderActive]);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, c[ImGuiCol_HeaderActive]);
        ImGui::PushStyleColor(ImGuiCol_Text, c[ImGuiCol_Text]);
        s_sidebar_style_push_count = 4;
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, c[ImGuiCol_Text]);
        s_sidebar_style_push_count = 1;
    }
}

void PopSidebarItemStyle()
{
    if (s_sidebar_style_push_count > 0)
    {
        ImGui::PopStyleColor(s_sidebar_style_push_count);
        s_sidebar_style_push_count = 0;
    }
}

} // namespace Theme
