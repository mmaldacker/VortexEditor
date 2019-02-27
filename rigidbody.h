#pragma once
#include <glm/trigonometric.hpp>

#include <Vortex2D/Vortex2D.h>
#include <Box2D/Box2D.h>

class Box2DSolver : public Vortex2D::Fluid::RigidBodySolver
{
public:
    Box2DSolver(b2World& world);
    void Step(float delta) override;

private:
    b2World& mWorld;
};

class Box2DRigidbody : public Vortex2D::Fluid::RigidBody
{
public:
    Box2DRigidbody(const Vortex2D::Renderer::Device& device,
                   const glm::ivec2& size,
                   Vortex2D::Renderer::Drawable& drawable,
                   Vortex2D::Fluid::RigidBody::Type type);

    void ApplyForces();
    void ApplyVelocities();

    void SetTransform(const glm::vec2& pos, float angle);

    b2Body* mBody;
};

class Rigidbody
{
public:
    virtual ~Rigidbody() {}
};

class PolygonRigidbody : public Rigidbody
{
public:
    PolygonRigidbody(const Vortex2D::Renderer::Device& device,
                     const glm::ivec2& size,
                     b2World& rWorld,
                     b2BodyType rType,
                     Vortex2D::Fluid::RigidBody::Type type,
                     const std::vector<glm::vec2>& points,
                     float density = 1.0f);

    Vortex2D::Fluid::Polygon mPolygon;
    Box2DRigidbody mRigidbody;
};

class RectangleRigidbody : public PolygonRigidbody
{
public:
    RectangleRigidbody(const Vortex2D::Renderer::Device& device,
                       const glm::ivec2& size,
                       b2World& rWorld,
                       b2BodyType rType,
                       Vortex2D::Fluid::RigidBody::Type type,
                       const glm::vec2& halfSize,
                       float density = 1.0f)
        : PolygonRigidbody(device, size, rWorld, rType, type,
                           {{-halfSize.x, -halfSize.y},
                           {halfSize.x, -halfSize.y},
                           {halfSize.x, halfSize.y},
                           {-halfSize.x, halfSize.y}},
                           density)
    {
    }
};

class CircleRigidbody : public Rigidbody
{
public:
    CircleRigidbody(const Vortex2D::Renderer::Device& device,
                    const glm::ivec2& size,
                    b2World& rWorld,
                    b2BodyType rType,
                    Vortex2D::Fluid::RigidBody::Type type,
                    const float radius,
                    float density = 1.0f);

    Vortex2D::Fluid::Circle mCircle;
    Box2DRigidbody mRigidbody;
};
