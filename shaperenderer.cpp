#include "shaperenderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>

struct QueryCallback : b2QueryCallback
{
    bool ReportFixture(b2Fixture* fixture) override
    {
        mContactFixture = fixture;
        return false;
    }

    b2Fixture* mContactFixture = nullptr;
};

ShapeRenderer::ShapeRenderer(const Vortex2D::Renderer::Device& device,
                             const glm::ivec2& size,
                             float scale,
                             std::vector<EntityPtr>& entities,
                             b2World& box2dWorld,
                             Vortex2D::Fluid::World& world)
    : mDevice(device)
      , mSize(size)
      , mScale(scale)
      , mEntities(entities)
      , mBox2dWorld(box2dWorld)
      , mWorld(world)
{
}

void ShapeRenderer::Render(Vortex2D::Renderer::RenderTarget& target)
{
    auto& io = ImGui::GetIO();

    static int shapeIndex = 1;
    static glm::vec4 color = glm::vec4(92.0f, 173.0f, 159.0f, 255.0f) / glm::vec4(255.0f);;
    static uint64_t id = 0;
    static ShapeType shapeType = Rectangle{};
    static b2Fixture* currentFixture = nullptr;

    if (ImGui::Begin("Entity Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::RadioButton("Circle", &shapeIndex, 0))
        {
            shapeType = Circle{};
        }
        if (ImGui::RadioButton("Rectangle", &shapeIndex, 1))
        {
            shapeType = Rectangle{};
        }
        ImGui::End();
    }

    if (!io.WantCaptureMouse && io.MouseDown[0])
    {
        if (currentFixture == nullptr)
        {
            QueryCallback callback;
            b2Vec2 mousePos = {io.MousePos.x / mScale, io.MousePos.y / mScale};
            mBox2dWorld.QueryAABB(&callback, {mousePos, mousePos});
            currentFixture = callback.mContactFixture;
        }

        if (currentFixture != nullptr)
        {
            auto& entity = *static_cast<Entity*>(currentFixture->GetBody()->GetUserData());
            glm::vec2 centre = entity.mShape->Position;

            if (io.KeysDown[GLFW_KEY_LEFT_CONTROL])
            {
                // rotation
                glm::vec2 v1 = {io.MouseClickedPos[0].x - centre.x, io.MouseClickedPos[0].y - centre.y};
                glm::vec2 v2 = {io.MousePos.x - centre.x, io.MousePos.y - centre.y};
                auto angle = glm::orientedAngle(glm::normalize(v1), glm::normalize(v2));
                entity.SetTransform(centre, glm::degrees(angle));
            }
            else
            {
                // translation
                auto delta = glm::vec2{io.MouseDelta.x, io.MouseDelta.y};
                entity.SetTransform(centre + delta, entity.mShape->Rotation);
            }
        }
        else
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
                [&](Polygon& /*polygon*/)
                {

                });

            mBuildShape->Position = {io.MouseClickedPos[0].x, io.MouseClickedPos[0].y};
            mBuildShape->Colour = color;
            mBuildCmd = target.Record({*mBuildShape});
        }
    }

    if (!io.WantCaptureMouse && io.MouseReleased[0])
    {
        currentFixture = nullptr;
    }

    if (!io.WantCaptureMouse && io.MouseReleased[0] && mBuildShape && IsValid(shapeType))
    {
        EntityPtr entity = std::make_unique<Entity>(mDevice,
                                                    mSize,
                                                    mScale,
                                                    std::to_string(++id),
                                                    shapeType,
                                                    std::move(mBuildShape),
                                                    std::move(mBuildCmd),
                                                    mBox2dWorld);

        mWorld.AddRigidbody(*entity->mRigidbody->mRigidbody);
        mEntities.push_back(std::move(entity));
    }

    mBuildCmd.Submit();
    for (auto& entity: mEntities)
    {
        entity->mCmd.Submit();
    }
}
