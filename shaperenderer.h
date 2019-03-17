#pragma once

#include <Vortex2D/Vortex2D.h>
#include <imgui.h>
#include "entity.h"

class ShapeRenderer
{
public:
    ShapeRenderer(const Vortex2D::Renderer::Device& device,
                  const glm::ivec2& size,
                  float scale,
                  std::vector<EntityPtr>& entities,
                  b2World& box2dWorld,
                  Vortex2D::Fluid::World& world);

    void Render(Vortex2D::Renderer::RenderTarget& target);

private:
    const Vortex2D::Renderer::Device& mDevice;
    glm::ivec2 mSize;
    float mScale;
    std::vector<EntityPtr>& mEntities;

    Vortex2D::Fluid::World& mWorld;
    b2World& mBox2dWorld;

    std::unique_ptr<Vortex2D::Renderer::Shape> mBuildShape;
    Vortex2D::Renderer::RenderCommand mBuildCmd;
};
