#pragma once

#include <Vortex2D/Vortex2D.h>
#include <imgui.h>
#include "entity.h"

class ShapeRenderer
{
public:
    ShapeRenderer(const Vortex2D::Renderer::Device& device, std::vector<Entity>& entities);

    void Render(Vortex2D::Renderer::RenderTarget& target);

private:
    const Vortex2D::Renderer::Device& mDevice;
    std::vector<Entity>& mEntities;

    std::unique_ptr<Vortex2D::Renderer::Shape> mBuildShape;
    Vortex2D::Renderer::RenderCommand mBuildCmd;
};
