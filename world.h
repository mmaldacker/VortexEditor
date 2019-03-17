#pragma once

#include <Vortex2D/Vortex2D.h>
#include "rigidbody.h"
#include "entity.h"

class World
{
public:
    World(const Vortex2D::Renderer::Device& device, const glm::ivec2& size, float scale, std::vector<Entity>& entities);

    void Record(Vortex2D::Renderer::RenderTarget& target, Vortex2D::Renderer::ColorBlendState blendState);
    void Render();
private:
    const Vortex2D::Renderer::Device& mDevice;
    glm::ivec2 mSize;
    float mScale;
    Vortex2D::Fluid::WaterWorld mWorld;
    b2World mBox2DWorld;
    Box2DSolver mBox2DSolver;
    std::vector<Entity>& mEntities;
    Vortex2D::Renderer::Rectangle mGravity;
    Vortex2D::Fluid::DistanceField mLiquidPhi;
    Vortex2D::Renderer::RenderCommand mVelocityRender, mWindowRender;
};
