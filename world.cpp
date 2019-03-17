#include "world.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <imgui.h>

namespace
{
b2BodyType GetBox2DType(int type)
{
    switch (type)
    {
    case 0:
        return b2_staticBody;
    case 1:
        return b2_dynamicBody;
    default:
        return b2_staticBody;
    }
}

Vortex2D::Fluid::RigidBody::Type GetVortex2DType(int type)
{
    switch (type)
    {
    case 0:
        return Vortex2D::Fluid::RigidBody::Type::eStatic;
    case 1:
        return Vortex2D::Fluid::RigidBody::Type::eWeak;
    case 2:
        return Vortex2D::Fluid::RigidBody::Type::eStrong;
    default:
        return Vortex2D::Fluid::RigidBody::Type::eStatic;
    }
}

const float gravityForce = 100.0f;

}

World::World(const Vortex2D::Renderer::Device& device, const glm::ivec2& size, float scale, std::vector<EntityPtr>& entities)
    : mDevice(device)
    , mSize(size)
    , mScale(scale)
    , mWorld(device, size, ImGui::GetIO().DeltaTime, 2)
    , mBox2DWorld(b2Vec2(0.0f, gravityForce))
    , mBox2DSolver(mBox2DWorld)
    , mEntities(entities)
    , mGravity(device, size)
    , mLiquidPhi(mWorld.LiquidDistanceField())
{
    mWorld.AttachRigidBodySolver(mBox2DSolver);
    mGravity.Colour = {0.0f, ImGui::GetIO().DeltaTime * gravityForce, 0.0f, 0.0f};
    mLiquidPhi.Colour = glm::vec4(36.0f, 123.0f, 160.0f, 255.0f) / glm::vec4(255.0f);
    mVelocityRender = mWorld.RecordVelocity({mGravity}, Vortex2D::Fluid::VelocityOp::Add);
}

b2World& World::GetBox2dWorld()
{
    return mBox2DWorld;
}

Vortex2D::Fluid::World& World::GetWorld()
{
    return mWorld;
}

void World::Record(Vortex2D::Renderer::RenderTarget& target, Vortex2D::Renderer::ColorBlendState blendState)
{
    mWindowRender = target.Record({mLiquidPhi}, blendState);
}

void World::Render()
{
    static int currentShapeIndex = 0;
    static int box2dType = 0;
    static int vortex2dType = 0;

    if (ImGui::Begin("World Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Combo("Shapes", &currentShapeIndex, +[](void* data, int idx, const char** outStr) {
            *outStr = static_cast<EntityPtr*>(data)[idx]->mId.c_str();
            return true;
        }, mEntities.data(), mEntities.size());

        ImGui::PushID("box2dtype");
        ImGui::Text("Box2D Type");
        ImGui::RadioButton("Static", &box2dType, 0);
        ImGui::RadioButton("Dynamic", &box2dType, 1);
        ImGui::PopID();
        ImGui::PushID("vortex2dtype");
        ImGui::Text("Vortex2D Type");
        ImGui::RadioButton("Static", &vortex2dType, 0);
        ImGui::RadioButton("Weak", &vortex2dType, 1);
        ImGui::RadioButton("Strong", &vortex2dType, 2);
        ImGui::PopID();
        if (ImGui::Button("Update"))
        {
            auto& entity = mEntities[currentShapeIndex];
            entity->mRigidbody->mBody->SetType(GetBox2DType(box2dType));
            entity->mRigidbody->mRigidbody->SetType(GetVortex2DType(vortex2dType));
            entity->mShape->Colour = glm::vec4(208.0f, 43.0f, 10.0f, 255.0f) / glm::vec4(255.0f);
        }

        ImGui::End();
    }

    auto& io = ImGui::GetIO();
    if (!io.WantCaptureMouse && io.MouseClicked[1])
    {
        glm::vec2 radius(32.0f);
        Vortex2D::Renderer::IntRectangle fluid(mDevice, radius);
        fluid.Position = {io.MouseClickedPos[1].x / mScale, io.MouseClickedPos[1].y / mScale};
        fluid.Anchor = {16.0f, 16.0f};
        fluid.Colour = glm::vec4(4);

        mWorld.RecordParticleCount({fluid}).Submit().Wait();
    }

    mWorld.SubmitVelocity(mVelocityRender);
    auto params = Vortex2D::Fluid::FixedParams(12);
    mWorld.Step(params);

    for (auto& entity: mEntities)
    {
        if (entity->mRigidbody->mBody->GetType() == b2_dynamicBody)
        {
            entity->UpdateTransform();
        }
    }

    mWindowRender.Submit(glm::scale(glm::vec3(mScale, mScale, 1.0f)));
}
