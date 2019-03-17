#include "shaperenderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

#include <GLFW/glfw3.h>

ShapeRenderer::ShapeRenderer(const Vortex2D::Renderer::Device& device, std::vector<Entity>& entities)
    : mDevice(device)
    , mEntities(entities)
{
}

void ShapeRenderer::Render(Vortex2D::Renderer::RenderTarget& target)
{
    auto& io = ImGui::GetIO();

    static int type = 1;
    static int shapeIndex = 1;
    static glm::vec4 color = glm::vec4(92.0f, 173.0f, 159.0f, 255.0f) / glm::vec4(255.0f);;
    static uint64_t id = 0;
    static ShapeType shapeType = Rectangle{};
    static int currentShapeIndex = 0;

    if (ImGui::Begin("Entity Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::RadioButton("Move", &type, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Create", &type, 1);
        ImGui::Separator();

        if (type == 0)
        {
            ImGui::Combo("Shapes", &currentShapeIndex, +[](void* data, int idx, const char** outStr) {
                *outStr = ((Entity*)data)[idx].mId.c_str();
                return true;
            }, mEntities.data(), mEntities.size());
        }
        else if (type == 1)
        {
            ImGui::PushID(1);
            if (ImGui::RadioButton("Circle", &shapeIndex, 0))
            {
                shapeType = Circle{};
            }
            if (ImGui::RadioButton("Rectangle", &shapeIndex, 1))
            {
                shapeType = Rectangle{};
            }
            ImGui::ColorPicker4("Color", &color.r);
            if (ImGui::Button("Create"))
            {
                auto entity = Create(std::to_string(++id), shapeType, std::move(mBuildShape), std::move(mBuildCmd));
                mEntities.push_back(std::move(entity));
            }
            ImGui::PopID();
        }
        ImGui::End();
    }

    if (type == 0 && !io.WantCaptureMouse && io.MouseDown[0])
    {
        auto& currentShape = *mEntities[currentShapeIndex].mShape;
        if (io.KeysDown[GLFW_KEY_LEFT_CONTROL])
        {
            // rotation
            glm::vec2 centre = currentShape.Position;
            glm::vec2 v1 = {io.MouseClickedPos[0].x - centre.x, io.MouseClickedPos[0].y - centre.y};
            glm::vec2 v2 = {io.MousePos.x - centre.x, io.MousePos.y - centre.y};
            auto angle = glm::orientedAngle(glm::normalize(v1), glm::normalize(v2));
            currentShape.Rotation = glm::degrees(angle);
        }
        else
        {
            // translation
            auto delta = glm::vec2{io.MouseDelta.x, io.MouseDelta.y};
            currentShape.Position += delta;
        }
    }

    if (type == 1 && !io.WantCaptureMouse && io.MouseDown[0])
    {
        auto size = glm::abs(glm::vec2{io.MouseClickedPos[0].x - io.MousePos.x, io.MouseClickedPos[0].y - io.MousePos.y});
        shapeType.match(
        [&](Rectangle& rectangle)
        {
                mBuildShape = std::make_unique<Vortex2D::Renderer::Rectangle>(mDevice, size * glm::vec2(2.0f));
                mBuildShape->Anchor = size;
                rectangle.mSize = size;
        },
        [&](Circle& circle)
        {
                auto radius = glm::length(size);
                mBuildShape = std::make_unique<Vortex2D::Renderer::Ellipse>(mDevice, glm::vec2(radius, radius));
                circle.mRadius = radius;
        },
        [&](Polygon& polygon)
        {

        });

        mBuildShape->Position = {io.MouseClickedPos[0].x, io.MouseClickedPos[0].y};
        mBuildShape->Colour = color;
        mBuildCmd = target.Record({*mBuildShape});
    }

    mBuildCmd.Submit();
    for (auto& entity: mEntities)
    {
        entity.mCmd.Submit();
    }
}
