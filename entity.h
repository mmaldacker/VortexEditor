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
    float mScale;
    std::string mId;
    ShapeType mShapeType;
    std::unique_ptr<Vortex2D::Renderer::Shape> mShape;
    Vortex2D::Renderer::RenderCommand mCmd;
    std::unique_ptr<Rigidbody> mRigidbody;

    Entity(const Vortex2D::Renderer::Device& device,
           const glm::ivec2& size,
           float scale,
           const std::string& id,
           ShapeType type,
           std::unique_ptr<Vortex2D::Renderer::Shape> shape,
           Vortex2D::Renderer::RenderCommand cmd,
           b2World& box2dWorld)
        : mScale(scale)
        , mId(id)
        , mShapeType(type)
        , mShape(std::move(shape))
        , mCmd(std::move(cmd))
    {
        mShapeType.match(
            [&](const Circle& circle)
            {
                auto radius = circle.mRadius / scale;
                mRigidbody = std::make_unique<CircleRigidbody>(device,
                                                               size,
                                                               box2dWorld,
                                                               b2_staticBody,
                                                               Vortex2D::Fluid::RigidBody::Type::eStatic,
                                                               radius);
            },
            [&](const Rectangle& rectangle)
            {
                auto rectSize = rectangle.mSize / scale;
                mRigidbody = std::make_unique<RectangleRigidbody>(device,
                                                                  size,
                                                                  box2dWorld,
                                                                  b2_staticBody,
                                                                  Vortex2D::Fluid::RigidBody::Type::eStatic,
                                                                  rectSize);
            },
            [&](const Polygon& /*polygon*/)
            {

            });

        mRigidbody->SetTransform(mShape->Position / mScale, mShape->Rotation);
        mRigidbody->mBody->SetUserData(this);
    }

    void SetTransform(const glm::vec2& pos, float angle)
    {
        mShape->Position = pos;
        mShape->Rotation = angle;
        mRigidbody->SetTransform(pos / mScale, angle);
    }
};

using EntityPtr = std::unique_ptr<Entity>;

inline bool IsValid(const ShapeType& type)
{
    return type.match(
        [&](const Circle& circle)
        {
            return circle.mRadius > 0.0f;
        },
        [&](const Rectangle& rectangle)
        {
            return rectangle.mSize.x > 0.0f && rectangle.mSize.y > 0.0f;
        },
        [&](const Polygon& /*polygon*/)
        {
            return false;
        });
}
