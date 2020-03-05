#include "world.h"

class UI
{
public:
  UI(const Vortex2D::Renderer::Device& device, World& world, const glm::ivec2& size, float scale);

  void Update(Vortex2D::Renderer::RenderTarget& target);

private:
  void RenderFluid();
  void CreateShape();
  void ResizeShape(Vortex2D::Renderer::RenderTarget& target);
  void MoveShape(Entity& entity);

  const Vortex2D::Renderer::Device& mDevice;
  glm::ivec2 mSize;
  float mScale;

  World& mWorld;

  Entity* mCurrentEntity;

  ShapeType mShapeType;
  std::unique_ptr<Vortex2D::Renderer::Shape> mBuildShape;
  Vortex2D::Renderer::RenderCommand mBuildCmd;

  Vortex2D::Renderer::Rectangle mForce;
  std::shared_ptr<Vortex2D::Renderer::IntRectangle> mFluid;
};
