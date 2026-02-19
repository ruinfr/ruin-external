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
    if (selected)
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImGui::ColorConvertU32ToFloat4(kSidebarSelectedBg));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::ColorConvertU32ToFloat4(kSidebarSelectedBg));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::ColorConvertU32ToFloat4(kSidebarSelectedBg));
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(kSidebarItemTextSelected));
        s_sidebar_style_push_count = 4;
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(kSidebarItemText));
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
