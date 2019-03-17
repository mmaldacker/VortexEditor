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

struct Entity
{
    std::unique_ptr<Vortex2D::Renderer::Shape> mShape;
    ShapeType mShapeType;
    Vortex2D::Renderer::RenderCommand mCmd;
    std::string mId;
    std::unique_ptr<Rigidbody> mRigidbody;

    void MakeRigidbody(const Vortex2D::Renderer::Device& device,
                       const glm::ivec2& size,
                       Vortex2D::Fluid::RigidBody::Type type,
                       b2World& box2dWorld,
                       b2BodyType box2dType,
                       float scale)
    {
        mShapeType.match(
            [&](const Circle& circle)
            {
                auto radius = circle.mRadius / scale;
                mRigidbody = std::make_unique<CircleRigidbody>(device,
                                                               size,
                                                               box2dWorld,
                                                               box2dType,
                                                               type,
                                                               radius);
            },
            [&](const Rectangle& rectangle)
            {
                auto size = rectangle.mSize / scale;
                mRigidbody = std::make_unique<RectangleRigidbody>(device,
                                                                  size,
                                                                  box2dWorld,
                                                                  box2dType,
                                                                  type,
                                                                  size);
            },
            [&](const Polygon& /*polygon*/)
            {

            });

        glm::vec2 pos = mShape->Position;
        float angle = mShape->Rotation;
        mRigidbody->SetTransform(pos / scale, angle);
    }
};

inline Entity Create(const std::string& id,
                     ShapeType type,
                     std::unique_ptr<Vortex2D::Renderer::Shape> shape,
                     Vortex2D::Renderer::RenderCommand cmd)
{
    return {std::move(shape), type, std::move(cmd), id, {}};
}
