#include "shapebuilder.h"

void ShapeBuilder::Render(Vortex2D::Renderer::RenderTarget& target)
{
    auto& io = ImGui::GetIO();

    static int type = 0;
    static glm::vec4 color(1.0f);
    static glm::vec2 size(0.0f);
    static char nameBuffer[20] = {0};

    if (!io.WantCaptureMouse && io.MouseDown[0])
    {
        size = glm::abs(glm::vec2{io.MouseClickedPos[0].x - io.MousePos.x, io.MouseClickedPos[0].y - io.MousePos.y});
        if (type == 0)
        {
            mBuildShape = std::make_shared<Vortex2D::Renderer::Ellipse>(mDevice, size);
        }
        else if (type == 1)
        {
            mBuildShape = std::make_shared<Vortex2D::Renderer::Rectangle>(mDevice, size);
        }

        mBuildShape->Position = {io.MouseClickedPos[0].x, io.MouseClickedPos[0].y};
        mBuildShape->Colour = color;
        mBuildCmd = target.Record({*mBuildShape});
    }

    if (ImGui::Begin("Shape builder", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::RadioButton("Circle", &type, 0);
        ImGui::RadioButton("Rectangle", &type, 1);
        ImGui::ColorPicker4("Color", &color.r);
        ImGui::InputText("Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));
        if (ImGui::Button("Create"))
        {
            mShapes.push_back({mBuildShape, std::move(mBuildCmd), {nameBuffer}});
        }
        ImGui::End();
    }

    if (ImGui::Begin("Shape list", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        for (auto& shape: mShapes)
        {
            ImGui::Text("Name: %s", shape.mName.c_str());
        }
        ImGui::End();
    }

    mBuildCmd.Submit();

    for (auto& shape: mShapes)
    {
        shape.mCmd.Submit();
    }
}
