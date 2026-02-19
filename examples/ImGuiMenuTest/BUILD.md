# ImGuiMenuTest — Build Instructions (Windows)

Standalone GLFW + OpenGL3 + Dear ImGui app to visually test the menu UI.  
**Result:** `ImGuiMenuTest.exe` opens a window and runs your menu render function every frame.

---

## Prerequisites

- **CMake** 3.10 or newer  
- **C++11** compiler (e.g. Visual Studio 2017+, or MinGW with GCC/Clang)  
- **OpenGL** (desktop, 3.0+)

**GLFW** is fetched automatically by CMake (FetchContent); no system install needed.  
The **gl3w** OpenGL loader is taken from the repo (`examples/libs/gl3w`).

---

## Build with CMake

From the **ImGuiMenuTest** folder (this directory):

```bat
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

- **Output:** `build\Release\ImGuiMenuTest.exe` (Visual Studio) or `build\ImGuiMenuTest.exe` (single-config generators).

Run:

```bat
Release\ImGuiMenuTest.exe
```

(or `.\ImGuiMenuTest.exe` if the binary is in `build`).

---

## CMake from repo root (optional)

If you prefer to run CMake from the repository root:

```bat
mkdir build
cd build
cmake -S examples/ImGuiMenuTest -B .
cmake --build . --config Release
```

Binary location is the same as above (e.g. `Release\ImGuiMenuTest.exe`).

---

## If CMake or GLFW is not available

1. **GLFW:** Install via vcpkg (`vcpkg install glfw3:x64-windows`) or download from [glfw.org](https://www.glfw.org/) and set `CMAKE_PREFIX_PATH` or `glfw3_DIR` so CMake can find it.
2. **One-config build (e.g. MinGW):**  
   ```bat
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build .
   ```
   The executable is then `build\ImGuiMenuTest.exe`.

---

## Visual Studio project (alternative)

To generate a Visual Studio solution instead of building from the command line:

```bat
cd examples\ImGuiMenuTest
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
```

Open `ImGuiMenuTest.sln`, set **ImGuiMenuTest** as startup project, build **Release**, then run (F5 or Ctrl+F5).  
Output: `build\Release\ImGuiMenuTest.exe`.

---

## Summary

| Step        | Command |
|------------|--------|
| Configure | `cd examples\ImGuiMenuTest\build` then `cmake ..` |
| Build     | `cmake --build . --config Release` |
| Run       | `Release\ImGuiMenuTest.exe` |

No modifications are made to Dear ImGui core or backend source; only the application wrapper and this build config are used.
