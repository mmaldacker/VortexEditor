#pragma once

#include <Vortex2D/Vortex2D.h>
#include "shapemanager.h"

class World
{
public:
    World(const Vortex2D::Renderer::Device& device, const glm::ivec2& size, std::vector<Shape>& shapes);

    void Record(Vortex2D::Renderer::RenderTarget& target, Vortex2D::Renderer::ColorBlendState blendState);
    void Render();
private:
    Vortex2D::Fluid::WaterWorld mWorld;
    std::vector<Shape>& mShapes;
    Vortex2D::Renderer::Rectangle mGravity;
    Vortex2D::Fluid::DistanceField mLiquidPhi;
    Vortex2D::Renderer::RenderCommand mVelocityRender, mWindowRender;
};
