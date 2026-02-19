#pragma once

struct ImGuiIO;

// Call after ImGui::CreateContext() to apply menu style and load fonts.
void InitMenuStyle();
void LoadMenuFonts(ImGuiIO& io);

// Call every frame inside the render loop (after ImGui::NewFrame()).
// Renders the "Menu" window with decorations, tabs, subtabs, and content.
void RenderMenu();
