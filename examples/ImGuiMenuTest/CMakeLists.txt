cmake_minimum_required(VERSION 3.14)
project(ImGuiMenuTest LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# ImGui root (two levels up from examples/ImGuiMenuTest)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(IMGUI_BACKENDS_DIR ${IMGUI_DIR}/backends)
set(IMGUI_EXAMPLES_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/../libs)
set(IMGUI_DX9_EXAMPLE ${CMAKE_CURRENT_SOURCE_DIR}/../example_win32_directx9)

# OpenGL
find_package(OpenGL REQUIRED)

# GLFW: use FetchContent if not found (no system install required)
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ImGui core sources
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

# Backends
set(IMGUI_BACKEND_SOURCES
    ${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp
)

# GL3W (OpenGL loader bundled in repo)
set(GL3W_SOURCE ${IMGUI_EXAMPLES_LIBS}/gl3w/GL/gl3w.c)

add_executable(ImGuiMenuTest
    main.cpp
    menu.cpp
    theme.cpp
    texture_loader.cpp
    ${IMGUI_SOURCES}
    ${IMGUI_BACKEND_SOURCES}
    ${GL3W_SOURCE}
)

target_include_directories(ImGuiMenuTest PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_BACKENDS_DIR}
    ${IMGUI_EXAMPLES_LIBS}/gl3w
    ${IMGUI_DX9_EXAMPLE}
)

target_link_libraries(ImGuiMenuTest
    OpenGL::GL
    glfw
)

if(WIN32)
    target_link_libraries(ImGuiMenuTest opengl32)
endif()

# Copy icons/ next to exe at build time for reliable icon loading
set(ICONS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../icons)
add_custom_command(TARGET ImGuiMenuTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:ImGuiMenuTest>/icons"
    COMMENT "Copy icons to exe directory"
)
if(EXISTS "${ICONS_SRC}/moon (1).png")
    add_custom_command(TARGET ImGuiMenuTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ICONS_SRC}/moon (1).png" "$<TARGET_FILE_DIR:ImGuiMenuTest>/icons/moon (1).png"
    )
endif()
if(EXISTS "${ICONS_SRC}/moon.png")
    add_custom_command(TARGET ImGuiMenuTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ICONS_SRC}/moon.png" "$<TARGET_FILE_DIR:ImGuiMenuTest>/icons/moon.png"
    )
endif()
# Tab icons: visuual.png, aim.png, whitelist.png (lowercase filenames)
foreach(ICON_NAME visuual aim whitelist)
    if(EXISTS "${ICONS_SRC}/${ICON_NAME}.png")
        add_custom_command(TARGET ImGuiMenuTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ICONS_SRC}/${ICON_NAME}.png" "$<TARGET_FILE_DIR:ImGuiMenuTest>/icons/${ICON_NAME}.png"
        )
    endif()
endforeach()
