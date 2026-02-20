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

// ---- UITheme: central palette (light + dark). All custom draw colors route through this. ----

static const UITheme& GetUIThemeLightImpl()
{
    // Light theme: fully neutral grayscale — no warm/yellow bias. Apple/Notion-style.
    static UITheme t = {};
    static bool init = false;
    if (!init)
    {
        // Base window (ultra-clean: almost white, still soft)
        t.WindowBg             = ImVec4(0.97f, 0.97f, 0.98f, 1.0f);
        // Sidebar / child areas
        t.ChildBg              = ImVec4(0.94f, 0.94f, 0.95f, 1.0f);
        // Panels (popups, tooltips)
        t.PopupBg               = ImVec4(0.95f, 0.95f, 0.96f, 1.0f);
        // Borders
        t.Border               = ImVec4(0.85f, 0.85f, 0.87f, 1.0f);
        t.BorderShadow         = ImVec4(0.f, 0.f, 0.f, 0.f);

        // Section headers (sidebar rows, selectables)
        t.Header               = ImVec4(0.92f, 0.92f, 0.93f, 1.0f);
        t.HeaderHovered        = ImVec4(0.88f, 0.88f, 0.90f, 1.0f);
        t.HeaderActive         = ImVec4(0.88f, 0.90f, 0.95f, 1.0f);  // slight cool tint for selected, not warm

        // Frames (inputs, sliders, checkboxes)
        t.FrameBg              = ImVec4(0.93f, 0.93f, 0.94f, 1.0f);
        t.FrameBgHovered       = ImVec4(0.89f, 0.89f, 0.91f, 1.0f);
        t.FrameBgActive        = ImVec4(0.86f, 0.86f, 0.89f, 1.0f);

        // Text
        t.Text                 = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);
        t.TextDisabled         = ImVec4(0.50f, 0.50f, 0.52f, 1.0f);

        // MenuBar = sidebar background (neutral)
        t.MenuBarBg            = t.ChildBg;
        t.ScrollbarBg          = ImVec4(0.96f, 0.96f, 0.97f, 0.53f);
        t.ScrollbarGrab        = ImVec4(0.81f, 0.81f, 0.84f, 1.0f);
        t.ScrollbarGrabHovered = ImVec4(0.72f, 0.72f, 0.75f, 1.0f);
        t.ScrollbarGrabActive  = ImVec4(0.63f, 0.63f, 0.66f, 1.0f);
        t.CheckMark            = ImVec4(0.20f, 0.20f, 0.22f, 1.0f);
        // Slider grab: slightly darker than track (FrameBg 0.93) — solid pill, no white
        t.SliderGrab           = ImVec4(0.72f, 0.72f, 0.76f, 1.0f);
        t.SliderGrabActive     = ImVec4(0.65f, 0.65f, 0.70f, 1.0f);
        t.Button               = ImVec4(0.90f, 0.90f, 0.92f, 1.0f);
        t.ButtonHovered        = ImVec4(0.86f, 0.86f, 0.89f, 1.0f);
        t.ButtonActive         = ImVec4(0.82f, 0.82f, 0.85f, 1.0f);
        t.Separator             = ImVec4(0.88f, 0.88f, 0.90f, 1.0f);
        t.SeparatorHovered     = t.CheckMark;
        t.SeparatorActive      = t.CheckMark;
        t.ResizeGrip           = ImVec4(0.80f, 0.80f, 0.82f, 0.50f);
        t.ResizeGripHovered    = ImVec4(0.70f, 0.70f, 0.73f, 0.70f);
        t.ResizeGripActive     = ImVec4(0.60f, 0.60f, 0.63f, 1.0f);
        t.Tab                  = t.Header;
        t.TabHovered           = t.HeaderHovered;
        t.TabActive            = t.HeaderActive;
        t.TabUnfocused         = t.MenuBarBg;
        t.TabUnfocusedActive   = t.Header;
        t.TableHeaderBg        = t.MenuBarBg;
        t.TableBorderStrong    = t.Border;
        t.TableBorderLight     = t.Separator;
        t.TableRowBg           = t.WindowBg;
        t.TableRowBgAlt        = t.ChildBg;
        t.TextSelectedBg       = ImVec4(0.72f, 0.72f, 0.78f, 0.35f);   // neutral cool tint
        t.DragDropTarget       = ImVec4(0.70f, 0.70f, 0.76f, 0.60f);
        t.NavHighlight         = ImVec4(0.72f, 0.72f, 0.78f, 0.80f);
        t.NavWindowingHighlight = ImVec4(0.92f, 0.92f, 0.94f, 0.70f);
        t.NavWindowingDimBg    = ImVec4(0.f, 0.f, 0.f, 0.40f);
        t.ModalWindowDimBg     = ImVec4(0.f, 0.f, 0.f, 0.60f);
        t.SliderText           = t.Text;
        init = true;
    }
    return t;
}

