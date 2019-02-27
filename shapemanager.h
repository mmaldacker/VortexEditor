#pragma once

#include <Vortex2D/Vortex2D.h>
#include <imgui.h>

struct Shape
{
    std::shared_ptr<Vortex2D::Renderer::Shape> mShape;
    Vortex2D::Renderer::RenderCommand mCmd;
    std::string mName;
    glm::vec2 mSize;

    enum class Type
    {
        Circle,
        Rectangle,
    };

    Type mType;
};

class ShapeManager
{
public:
    ShapeManager(const Vortex2D::Renderer::Device& device, std::vector<Shape>& shapes);

    void Render(Vortex2D::Renderer::RenderTarget& target);

private:
    const Vortex2D::Renderer::Device& mDevice;
    std::vector<Shape>& mShapes;

    std::shared_ptr<Vortex2D::Renderer::Shape> mBuildShape;
    Vortex2D::Renderer::RenderCommand mBuildCmd;
};
