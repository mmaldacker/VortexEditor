cmake_minimum_required(VERSION 3.2)

project(VortexEditor)
set(CMAKE_CXX_STANDARD 14)

# cmake file to download and include dependencies
include(cmake/DownloadProject.cmake)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
if (APPLE)
    set(GLFW_VULKAN_STATIC ON  CACHE BOOL "" FORCE)
endif()
download_project(PROJ                glfw3
                 GIT_REPOSITORY      https://github.com/glfw/glfw.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${glfw3_SOURCE_DIR} ${glfw3_BINARY_DIR})

download_project(PROJ                Box2D
                 GIT_REPOSITORY      https://github.com/erincatto/Box2D.git
                 GIT_TAG             v2.3.1
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})

set(BOX2D_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${Box2D_SOURCE_DIR}/Box2D ${Box2D_BINARY_DIR})

download_project(PROJ                Vortex2D
                 GIT_REPOSITORY      https://github.com/mmaldacker/Vortex2D.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${Vortex2D_SOURCE_DIR} ${Vortex2D_BINARY_DIR})

file(GLOB IMGUI_SRC
    "imgui/imgui.h"
    "imgui/imgui.cpp"
    "imgui/imguiconfig.h"
    "imgui/imgui_draw.cpp"
    "imgui/imgui_internal.h"
    "imgui/imgui_widgets.cpp"
    )

compile_shader(SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/imgui.frag
                 ${CMAKE_CURRENT_SOURCE_DIR}/imgui.vert
               OUTPUT "vortexeditor_spirv"
               VERSION 1.0)

add_executable(VortexEditor
    main.cpp
    imgui.frag
    imgui.vert
    imguirenderer.h
    imguirenderer.cpp
    shaperenderer.h
    shaperenderer.cpp
    world.h
    world.cpp
    rigidbody.h
    rigidbody.cpp
    entity.h
    entity.cpp
    ${IMGUI_SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/vortexeditor_spirv.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/vortexeditor_spirv.h
)

target_link_libraries(VortexEditor glfw vortex2d Box2D)

# Ensure glfw is included first
target_include_directories(VortexEditor PRIVATE
    $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
    ${CMAKE_CURRENT_BINARY_DIR}
    "imgui"
    ${Box2D_SOURCE_DIR}/Box2D)

if (WIN32)
    vortex2d_copy_dll(VortexEditor)
endif()
