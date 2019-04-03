#pragma once

#include <Vortex2D/Vortex2D.h>
#include "entity.h"
#include "rigidbody.h"

class World
{
public:
  World(const Vortex2D::Renderer::Device& device,
        const glm::ivec2& size,
        float scale,
        std::vector<EntityPtr>& entities);

  b2World& GetBox2dWorld();
  Vortex2D::Fluid::World& GetWorld();

  void Record(Vortex2D::Renderer::RenderTarget& target,
              Vortex2D::Renderer::ColorBlendState blendState);
  void Render();

private:
  const Vortex2D::Renderer::Device& mDevice;
  glm::ivec2 mSize;
  float mScale;
  Vortex2D::Fluid::WaterWorld mWorld;
  b2World mBox2DWorld;
  Box2DSolver mBox2DSolver;
  std::vector<EntityPtr>& mEntities;
  Vortex2D::Renderer::Rectangle mGravity;
  Vortex2D::Fluid::DistanceField mLiquidPhi;
  Vortex2D::Renderer::Rectangle mForce;
  std::shared_ptr<Vortex2D::Renderer::IntRectangle> mFluid;
  Vortex2D::Renderer::RenderCommand mVelocityRender, mWindowRender;
};
