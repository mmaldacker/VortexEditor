#include <Vortex2D/Vortex2D.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <chrono>
#include "imguirenderer.h"
#include "shapemanager.h"
#include "world.h"

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

void ErrorCallback(int error, const char* description)
{
    throw std::runtime_error("GLFW Error: " +
                             std::to_string(error) +
                             " What: " +
                             std::string(description));
}

static bool g_MouseJustPressed[5] = { false, false, false, false, false };

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(g_MouseJustPressed))
        g_MouseJustPressed[button] = true;
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += (float)xoffset;
    io.MouseWheel += (float)yoffset;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)
        io.KeysDown[key] = true;
    if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;

    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void CharCallback(GLFWwindow* window, unsigned int c)
{
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

void UpdateInput(GLFWwindow* glfwWindow)
{
    auto& io = ImGui::GetIO();
    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
    {
        io.MouseDown[i] = g_MouseJustPressed[i] || glfwGetMouseButton(glfwWindow, i) != 0;
        g_MouseJustPressed[i] = false;
    }

    const ImVec2 mouse_pos_backup = io.MousePos;
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

    if (glfwGetWindowAttrib(glfwWindow, GLFW_FOCUSED) != 0)
    {
        if (io.WantSetMousePos)
        {
            glfwSetCursorPos(glfwWindow, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
        }
        else
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(glfwWindow, &mouse_x, &mouse_y);
            io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
        }
    }
}

int main(int argc, char** argv)
{
    glfwInit();

    glfwSetErrorCallback(ErrorCallback);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    bool validation = true;
    glm::ivec2 windowSize = {1024, 1024};
    glm::ivec2 fluidSize = {256, 256};
    GLFWwindow* glfwWindow = GetGLFWWindow(windowSize);
    glm::vec2 scale = GetGLFWindowScale(glfwWindow);
    float fluidScale = 4.0f * scale.x;

    {
        Vortex2D::Renderer::Instance instance("VulkanEditor", GetGLFWExtensions(), validation);
        vk::SurfaceKHR surface(GetGLFWSurface(glfwWindow, static_cast<VkInstance>(instance.GetInstance())));
        Vortex2D::Renderer::Device device(instance.GetPhysicalDevice(), surface, validation);
        Vortex2D::Renderer::RenderWindow window(device, surface, windowSize.x * scale.x, windowSize.y * scale.y);

        glfwSetMouseButtonCallback(glfwWindow, MouseButtonCallback);
        glfwSetScrollCallback(glfwWindow, ScrollCallback);
        glfwSetKeyCallback(glfwWindow, KeyCallback);
        glfwSetCharCallback(glfwWindow, CharCallback);

        ImGui::CreateContext();
        ImGui::StyleColorsClassic();
        auto& io = ImGui::GetIO();

        io.DisplaySize = ImVec2(windowSize.x * scale.x, windowSize.y * scale.y);
        io.FontGlobalScale = scale.x;
        io.DeltaTime = 1.0f/60.0f;

        Vortex2D::Renderer::Clear clear(glm::vec4(0.1f));
        auto clearCmd = window.Record({clear});
        ImGuiRenderer renderer(device);
        Vortex2D::Renderer::RenderCommand imguiCmd;

        std::vector<Shape> shapes;
        ShapeManager shapeManager(device, shapes);
        World world(device, fluidSize, fluidScale, shapes);

        Vortex2D::Renderer::ColorBlendState blendState;
        blendState.ColorBlend
            .setBlendEnable(true)
            .setAlphaBlendOp(vk::BlendOp::eAdd)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

        world.Record(window, blendState);
        imguiCmd = window.Record({renderer}, blendState);

        constexpr int32_t timeWindowSize = 200;
        float timePoints[timeWindowSize] = {0.0f};
        int timePointIndex = 0;

        while(!glfwWindowShouldClose(glfwWindow))
        {
            auto start = std::chrono::system_clock::now();

            glfwPollEvents();
            UpdateInput(glfwWindow);
            clearCmd.Submit();

            ImGui::NewFrame();

            if (ImGui::Begin("Frame timing", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::PlotLines("", timePoints, timeWindowSize, timePointIndex, nullptr, 0.0f, 40.0f, ImVec2(0, 80));
                ImGui::End();
            }

            shapeManager.Render(window);
            world.Render();

            ImGui::Render();

            imguiCmd.Wait();
            renderer.Update();

            imguiCmd = window.Record({renderer}, blendState);
            imguiCmd.Submit();
            window.Display();

            auto end = std::chrono::system_clock::now();
            timePoints[timePointIndex] = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            timePointIndex = (timePointIndex + 1) % timeWindowSize;
        }

        ImGui::DestroyContext();
    }

    glfwTerminate();
}
