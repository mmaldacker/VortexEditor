#include "rigidbody.h"

namespace
{

float sign(float value)
{
    if (value >= 0.0f) return 1.0f;
    return -1.0;
}

b2FixtureDef GetPolygonFixtureDef(const std::vector<glm::vec2>& points)
{
    static b2PolygonShape shape;

    std::vector<b2Vec2> b2Points;
    for (auto& point: points)
    {
        b2Points.push_back({point.x - sign(point.x) * b2_polygonRadius,
                            point.y - sign(point.y) * b2_polygonRadius});
    }

    shape.Set(b2Points.data(), static_cast<int>(b2Points.size()));

    static b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;

    return fixtureDef;
}

b2FixtureDef GetCircleFixtureDef(float radius)
{
    static b2CircleShape shape;
    shape.m_radius = radius;

    static b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;

    return fixtureDef;
}

b2Body* CreateBody(b2World& world, b2FixtureDef fixtureDef, b2BodyType type, float density)
{
    b2BodyDef def;
    def.type = type;

    auto* body = world.CreateBody(&def);

    fixtureDef.density = density;
    body->CreateFixture(&fixtureDef);

    return body;
}

}

Box2DSolver::Box2DSolver(b2World& world)
    : mWorld(world)
{

}

void Box2DSolver::Step(float delta)
{
    const int velocityStep = 8;
    const int positionStep = 3;
    mWorld.Step(delta, velocityStep, positionStep);
}

Box2DRigidbody::Box2DRigidbody(const Vortex2D::Renderer::Device& device,
                               const glm::ivec2& size,
                               Vortex2D::Renderer::Drawable& drawable,
                               Vortex2D::Fluid::RigidBody::Type type,
                               b2Body* body)
    : Vortex2D::Fluid::RigidBody(device, size, drawable, type)
    , mBody(body)
{
}

void Box2DRigidbody::ApplyForces()
{
    if (GetType() & Vortex2D::Fluid::RigidBody::Type::eWeak)
    {
        auto force = GetForces();
        b2Vec2 b2Force = {force.velocity.x, force.velocity.y};

        mBody->ApplyForceToCenter(b2Force, true);
        mBody->ApplyTorque(force.angular_velocity, true);
    }
}

void Box2DRigidbody::ApplyVelocities()
{
    auto pos = mBody->GetPosition();
    Position = {pos.x, pos.y};
    Rotation = glm::degrees(mBody->GetAngle());

    if (GetType() & Vortex2D::Fluid::RigidBody::Type::eStatic)
    {
        glm::vec2 vel = {mBody->GetLinearVelocity().x, mBody->GetLinearVelocity().y};
        float angularVelocity = mBody->GetAngularVelocity();
        SetVelocities(vel, angularVelocity);
    }
}

void Rigidbody::SetTransform(const glm::vec2& pos, float angle)
{
    mRigidbody->Position = pos;
    mRigidbody->Rotation = angle;
    mBody->SetTransform({pos.x, pos.y}, glm::radians(angle));
}

PolygonRigidbody::PolygonRigidbody(const Vortex2D::Renderer::Device& device,
                                   const glm::ivec2& size,
                                   b2World& rWorld,
                                   b2BodyType rType,
                                   Vortex2D::Fluid::RigidBody::Type type,
                                   const std::vector<glm::vec2>& points,
                                   float density)
    : mPolygon(device, points)
{
    mBody = CreateBody(rWorld, GetPolygonFixtureDef(points), rType, density);
    mRigidbody = std::make_unique<Box2DRigidbody>(device, size, mPolygon, type, mBody);
    mRigidbody->SetMassData(mBody->GetMass(), mBody->GetInertia());
}

CircleRigidbody::CircleRigidbody(const Vortex2D::Renderer::Device& device,
                                 const glm::ivec2& size,
                                 b2World& rWorld,
                                 b2BodyType rType,
                                 Vortex2D::Fluid::RigidBody::Type type,
                                 const float radius,
                                 float density)
    : mCircle(device, radius)
{
    mBody = CreateBody(rWorld, GetCircleFixtureDef(radius), rType, density);
    mRigidbody = std::make_unique<Box2DRigidbody>(device, size, mCircle, type, mBody);
    mRigidbody->SetMassData(mBody->GetMass(), mBody->GetInertia());
}

