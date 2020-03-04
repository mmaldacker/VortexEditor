#include "world.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <GLFW/glfw3.h>

namespace
{
const float gravityForce = 100.0f;

struct QueryCallback : b2QueryCallback
{
  bool ReportFixture(b2Fixture* fixture) override
  {
    mContactFixture = fixture;
    return false;
  }

  b2Fixture* mContactFixture = nullptr;
};
}  // namespace

World::World(const Vortex2D::Renderer::Device& device, const glm::ivec2& size, float scale)
    : mDevice(device)
    , mSize(size)
    , mScale(scale)
    , mWorld(device,
             size,
             ImGui::GetIO().DeltaTime,
             2,
             Vortex2D::Fluid::Velocity::InterpolationMode::Linear)
    , mBox2DWorld(b2Vec2(0.0f, gravityForce))
    , mBox2DSolver(mBox2DWorld)
    , mGravity(device, size)
    , mLiquidPhi(mWorld.LiquidDistanceField())
{
  mWorld.AttachRigidBodySolver(mBox2DSolver);
  mGravity.Colour = {0.0f, ImGui::GetIO().DeltaTime * gravityForce, 0.0f, 0.0f};
  mLiquidPhi.Colour = glm::vec4(36.0f, 123.0f, 160.0f, 255.0f) / glm::vec4(255.0f);
}

b2World& World::GetBox2dWorld()
{
  return mBox2DWorld;
}

Vortex2D::Fluid::WaterWorld& World::GetWorld()
{
  return mWorld;
}

void World::Record(Vortex2D::Renderer::RenderTarget& target,
                   Vortex2D::Renderer::ColorBlendState blendState)
{
  mWindowRender = target.Record({mLiquidPhi}, blendState);
}

void World::RecordVelocity(Vortex2D::Renderer::Drawable& velocity)
{
  mVelocityRender = mWorld.RecordVelocity({mGravity, velocity}, Vortex2D::Fluid::VelocityOp::Add);
}

void World::Render(Vortex2D::Renderer::RenderTarget& target)
{
  mWorld.SubmitVelocity(mVelocityRender);
  auto params = Vortex2D::Fluid::FixedParams(12);
  mWorld.Step(params);

  mWindowRender.Submit(glm::scale(glm::vec3(mScale, mScale, 1.0f)));

  for (int i = 0; i < mEntities.size(); i++)
  {
    auto& entity = *mEntities[i];
    if (entity.mRigidbody->mBody->GetType() == b2_dynamicBody)
    {
      entity.UpdateTransform();
    }
  }

  for (auto& entity : mEntities)
  {
    entity->mCmd.Submit();
  }
}

Entity* World::FindEntity(const glm::vec2& pos)
{
  b2Fixture* currentFixture = nullptr;

  QueryCallback callback;
  b2Vec2 scaledPos = {pos.x / mScale, pos.y / mScale};
  mBox2DWorld.QueryAABB(&callback, {scaledPos, scaledPos});
  if (callback.mContactFixture != nullptr && callback.mContactFixture->TestPoint(scaledPos))
  {
    currentFixture = callback.mContactFixture;
  }

  if (currentFixture != nullptr)
  {
    return static_cast<Entity*>(currentFixture->GetBody()->GetUserData());
  }

  return nullptr;
}

void World::Clear()
{
  while (!mEntities.empty())
  {
    auto& entity = *mEntities.back();
    entity.mCmd.Wait();
    mWorld.RemoveRigidBody(*entity.mRigidbody->mRigidbody);
    mBox2DWorld.DestroyBody(entity.mRigidbody->mBody);
    mEntities.pop_back();
  }
}

void World::Add(EntityPtr entity)
{
  mWorld.AddRigidbody(*entity->mRigidbody->mRigidbody);
  mEntities.push_back(std::move(entity));
}

void World::Delete(Entity* entity)
{
  auto it = std::find_if(mEntities.begin(), mEntities.end(), [&](const auto& entityPtr) {
    return entityPtr.get() == entity;
  });

  if (it == mEntities.end())
  {
    return;
  }

  auto& entityRef = *(*it);

  entityRef.mCmd.Wait();
  mWorld.RemoveRigidBody(*entityRef.mRigidbody->mRigidbody);
  mBox2DWorld.DestroyBody(entityRef.mRigidbody->mBody);
  mEntities.erase(it);
}
