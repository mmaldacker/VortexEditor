cmake_minimum_required(VERSION 3.11)

project(VortexEditor)
set(CMAKE_CXX_STANDARD 14)

# cmake file to download and include dependencies
include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
if (APPLE)
    set(GLFW_VULKAN_STATIC ON  CACHE BOOL "" FORCE)
endif()
FetchContent_Declare(glfw3
                     GIT_REPOSITORY      https://github.com/glfw/glfw.git
                     GIT_TAG             master)
if(NOT glfw3_POPULATED)
  FetchContent_Populate(glfw3)
  add_subdirectory(${glfw3_SOURCE_DIR} ${glfw3_BINARY_DIR})
endif()

FetchContent_Declare(box2d
                     GIT_REPOSITORY      https://github.com/erincatto/Box2D.git
                     GIT_TAG             v2.3.1)

set(BOX2D_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
if(NOT box2d_POPULATED)
  FetchContent_Populate(box2d)
  add_subdirectory(${box2d_SOURCE_DIR}/Box2D ${box2d_BINARY_DIR})
endif()

FetchContent_Declare(vortex2d
                     GIT_REPOSITORY      https://github.com/mmaldacker/Vortex2D.git
                     GIT_TAG             master)
if(NOT vortex2d_POPULATED)
  FetchContent_Populate(vortex2d)
  add_subdirectory(${vortex2d_SOURCE_DIR} ${vortex2d_BINARY_DIR})
endif()

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
    world.h
    world.cpp
    rigidbody.h
    rigidbody.cpp
    entity.h
    entity.cpp
    ui.h
    ui.cpp
    ${IMGUI_SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/vortexeditor_spirv.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/vortexeditor_spirv.h
)

target_link_libraries(VortexEditor glfw vortex2d Box2D glm)

# Ensure glfw is included first
target_include_directories(VortexEditor PRIVATE
    $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
    ${CMAKE_CURRENT_BINARY_DIR}
    "imgui"
    ${box2d_SOURCE_DIR}/Box2D)

if (WIN32)
    vortex2d_copy_dll(VortexEditor)
endif()
