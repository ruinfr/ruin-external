#pragma once

#include "imgui.h"

// Load image from path into an OpenGL texture. Uses project's OpenGL backend.
// Returns ImTextureID (GLuint) or (ImTextureID)0 on failure. Load once (e.g. lazy init).
// Safe: returns 0 if file missing or load fails; no crash.
ImTextureID LoadTextureFromFileOpenGL(const char* path);

// Release OpenGL texture (call on shutdown if backend requires). Safe to call with 0.
void ReleaseTextureOpenGL(ImTextureID id);