static const UITheme& GetUIThemeDarkImpl()
{
    // Near-black dark mode: no pure black, no blue tint, no element goes to pure white on hover.
    static UITheme t = {};
    static bool init = false;
    if (!init)
    {
        // Base: dark gray close to black (neutral, no blue)
        t.WindowBg             = ImVec4(0.08f, 0.08f, 0.09f, 1.0f);
        t.ChildBg              = ImVec4(0.10f, 0.10f, 0.11f, 1.0f);  // panels slightly lighter
        t.PopupBg               = t.ChildBg;
        t.Border                = ImVec4(0.20f, 0.20f, 0.22f, 1.0f);  // visible but subtle
        t.Separator             = ImVec4(0.18f, 0.18f, 0.20f, 1.0f);
        t.BorderShadow          = ImVec4(0.f, 0.f, 0.f, 0.5f);

        // Text: soft white; disabled still readable
        t.Text                 = ImVec4(0.90f, 0.90f, 0.92f, 1.0f);
        t.TextDisabled         = ImVec4(0.52f, 0.52f, 0.56f, 1.0f);

        // Frames (inputs): slightly lighter than window; hover/active = very subtle lift (no white)
        t.FrameBg              = ImVec4(0.12f, 0.12f, 0.13f, 1.0f);
        t.FrameBgHovered       = ImVec4(0.15f, 0.15f, 0.16f, 1.0f);
        t.FrameBgActive        = ImVec4(0.17f, 0.17f, 0.19f, 1.0f);

        t.MenuBarBg            = t.ChildBg;
        t.ScrollbarBg          = ImVec4(0.06f, 0.06f, 0.07f, 0.53f);
        t.ScrollbarGrab        = ImVec4(0.26f, 0.26f, 0.28f, 1.0f);
        t.ScrollbarGrabHovered = ImVec4(0.32f, 0.32f, 0.34f, 1.0f);
        t.ScrollbarGrabActive  = ImVec4(0.38f, 0.38f, 0.40f, 1.0f);

        // CheckMark: light so checkbox tick is visible on dark frame; sidebar accent uses same
        t.CheckMark            = ImVec4(0.90f, 0.90f, 0.92f, 1.0f);
        // Slider grab: slightly lighter than track (FrameBg 0.12) — solid pill, never white
        t.SliderGrab           = ImVec4(0.26f, 0.26f, 0.28f, 1.0f);
        t.SliderGrabActive     = ImVec4(0.30f, 0.30f, 0.32f, 1.0f);

        // Buttons: hover only slightly brighter than Button, active slightly stronger; never near white
        t.Button               = ImVec4(0.16f, 0.16f, 0.18f, 1.0f);
        t.ButtonHovered        = ImVec4(0.19f, 0.19f, 0.21f, 1.0f);
        t.ButtonActive         = ImVec4(0.21f, 0.21f, 0.23f, 1.0f);

        // Headers (sidebar rows): subtle selected and hover, no bright jump
        t.Header               = ImVec4(0.11f, 0.11f, 0.12f, 1.0f);
        t.HeaderHovered        = ImVec4(0.14f, 0.14f, 0.15f, 1.0f);
        t.HeaderActive         = ImVec4(0.16f, 0.16f, 0.18f, 1.0f);

        t.SeparatorHovered     = t.CheckMark;
        t.SeparatorActive      = t.CheckMark;
        t.ResizeGrip           = ImVec4(0.24f, 0.24f, 0.26f, 0.50f);
        t.ResizeGripHovered    = ImVec4(0.30f, 0.30f, 0.32f, 0.70f);
        t.ResizeGripActive     = ImVec4(0.34f, 0.34f, 0.36f, 1.0f);

        t.Tab                  = t.Header;
        t.TabHovered           = t.HeaderActive;
        t.TabActive            = t.HeaderActive;
        t.TabUnfocused         = t.ChildBg;
        t.TabUnfocusedActive   = t.Header;
        t.TableHeaderBg        = t.Header;
        t.TableBorderStrong    = ImVec4(0.22f, 0.22f, 0.24f, 1.0f);
        t.TableBorderLight     = ImVec4(0.16f, 0.16f, 0.18f, 1.0f);
        t.TableRowBg           = t.WindowBg;
        t.TableRowBgAlt        = t.ChildBg;
        t.TextSelectedBg       = ImVec4(0.28f, 0.34f, 0.44f, 0.35f);
        t.DragDropTarget       = ImVec4(0.32f, 0.38f, 0.48f, 0.60f);
        t.NavHighlight         = t.CheckMark;
        t.NavWindowingHighlight = ImVec4(0.90f, 0.90f, 0.92f, 0.70f);  // soft white, not pure
        t.NavWindowingDimBg    = ImVec4(0.f, 0.f, 0.f, 0.50f);
        t.ModalWindowDimBg     = ImVec4(0.f, 0.f, 0.f, 0.60f);
        t.SliderText           = t.Text;
        init = true;
    }
    return t;
}

