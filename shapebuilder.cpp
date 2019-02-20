#include "shapebuilder.h"

void ShapeBuilder::Render(Vortex2D::Renderer::RenderTarget& target)
{
    auto& io = ImGui::GetIO();

    static int type = 0;
    static glm::vec4 color(1.0f);

    if (ImGui::Begin("Shape builder", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::RadioButton("Circle", &type, 0);
        ImGui::RadioButton("Rectangle", &type, 1);
        ImGui::ColorPicker4("Color", &color.r);
        ImGui::End();
    }

    static glm::vec2 size;
    static std::shared_ptr<Vortex2D::Renderer::Shape> shape;
    static Vortex2D::Renderer::RenderCommand cmd;

    if (!io.WantCaptureMouse && io.MouseDown[0])
    {
        size = glm::abs(glm::vec2{io.MouseClickedPos[0].x - io.MousePos.x, io.MouseClickedPos[0].y - io.MousePos.y});
        if (type == 0)
        {
            shape = std::make_shared<Vortex2D::Renderer::Ellipse>(mDevice, size);

        }
        else if (type == 1)
        {
            shape = std::make_shared<Vortex2D::Renderer::Rectangle>(mDevice, size);
        }

        shape->Position = {io.MouseClickedPos[0].x, io.MouseClickedPos[0].y};
        shape->Colour = color;
        cmd = target.Record({*shape});
    }

    if (!io.WantCaptureMouse && io.MouseReleased[0])
    {
        mShapes.push_back(shape);
        mCmds.push_back(std::move(cmd));
        shape.reset();
        cmd = Vortex2D::Renderer::RenderCommand();
    }

    cmd.Submit();

    for (auto& cmd: mCmds)
    {
        cmd.Submit();
    }
}
