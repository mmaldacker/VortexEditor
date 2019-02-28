#include "shapemanager.h"
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

ShapeManager::ShapeManager(const Vortex2D::Renderer::Device& device, std::vector<Shape>& shapes)
    : mDevice(device)
    , mShapes(shapes)
{
}

void ShapeManager::Render(Vortex2D::Renderer::RenderTarget& target)
{
    auto& io = ImGui::GetIO();

    static int type = 1;
    static int shapeType = 0;
    static glm::vec4 color = glm::vec4(92.0f, 173.0f, 159.0f, 255.0f) / glm::vec4(255.0f);;
    static std::array<char, 20> nameBuffer = {"shape0"};
    static glm::vec2 currentSize = {0.0f, 0.0f};
    static Shape::Type currentType;
    static int currentShapeIndex = 0;

    if (ImGui::Begin("Shape Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::RadioButton("Move", &type, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Create", &type, 1);
        ImGui::Separator();

        if (type == 0)
        {
            ImGui::Combo("Shapes", &currentShapeIndex, +[](void* data, int idx, const char** outStr) {
                *outStr = ((Shape*)data)[idx].mName.c_str();
                return true;
            }, mShapes.data(), mShapes.size());
        }
        else if (type == 1)
        {
            ImGui::PushID(1);
            ImGui::RadioButton("Circle", &shapeType, 0);
            ImGui::RadioButton("Rectangle", &shapeType, 1);
            ImGui::ColorPicker4("Color", &color.r);
            ImGui::InputText("Name", nameBuffer.data(), nameBuffer.size());
            if (ImGui::Button("Create"))
            {
                mShapes.push_back({mBuildShape, std::move(mBuildCmd), {nameBuffer.data()}, currentSize, currentType});
                auto name = "shape" + std::to_string(mShapes.size());
                std::copy(name.begin(), name.end(), nameBuffer.data());
            }
            ImGui::PopID();
        }
        ImGui::End();
    }

    if (type == 0 && !io.WantCaptureMouse && io.MouseDown[0])
    {
        auto currentShape = mShapes[currentShapeIndex].mShape;
        if (io.KeysDown[GLFW_KEY_LEFT_CONTROL])
        {
            // rotation
            glm::vec2 centre = currentShape->Position;
            glm::vec2 v1 = {io.MouseClickedPos[0].x - centre.x, io.MouseClickedPos[0].y - centre.y};
            glm::vec2 v2 = {io.MousePos.x - centre.x, io.MousePos.y - centre.y};
            auto angle = glm::orientedAngle(glm::normalize(v1), glm::normalize(v2));
            currentShape->Rotation = glm::degrees(angle);
        }
        else
        {
            // translation
            auto size = glm::vec2{io.MouseDelta.x, io.MouseDelta.y};
            currentShape->Position = size + currentShape->Position;
        }
    }

    if (type == 1 && !io.WantCaptureMouse && io.MouseDown[0])
    {
        auto size = glm::abs(glm::vec2{io.MouseClickedPos[0].x - io.MousePos.x, io.MouseClickedPos[0].y - io.MousePos.y});
        if (shapeType == 0)
        {
            auto radius = glm::length(size);
            mBuildShape = std::make_shared<Vortex2D::Renderer::Ellipse>(mDevice, glm::vec2(radius, radius));
            currentSize = {radius, radius};
            currentType = Shape::Type::Circle;
        }
        else if (shapeType == 1)
        {
            mBuildShape = std::make_shared<Vortex2D::Renderer::Rectangle>(mDevice, size * glm::vec2(2.0f));
            mBuildShape->Anchor = size;
            currentSize = size;
            currentType = Shape::Type::Rectangle;
        }

        mBuildShape->Position = {io.MouseClickedPos[0].x, io.MouseClickedPos[0].y};
        mBuildShape->Colour = color;
        mBuildCmd = target.Record({*mBuildShape});
    }

    mBuildCmd.Submit();
    for (auto& shape: mShapes)
    {
        shape.mCmd.Submit();
    }
}
