#pragma once

#include <Vortex2D/Vortex2D.h>

class ImGuiRenderer : public Vortex2D::Renderer::Drawable
{
public:
    ImGuiRenderer(const Vortex2D::Renderer::Device& device);

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState) override;

    struct Vertex
    {
        alignas(8) glm::vec2 pos;
        alignas(8) glm::vec2 uv;
        alignas(8) glm::u8vec4 color;
    };

    const Vortex2D::Renderer::Device& mDevice;
    Vortex2D::Renderer::DescriptorSet mDescriptorSet;
    Vortex2D::Renderer::GraphicsPipeline mPipeline;
    Vortex2D::Renderer::Texture mFontTexture;
    vk::UniqueSampler mSampler;
    Vortex2D::Renderer::VertexBuffer<Vertex> mVertexBuffer, mLocalVertexBuffer;
    Vortex2D::Renderer::IndexBuffer mIndexBuffer, mLocalIndexBuffer;
    Vortex2D::Renderer::CommandBuffer mCopy;
};
