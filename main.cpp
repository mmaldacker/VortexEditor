#include <Vortex2D/Vortex2D.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "imguirenderer.h"

void ErrorCallback(int error, const char* description)
{
    throw std::runtime_error("GLFW Error: " +
                             std::to_string(error) +
                             " What: " +
                             std::string(description));
}

std::vector<const char*> GetGLFWExtensions()
{
    std::vector<const char*> extensions;
    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions;

    // get the required extensions from GLFW
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (unsigned int i = 0; i < glfwExtensionCount; i++)
    {
        extensions.push_back(glfwExtensions[i]);
    }

    return extensions;
}

vk::SurfaceKHR GetGLFWSurface(GLFWwindow* window, vk::Instance instance)
{
    // create surface
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }

    return surface;
}

glm::vec2 GetGLFWindowScale(GLFWwindow* window)
{
    glm::vec2 scale;
    glfwGetWindowContentScale(window, &scale.x, &scale.y);

    return scale;
}

GLFWwindow* GetGLFWWindow(const glm::ivec2& size)
{
    GLFWwindow* window = glfwCreateWindow(size.x, size.y, "Vortex2D App", nullptr, nullptr);
    if (window == nullptr)
    {
        throw std::runtime_error("Could not create glfw window");
    }

    glm::vec2 scale = GetGLFWindowScale(window);
    glfwSetWindowSize(window, size.x * scale.x, size.y * scale.y);

    return window;
}

int main(int argc, char** argv)
{
    glfwInit();

    glfwSetErrorCallback(ErrorCallback);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    bool validation = true;
    glm::ivec2 windowSize = {1024, 1024};
    GLFWwindow* glfwWindow = GetGLFWWindow(windowSize);
    glm::vec2 scale = GetGLFWindowScale(glfwWindow);
    Vortex2D::Renderer::Instance instance("VulkanEditor", GetGLFWExtensions(), validation);
    vk::SurfaceKHR surface(GetGLFWSurface(glfwWindow, static_cast<VkInstance>(instance.GetInstance())));
    Vortex2D::Renderer::Device device(instance.GetPhysicalDevice(), surface, validation);
    Vortex2D::Renderer::RenderWindow window(device, surface, windowSize.x * scale.x, windowSize.y * scale.y);

    ImGui::CreateContext();

    ImGuiRenderer renderer(device);

    while(!glfwWindowShouldClose(glfwWindow))
    {
        glfwPollEvents();
        ImGui::NewFrame();

        ImGui::EndFrame();
        ImGui::Render();

        window.Record({renderer}).Submit();
        window.Display();
    }

    ImGui::DestroyContext();
    glfwTerminate();
}
