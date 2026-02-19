#include "imgui.h"
#include "imgui_internal.h"
#include "byte.h"
#include "menu.h"
#include <cmath>

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

static void decorations()
{
    pos = ImGui::GetWindowPos();
    draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(pos, ImVec2(pos.x + 805, pos.y + 480), IM_COL32(235, 235, 240, 255), 12);
}

static void tabs_()
{
    ImGui::BeginChild("##tabs", ImVec2(50, 475));
    ImGui::SetCursorPos(ImVec2(27, 7));
    ImGui::PushFont(iconfont);
    ImGui::Text("K");
    ImGui::PopFont();
    ImGui::SetCursorPos(ImVec2(17, 75));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 16));
    ImGui::BeginGroup();
    if (ImGui::tab("A", 0 == tabs)) tabs = 0;
    if (ImGui::tab("B", 1 == tabs)) tabs = 1;
    if (ImGui::tab("C", 2 == tabs)) tabs = 2;
    if (ImGui::tab("D", 3 == tabs)) tabs = 3;
    if (ImGui::tab("E", 4 == tabs)) tabs = 4;
    ImGui::SetCursorPosY(450);
    if (ImGui::settingsbutton("L")) sett = 1;
    ImGui::EndGroup();
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

static void subtabs_()
{
    if (tabs != 1)
        return;
    ImGui::SetCursorPos(ImVec2(50, 0));
    ImGui::BeginChild("Visuals", ImVec2(160, 475));
    ImGui::SetCursorPos(ImVec2(15, 50));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
    ImGui::BeginGroup();
    ImGui::TextColored(ImVec4(195/255.f, 195/255.f, 200/255.f, 1.f), "ENEMY");
    ImGui::Spacing();
    if (ImGui::sub("ESP", 0 == subtabs)) subtabs = 0;
    if (ImGui::sub("Chams", 1 == subtabs)) subtabs = 1;
    if (ImGui::sub("Other", 2 == subtabs)) subtabs = 2;
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(195/255.f, 195/255.f, 200/255.f, 1.f), "TEAM");
    ImGui::Spacing();
    if (ImGui::sub("ESP##1", 3 == subtabs)) subtabs = 3;
    if (ImGui::sub("Chams##1", 4 == subtabs)) subtabs = 4;
    if (ImGui::sub("Other##1", 5 == subtabs)) subtabs = 5;
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(195/255.f, 195/255.f, 200/255.f, 1.f), "WORLD");
    ImGui::Spacing();
    if (ImGui::sub("ESP##2", 6 == subtabs)) subtabs = 6;
    if (ImGui::sub("Chams##2", 7 == subtabs)) subtabs = 7;
    if (ImGui::sub("Other##2", 8 == subtabs)) subtabs = 8;
    ImGui::EndGroup();
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

static void function()
{
    if (tabs != 1)
        return;
    if (subtabs == 0)
    {
        ImGui::SetCursorPos(ImVec2(210, 0));
        ImGui::BeginChild("General", ImVec2(280, 475));
        ImGui::SetCursorPos(ImVec2(15, 50));
        ImGui::BeginGroup();
        ImGui::Checkbox("Checkbox", &checkbox);
        ImGui::SliderInt("SliderInt", &sliderint, 0, 100);
        ImGui::Button("Button", ImVec2(220, 30));
        ImGui::EndGroup();
        ImGui::EndChild();
        ImGui::SameLine(0, 0);
        ImGui::BeginChild("Preview", ImVec2(315, 475));
        ImGui::EndChild();
    }
}

void InitMenuStyle()
{
    ImGui::StyleColorsDark();
    ImGuiStyle* style = &ImGui::GetStyle();
    style->Alpha = 1.f;
    style->WindowRounding = 5;
    style->FramePadding = ImVec2(4, 3);
    style->WindowPadding = ImVec2(0, 0);
    style->ItemInnerSpacing = ImVec2(4, 4);
    style->ItemSpacing = ImVec2(8, 0);
    style->FrameRounding = 12;
    style->ScrollbarSize = 2.f;
    style->ScrollbarRounding = 12.f;
    style->PopupRounding = 4.f;
    style->Rounding = 11.f;

    ImVec4* colors = style->Colors;
    colors[ImGuiCol_ChildBg] = ImVec4(26/255.f, 30/255.f, 35/255.f, 0.f);
    colors[ImGuiCol_Border] = ImVec4(1, 1, 1, 0);
    colors[ImGuiCol_FrameBg] = ImVec4(18/255.f, 19/255.f, 23/255.f, 1.f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(25/255.f, 25/255.f, 33/255.f, 1.f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(25/255.f, 25/255.f, 33/255.f, 1.f);
    colors[ImGuiCol_Header] = ImVec4(141/255.f, 142/255.f, 144/255.f, 1.f);
    colors[ImGuiCol_HeaderActive] = ImVec4(141/255.f, 142/255.f, 144/255.f, 1.f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(141/255.f, 142/255.f, 144/255.f, 1.f);
    colors[ImGuiCol_PopupBg] = ImVec4(141/255.f, 142/255.f, 144/255.f, 1.f);
    colors[ImGuiCol_Button] = ImVec4(160/255.f, 30/255.f, 30/255.f, 1.f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(190/255.f, 45/255.f, 35/255.f, 1.f);
    colors[ImGuiCol_ButtonActive] = ImVec4(220/255.f, 60/255.f, 40/255.f, 1.f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(110/255.f, 122/255.f, 200/255.f, 1.f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1, 1, 1, 1.f);
    colors[ImGuiCol_CheckMark] = ImVec4(1, 1, 1, 1.f);
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
    ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_HorizontalScrollbar);
    float x = 805.f * dpi_scale;
    float y = 575.f * dpi_scale;
    pos = ImGui::GetWindowPos();
    draw = ImGui::GetWindowDrawList();
    ImGui::SetWindowSize(ImVec2(ImFloor(x), ImFloor(y)));
    decorations();
    tabs_();
    subtabs_();
    function();
    ImGui::End();
}
