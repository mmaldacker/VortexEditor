#include "world.h"

World::World(const Vortex2D::Renderer::Device& device, const glm::ivec2& size, std::vector<Shape>& shapes)
    : mWorld(device, size, ImGui::GetIO().DeltaTime, 2)
    , mShapes(shapes)
    , mGravity(device, size)
    , mLiquidPhi(mWorld.LiquidDistanceField())

{
    mGravity.Colour = {0.0f, 3.0f, 0.0f, 0.0f};
    mLiquidPhi.Colour = glm::vec4(36.0f, 123.0f, 160.0f, 255.0f) / glm::vec4(255.0f);
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
            *outStr = ((Shape*)data)[idx].mName.c_str();
            return true;
        }, mShapes.data(), mShapes.size());

        ImGui::PushID("box2dtype");
        ImGui::Text("Box2D Type");
        ImGui::RadioButton("Dynamic", &box2dType, 0);
        ImGui::RadioButton("Static", &box2dType, 1);
        ImGui::PopID();
        ImGui::PushID("vortex2dtype");
        ImGui::Text("Vortex2D Type");
        ImGui::RadioButton("Static", &vortex2dType, 0);
        ImGui::RadioButton("Weak", &vortex2dType, 1);
        ImGui::RadioButton("Strong", &vortex2dType, 2);
        ImGui::PopID();
        if (ImGui::Button("Add"))
        {

        }
        ImGui::End();
    }

    mWorld.SubmitVelocity(mVelocityRender);
    auto params = Vortex2D::Fluid::FixedParams(12);
    mWorld.Step(params);

    mWindowRender.Submit();
}