const UITheme& GetUIThemeLight() { return GetUIThemeLightImpl(); }
const UITheme& GetUIThemeDark()  { return GetUIThemeDarkImpl(); }

void FillStyleFromTheme(const UITheme& t, ImVec4* c)
{
    c[ImGuiCol_Text]                  = t.Text;
    c[ImGuiCol_All]                   = t.Text;
    c[ImGuiCol_TextDisabled]          = t.TextDisabled;
    c[ImGuiCol_WindowBg]              = t.WindowBg;
    c[ImGuiCol_ChildBg]               = t.ChildBg;
    c[ImGuiCol_main]                  = t.ChildBg;
    c[ImGuiCol_PopupBg]               = t.PopupBg;
    c[ImGuiCol_Border]                = t.Border;
    c[ImGuiCol_BorderShadow]          = t.BorderShadow;
    c[ImGuiCol_FrameBg]               = t.FrameBg;
    c[ImGuiCol_FrameBgHovered]        = t.FrameBgHovered;
    c[ImGuiCol_FrameBgActive]         = t.FrameBgActive;
    c[ImGuiCol_TitleBg]               = t.WindowBg;
    c[ImGuiCol_TitleBgActive]         = t.WindowBg;
    c[ImGuiCol_TitleBgCollapsed]      = t.WindowBg;
    c[ImGuiCol_MenuBarBg]             = t.MenuBarBg;
    c[ImGuiCol_ScrollbarBg]           = t.ScrollbarBg;
    c[ImGuiCol_ScrollbarGrab]         = t.ScrollbarGrab;
    c[ImGuiCol_ScrollbarGrabHovered]  = t.ScrollbarGrabHovered;
    c[ImGuiCol_ScrollbarGrabActive]   = t.ScrollbarGrabActive;
    c[ImGuiCol_CheckMark]             = t.CheckMark;
    c[ImGuiCol_Tab]                   = t.Tab;
    c[ImGuiCol_SliderGrab]            = t.SliderGrab;
    c[ImGuiCol_SliderGrabActive]      = t.SliderGrabActive;
    c[ImGuiCol_Button]                = t.Button;
    c[ImGuiCol_ButtonHovered]         = t.ButtonHovered;
    c[ImGuiCol_ButtonActive]          = t.ButtonActive;
    c[ImGuiCol_Header]                = t.Header;
    c[ImGuiCol_HeaderHovered]         = t.HeaderHovered;
    c[ImGuiCol_HeaderActive]          = t.HeaderActive;
    c[ImGuiCol_Separator]             = t.Separator;
    c[ImGuiCol_SeparatorHovered]      = t.SeparatorHovered;
    c[ImGuiCol_SeparatorActive]       = t.SeparatorActive;
    c[ImGuiCol_ResizeGrip]            = t.ResizeGrip;
    c[ImGuiCol_ResizeGripHovered]     = t.ResizeGripHovered;
    c[ImGuiCol_ResizeGripActive]      = t.ResizeGripActive;
    c[ImGuiCol_TabColor]              = t.Tab;
    c[ImGuiCol_TabHovered]            = t.TabHovered;
    c[ImGuiCol_TabActive]             = t.TabActive;
    c[ImGuiCol_TabUnfocused]          = t.TabUnfocused;
    c[ImGuiCol_TabUnfocusedActive]    = t.TabUnfocusedActive;
    c[ImGuiCol_PlotLines]             = t.CheckMark;
    c[ImGuiCol_PlotLinesHovered]      = t.ButtonHovered;
    c[ImGuiCol_PlotHistogram]         = t.CheckMark;
    c[ImGuiCol_PlotHistogramHovered]  = t.ButtonHovered;
    c[ImGuiCol_TableHeaderBg]         = t.TableHeaderBg;
    c[ImGuiCol_TableBorderStrong]     = t.TableBorderStrong;
    c[ImGuiCol_TableBorderLight]      = t.TableBorderLight;
    c[ImGuiCol_TableRowBg]            = t.TableRowBg;
    c[ImGuiCol_TableRowBgAlt]         = t.TableRowBgAlt;
    c[ImGuiCol_TextSelectedBg]        = t.TextSelectedBg;
    c[ImGuiCol_DragDropTarget]        = t.DragDropTarget;
    c[ImGuiCol_NavHighlight]          = t.NavHighlight;
    c[ImGuiCol_NavWindowingHighlight] = t.NavWindowingHighlight;
    c[ImGuiCol_NavWindowingDimBg]     = t.NavWindowingDimBg;
    c[ImGuiCol_ModalWindowDimBg]      = t.ModalWindowDimBg;
    c[ImGuiCol_SliderText]            = t.SliderText;
}

