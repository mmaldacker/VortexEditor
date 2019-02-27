#include "imguirenderer.h"
#include "vortexeditor_spirv.h"

int GetWidth()
{
    unsigned char* pixels;
    int width, height;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    return width;
}

int GetHeight()
{
    unsigned char* pixels;
    int width, height;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    return height;
}

ImGuiRenderer::ImGuiRenderer(const Vortex2D::Renderer::Device& device)
    : mDevice(device)
      , mFontTexture(device, GetWidth(), GetHeight(), vk::Format::eR8G8B8A8Unorm, VMA_MEMORY_USAGE_GPU_ONLY)
      , mVertexBuffer(device, 1, VMA_MEMORY_USAGE_GPU_ONLY)
      , mLocalVertexBuffer(device, 1, VMA_MEMORY_USAGE_CPU_TO_GPU)
      , mIndexBuffer(device, 1, VMA_MEMORY_USAGE_GPU_ONLY)
      , mLocalIndexBuffer(device, 1, VMA_MEMORY_USAGE_CPU_TO_GPU)
      , mCopy(device)
{
    Vortex2D::SPIRV::Reflection reflectionVert(Vortex2D::SPIRV::imgui_vert);
    Vortex2D::SPIRV::Reflection reflectionFrag(Vortex2D::SPIRV::imgui_frag);

    Vortex2D::Renderer::PipelineLayout layout = {{reflectionVert, reflectionFrag}};
    mDescriptorSet = device.GetLayoutManager().MakeDescriptorSet(layout);

    mSampler = Vortex2D::Renderer::SamplerBuilder().Filter(vk::Filter::eLinear).Create(device.Handle());
    Bind(device, mDescriptorSet, layout, {{*mSampler, mFontTexture, 0}});

    vk::ShaderModule vertexShader = device.GetShaderModule(Vortex2D::SPIRV::imgui_vert);
    vk::ShaderModule fragShader = device.GetShaderModule(Vortex2D::SPIRV::imgui_frag);

    mPipeline = Vortex2D::Renderer::GraphicsPipeline::Builder()
                    .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
                    .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
                    .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, pos))
                    .VertexAttribute(1, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, uv))
                    .VertexAttribute(2, 0, vk::Format::eR8G8B8A8Unorm, offsetof(ImDrawVert, col))
                    .VertexBinding(0, sizeof(ImDrawVert))
                    .DynamicState(vk::DynamicState::eScissor)
                    .DynamicState(vk::DynamicState::eViewport)
                    .Layout(mDescriptorSet.pipelineLayout);

    unsigned char* pixels;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, nullptr, nullptr);
    Vortex2D::Renderer::Texture localTexture(device, mFontTexture.GetWidth(), mFontTexture.GetHeight(), mFontTexture.GetFormat(), VMA_MEMORY_USAGE_CPU_ONLY);

    localTexture.CopyFrom(pixels);
    device.Execute([&](vk::CommandBuffer commandBuffer) {
        mFontTexture.CopyFrom(commandBuffer, localTexture);
    });

    ImGui::GetIO().Fonts->SetTexID((ImTextureID)(VkImage)mFontTexture.Handle());
}

void ImGuiRenderer::Update()
{
    auto* data = ImGui::GetDrawData();
    if (data->TotalIdxCount == 0) return;

    mLocalVertexBuffer.Resize(data->TotalVtxCount * sizeof(ImDrawVert));
    mVertexBuffer.Resize(data->TotalVtxCount * sizeof(ImDrawVert));

    mLocalIndexBuffer.Resize(data->TotalIdxCount * sizeof(ImDrawIdx));
    mIndexBuffer.Resize(data->TotalIdxCount * sizeof(ImDrawIdx));

    uint32_t vtxOffset = 0;
    uint32_t idxOffet = 0;
    for (int n = 0; n < data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = data->CmdLists[n];
        mLocalVertexBuffer.CopyFrom(vtxOffset * sizeof(ImDrawVert), cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        mLocalIndexBuffer.CopyFrom(idxOffet * sizeof(ImDrawIdx), cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtxOffset += cmd_list->VtxBuffer.Size;
        idxOffet += cmd_list->IdxBuffer.Size;
    }

    mCopy
        .Record([&](vk::CommandBuffer commandBuffer)
                {
                    mVertexBuffer.CopyFrom(commandBuffer, mLocalVertexBuffer);
                    mIndexBuffer.CopyFrom(commandBuffer, mLocalIndexBuffer);
                })
        .Submit();
}

void ImGuiRenderer::Initialize(const Vortex2D::Renderer::RenderState& renderState)
{
    mPipeline.Create(mDevice.Handle(), renderState);
}

void ImGuiRenderer::Update(const glm::mat4& projection, const glm::mat4& view)
{

}

void ImGuiRenderer::Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState)
{
    auto* data = ImGui::GetDrawData();
    if (data == nullptr)
    {
        return;
    }

    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
    commandBuffer.bindIndexBuffer(mIndexBuffer.Handle(), 0, vk::IndexType::eUint16);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     mDescriptorSet.pipelineLayout, 0, {*mDescriptorSet.descriptorSet}, {});

    glm::vec2 scale(2.0f / data->DisplaySize.x, 2.0f / data->DisplaySize.y);
    glm::vec2 translate(-1.0f - data->DisplayPos.x * scale[0], -1.0f - data->DisplayPos.y * scale[1]);
    commandBuffer.pushConstants(mDescriptorSet.pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, vk::ArrayProxy<const glm::vec2>{scale, translate});

    commandBuffer.setViewport(0, {vk::Viewport(0, 0, data->DisplaySize.x, data->DisplaySize.y)});

    int vtxOffset = 0;
    int idxOffset = 0;
    for (int n = 0; n < data->CmdListsCount; n++)
    {
        const ImDrawList* cmdList = data->CmdLists[n];
        for (int i = 0; i < cmdList->CmdBuffer.Size; i++)
        {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmdList, pcmd);
            }
            else
            {
                ImVec2 pos = data->DisplayPos;
                vk::Offset2D offset((int)(pcmd->ClipRect.x - pos.x), (int)(pcmd->ClipRect.y - pos.y));
                vk::Extent2D extent((uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x), (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                vk::Rect2D rect(offset, extent);
                commandBuffer.setScissor(0, {rect});
                commandBuffer.drawIndexed(pcmd->ElemCount, 1, idxOffset, vtxOffset, 0);
            }
            idxOffset += pcmd->ElemCount;
        }
        vtxOffset += cmdList->VtxBuffer.Size;
    }
}
