#pragma once

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Utils/mapbox/variant.hpp>
#include "rigidbody.h"

struct Circle
{
  float mRadius;
};

struct Rectangle
{
  glm::vec2 mSize;
};

struct Polygon
{
  std::vector<glm::vec2> mVertices;
};

using ShapeType = mapbox::util::variant<Circle, Rectangle, Polygon>;
bool IsValid(const ShapeType& type);

struct Entity
{
  float mScale;
  ShapeType mShapeType;
  std::unique_ptr<Vortex2D::Renderer::Shape> mShape;
  Vortex2D::Renderer::RenderCommand mCmd;
  std::unique_ptr<Rigidbody> mRigidbody;

  Entity(const Vortex2D::Renderer::Device& device,
         const glm::ivec2& size,
         float scale,
         ShapeType type,
         std::unique_ptr<Vortex2D::Renderer::Shape> shape,
         Vortex2D::Renderer::RenderCommand cmd,
         b2World& box2dWorld);

  void SetTransform(const glm::vec2& pos, float angle);
  void UpdateTransform();
};

using EntityPtr = std::unique_ptr<Entity>;