void ThemeLight(ImVec4* colors)
{
    FillStyleFromTheme(GetUIThemeLight(), colors);
}

void ThemeDark(ImVec4* colors)
{
    FillStyleFromTheme(GetUIThemeDark(), colors);
}

// Full palettes built once; no per-frame allocation. Used by ApplyBlendedTheme and GetBlendedColor.
static ImVec4* GetLightPalette()
{
    static ImVec4 s_light[ImGuiCol_COUNT];
    static bool s_light_init = false;
    if (!s_light_init) { ThemeLight(s_light); s_light_init = true; }
    return s_light;
}
static ImVec4* GetDarkPalette()
{
    static ImVec4 s_dark[ImGuiCol_COUNT];
    static bool s_dark_init = false;
    if (!s_dark_init) { ThemeDark(s_dark); s_dark_init = true; }
    return s_dark;
}

void ApplyBlendedTheme(float t)
{
    t = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
    ImVec4* light = GetLightPalette();
    ImVec4* dark = GetDarkPalette();
    ImVec4* dst = ImGui::GetStyle().Colors;
    for (int i = 0; i < ImGuiCol_COUNT; i++)
        dst[i] = Lerp(light[i], dark[i], t);
}

ImVec4 GetBlendedColor(ImGuiCol_ col, float t)
{
    t = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
    ImVec4* light = GetLightPalette();
    ImVec4* dark = GetDarkPalette();
    return Lerp(light[col], dark[col], t);
}

ImVec4 GetAccentColor(float t)
{
    return GetBlendedColor(ImGuiCol_CheckMark, t);
}

SidebarColors GetSidebarColors(float t)
{
    SidebarColors out;
    out.header         = GetBlendedColor(ImGuiCol_Header, t);
    out.headerHovered  = GetBlendedColor(ImGuiCol_HeaderHovered, t);
    out.headerActive   = GetBlendedColor(ImGuiCol_HeaderActive, t);
    out.checkMark      = GetBlendedColor(ImGuiCol_CheckMark, t);
    return out;
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

    FillStyleFromTheme(GetUIThemeLight(), s.Colors);
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
