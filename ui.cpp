#include "ui.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <Box2D/Box2D.h>
#include <GLFW/glfw3.h>

namespace
{
const glm::vec2 radius(16.0f);
const glm::vec2 anchor = radius / glm::vec2(2.0f);
static glm::vec4 green = glm::vec4(92.0f, 173.0f, 159.0f, 255.0f) / glm::vec4(255.0f);
static glm::vec4 red = glm::vec4(208.0f, 43.0f, 10.0f, 255.0f) / glm::vec4(255.0f);
}  // namespace

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

UI::UI(const Vortex2D::Renderer::Device& device, World& world, const glm::ivec2& size, float scale)
    : mDevice(device)
    , mSize(size)
    , mScale(scale)
    , mWorld(world)
    , mCurrentEntity(nullptr)
    , mShapeType(Rectangle{})
    , mForce(device, radius)
{
  mForce.Colour = glm::vec4(0.0f);
  mWorld.RecordVelocity(mForce);
}

void UI::RenderFluid()
{
  auto& io = ImGui::GetIO();

  glm::vec2 pos = {io.MouseClickedPos[0].x / mScale, io.MouseClickedPos[0].y / mScale};
  if (io.KeysDown[GLFW_KEY_LEFT_CONTROL])
  {
    auto size = glm::abs(glm::vec2{(io.MouseClickedPos[0].x - io.MousePos.x) / mScale,
                                   (io.MouseClickedPos[0].y - io.MousePos.y) / mScale});

    mFluid = std::make_shared<Vortex2D::Renderer::IntRectangle>(mDevice, size);
    mForce.Colour = glm::vec4(0.0f);
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
  mWorld.GetWorld().RecordParticleCount({*mFluid}).Submit();
}

void UI::MoveShape(Entity& entity)
{
  auto& io = ImGui::GetIO();

  glm::vec2 centre = entity.mShape->Position;

  if (io.KeysDown[GLFW_KEY_LEFT_CONTROL])
  {
    // rotation
    glm::vec2 v1 = {io.MouseClickedPos[0].x - centre.x, io.MouseClickedPos[0].y - centre.y};
    glm::vec2 v2 = {io.MousePos.x - centre.x, io.MousePos.y - centre.y};
    auto angle = glm::orientedAngle(glm::normalize(v1), glm::normalize(v2));
    entity.SetTransform(centre, glm::degrees(angle));
  }
  else
  {
    // translation
    glm::vec2 delta = {io.MouseDelta.x, io.MouseDelta.y};
    entity.SetTransform(centre + delta, entity.mShape->Rotation);
  }
}

void UI::ResizeShape(Vortex2D::Renderer::RenderTarget& target)
{
  auto& io = ImGui::GetIO();

  auto size = glm::abs(
      glm::vec2{io.MouseClickedPos[0].x - io.MousePos.x, io.MouseClickedPos[0].y - io.MousePos.y});

  mShapeType.match(
      [&](Rectangle& rectangle) {
        auto halfSize = size / glm::vec2(2.0f);
        mBuildShape = std::make_unique<Vortex2D::Renderer::Rectangle>(mDevice, size);
        mBuildShape->Position =
            glm::vec2(io.MouseClickedPos[0].x, io.MouseClickedPos[0].y) + halfSize;
        mBuildShape->Anchor = halfSize;
        rectangle.mSize = size;
      },
      [&](Circle& circle) {
        auto radius = glm::length(size);
        mBuildShape =
            std::make_unique<Vortex2D::Renderer::Ellipse>(mDevice, glm::vec2(radius, radius));
        mBuildShape->Position = {io.MouseClickedPos[0].x, io.MouseClickedPos[0].y};
        circle.mRadius = radius;
      },
      [&](Polygon& /*polygon*/) {

      });

  mBuildShape->Colour = green;

  Vortex2D::Renderer::ColorBlendState blendState;
  blendState.ColorBlend.setBlendEnable(true)
      .setAlphaBlendOp(vk::BlendOp::eAdd)
      .setColorBlendOp(vk::BlendOp::eAdd)
      .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
      .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
      .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
      .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

  mBuildCmd = target.Record({*mBuildShape}, blendState);
}

void UI::CreateShape()
{
  if (mBuildShape && IsValid(mShapeType))
  {
    EntityPtr entity = std::make_unique<Entity>(mDevice,
                                                mSize,
                                                mScale,
                                                mShapeType,
                                                std::move(mBuildShape),
                                                std::move(mBuildCmd),
                                                mWorld.GetBox2dWorld());

    mWorld.Add(std::move(entity));
  }
}

void UI::Update(Vortex2D::Renderer::RenderTarget& target)
{
  static bool renderFluid = false;

  auto& io = ImGui::GetIO();

  if (ImGui::BeginPopupContextVoid("World Manager"))
  {
    if (mCurrentEntity == nullptr)
    {
      if (ImGui::Selectable("Rectangle", !renderFluid && mShapeType.is<Rectangle>()))
      {
        mShapeType = Rectangle{};
        renderFluid = false;
      }
      if (ImGui::Selectable("Circle", !renderFluid && mShapeType.is<Circle>()))
      {
        mShapeType = Circle{};
        renderFluid = false;
      }
      if (ImGui::Selectable("Fluid", renderFluid))
      {
        renderFluid = true;
      }
    }
    else
    {
      if (ImGui::Selectable("Static"))
      {
        mCurrentEntity->mRigidbody->mBody->SetType(GetBox2DType(0));
        mCurrentEntity->mRigidbody->mRigidbody->SetType(GetVortex2DType(0));
        mCurrentEntity = nullptr;
      }
      if (ImGui::Selectable("Dynamic"))
      {
        mCurrentEntity->mRigidbody->mBody->SetType(GetBox2DType(1));
        mCurrentEntity->mRigidbody->mRigidbody->SetType(GetVortex2DType(1));
        mCurrentEntity->mShape->Colour = red;
        mCurrentEntity = nullptr;
      }
      if (ImGui::Selectable("Delete"))
      {
        mWorld.Delete(mCurrentEntity);
        mCurrentEntity = nullptr;
      }
    }

    if (ImGui::Selectable("Clear"))
    {
      mWorld.Clear();
    }

    ImGui::EndPopup();
  }

  if (io.WantCaptureMouse)
  {
    return;
  }

  if (io.MouseClicked[0] || io.MouseClicked[1])
  {
    mCurrentEntity = mWorld.FindEntity({io.MousePos.x, io.MousePos.y});
  }

  if (io.MouseDown[0])
  {
    if (renderFluid)
    {
      RenderFluid();
    }
    else if (mCurrentEntity != nullptr)
    {
      MoveShape(*mCurrentEntity);
    }
    else
    {
      ResizeShape(target);
    }
  }

  if (io.MouseReleased[0])
  {
    if (mCurrentEntity == nullptr)
    {
      CreateShape();
    }

    mCurrentEntity = nullptr;
  }

  mBuildCmd.Submit();
}
