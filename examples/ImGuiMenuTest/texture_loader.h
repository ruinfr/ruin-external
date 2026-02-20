#pragma once

#include "imgui.h"

// Load image from path into an OpenGL texture. Uses project's OpenGL backend.
// Returns ImTextureID (GLuint) or (ImTextureID)0 on failure. Load once (e.g. lazy init).
// Safe: returns 0 if file missing or load fails; no crash.
ImTextureID LoadTextureFromFileOpenGL(const char* path);

// Release OpenGL texture (call on shutdown if backend requires). Safe to call with 0.
void ReleaseTextureOpenGL(ImTextureID id);

// Load image with vertical flip (for icon use). Uses stb_image, GL_RGBA, LINEAR, CLAMP_TO_EDGE.
// Returns ImTextureID (GLuint) or 0 on failure. out_width/out_height may be NULL.
ImTextureID LoadTextureFromFile(const char* filename, int* out_width, int* out_height);
