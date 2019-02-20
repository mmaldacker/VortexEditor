#pragma once

#include <Vortex2D/Vortex2D.h>
#include <imgui.h>

class ShapeBuilder
{
public:
    ShapeBuilder(const Vortex2D::Renderer::Device& device)
        : mDevice(device)
    {

    }

    void Render(Vortex2D::Renderer::RenderTarget& target);

private:
    const Vortex2D::Renderer::Device& mDevice;
    std::vector<std::shared_ptr<Vortex2D::Renderer::Shape>> mShapes;
    std::vector<Vortex2D::Renderer::RenderCommand> mCmds;
};
