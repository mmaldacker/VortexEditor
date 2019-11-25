#include "world.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <GLFW/glfw3.h>

namespace
{
b2BodyType GetBox2DType(int type)
{
  switch (type)
  {
    case 0:
      return b2_staticBody;
    case 1:
    case 2:
      return b2_dynamicBody;
    default:
      return b2_staticBody;
  }
}

Vortex2D::Fluid::RigidBody::Type GetVortex2DType(int type)
{
  switch (type)
  {
    case 0:
      return Vortex2D::Fluid::RigidBody::Type::eStatic;
    case 1:
      return Vortex2D::Fluid::RigidBody::Type::eWeak;
    case 2:
      return Vortex2D::Fluid::RigidBody::Type::eStrong;
    default:
      return Vortex2D::Fluid::RigidBody::Type::eStatic;
  }
}

const float gravityForce = 100.0f;
const glm::vec2 radius(16.0f);
const glm::vec2 anchor = radius / glm::vec2(2.0f);
}  // namespace

World::World(const Vortex2D::Renderer::Device& device,
             const glm::ivec2& size,
             float scale,
             std::vector<EntityPtr>& entities)
    : mDevice(device)
    , mSize(size)
    , mScale(scale)
    , mWorld(device, size, ImGui::GetIO().DeltaTime, 2, Vortex2D::Fluid::Velocity::InterpolationMode::Linear)
    , mBox2DWorld(b2Vec2(0.0f, gravityForce))
    , mBox2DSolver(mBox2DWorld)
    , mEntities(entities)
    , mGravity(device, size)
    , mLiquidPhi(mWorld.LiquidDistanceField())
    , mForce(device, radius)
{
  mWorld.AttachRigidBodySolver(mBox2DSolver);
  mGravity.Colour = {0.0f, ImGui::GetIO().DeltaTime * gravityForce, 0.0f, 0.0f};
  mLiquidPhi.Colour = glm::vec4(36.0f, 123.0f, 160.0f, 255.0f) / glm::vec4(255.0f);
  mForce.Colour = glm::vec4(0.0f);
  mVelocityRender = mWorld.RecordVelocity({mGravity, mForce}, Vortex2D::Fluid::VelocityOp::Add);
}

b2World& World::GetBox2dWorld()
{
  return mBox2DWorld;
}

Vortex2D::Fluid::World& World::GetWorld()
{
  return mWorld;
}

void World::Record(Vortex2D::Renderer::RenderTarget& target,
                   Vortex2D::Renderer::ColorBlendState blendState)
{
  mWindowRender = target.Record({mLiquidPhi}, blendState);
}

void World::Render()
{
  static int currentShapeIndex = -1;
  static int type = 0;
  static bool run = true;

  if (ImGui::Begin("World Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
  {
    if (ImGui::BeginCombo(
            "Entities", currentShapeIndex >= 0 ? mEntities[currentShapeIndex]->mId.c_str() : "N/A"))
    {
      if (ImGui::Selectable("N/A", currentShapeIndex == -1))
      {
        currentShapeIndex = -1;
      }
      for (int i = 0; i < static_cast<int>(mEntities.size()); i++)
      {
        const bool itemSelected = i == currentShapeIndex;
        if (ImGui::Selectable(mEntities[i]->mId.c_str(), itemSelected))
        {
          currentShapeIndex = i;
        }
      }
      ImGui::EndCombo();
    }

    ImGui::PushID("type");
    ImGui::Text("Rigidbody Type");
    ImGui::RadioButton("Static", &type, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Dynamic Weak", &type, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Dynamic Strong", &type, 2);
    ImGui::PopID();
    if (ImGui::Button("Update"))
    {
      if (currentShapeIndex >= 0)
      {
        auto& entity = *mEntities[currentShapeIndex];
        entity.mRigidbody->mBody->SetType(GetBox2DType(type));
        entity.mRigidbody->mRigidbody->SetType(GetVortex2DType(type));
        entity.mShape->Colour = glm::vec4(208.0f, 43.0f, 10.0f, 255.0f) / glm::vec4(255.0f);
        currentShapeIndex = -1;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete"))
    {
      if (currentShapeIndex >= 0)
      {
        auto& entity = *mEntities[currentShapeIndex];
        entity.mCmd.Wait();
        mWorld.RemoveRigidBody(*entity.mRigidbody->mRigidbody);
        mBox2DWorld.DestroyBody(entity.mRigidbody->mBody);
        mEntities.erase(mEntities.begin() + currentShapeIndex);
        currentShapeIndex = -1;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clean"))
    {
      while (!mEntities.empty())
      {
        auto& entity = *mEntities.back();
        entity.mCmd.Wait();
        mWorld.RemoveRigidBody(*entity.mRigidbody->mRigidbody);
        mBox2DWorld.DestroyBody(entity.mRigidbody->mBody);
        mEntities.pop_back();
      }

      currentShapeIndex = -1;
    }
    ImGui::SameLine();
    ImGui::Checkbox("Run", &run);
    ImGui::End();
  }

  auto& io = ImGui::GetIO();
  if (!io.WantCaptureMouse && io.MouseDown[1])
  {
    glm::vec2 pos = {io.MouseClickedPos[1].x / mScale, io.MouseClickedPos[1].y / mScale};

    if (io.KeysDown[GLFW_KEY_LEFT_CONTROL])
    {
      auto size = glm::abs(glm::vec2{(io.MouseClickedPos[1].x - io.MousePos.x) / mScale,
                                     (io.MouseClickedPos[1].y - io.MousePos.y) / mScale});

      mFluid = std::make_shared<Vortex2D::Renderer::IntRectangle>(mDevice, size);
    }
    else
    {
      mFluid = std::make_shared<Vortex2D::Renderer::IntRectangle>(mDevice, radius);
      mFluid->Anchor = anchor;

      glm::vec2 currPos = {io.MousePos.x / mScale, io.MousePos.y / mScale};
      glm::vec2 delta = currPos - pos;

      mForce.Position = pos;
      mForce.Anchor = anchor;
      mForce.Colour = glm::vec4(delta, 0.0f, 0.0f);
    }

    mFluid->Colour = glm::vec4(4);
    mFluid->Position = pos;
    mWorld.RecordParticleCount({*mFluid}).Submit();
  }

  if (!io.WantCaptureMouse && io.MouseReleased[1])
  {
    mForce.Colour = glm::vec4(0.0f);
  }

  if (run)
  {
    mWorld.SubmitVelocity(mVelocityRender);
    auto params = Vortex2D::Fluid::FixedParams(12);
    mWorld.Step(params);
  }
  else
  {
    mWorld.ParticlePhi();
  }

  for (int i = 0; i < mEntities.size(); i++)
  {
    auto& entity = *mEntities[i];
    if (i == currentShapeIndex)
    {
      entity.mShape->Colour.a = 0.2f;
    }
    else
    {
      entity.mShape->Colour.a = 1.0f;
    }

    if (entity.mRigidbody->mBody->GetType() == b2_dynamicBody)
    {
      entity.UpdateTransform();
    }
  }

  mWindowRender.Submit(glm::scale(glm::vec3(mScale, mScale, 1.0f)));
}
