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

add_executable(VortexEditor
    main.cpp
)

target_link_libraries(VortexEditor vortex2d)

if (WIN32)
    vortex2d_copy_dll(VortexEditor)
endif()
