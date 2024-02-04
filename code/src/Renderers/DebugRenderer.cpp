// Autogenerated code for vkmmc project
// Source file
#include "DebugRenderer.h"
#include "Debug.h"
#include "InitVulkanTypes.h"
#include "Shader.h"
#include "Globals.h"
#include "VulkanRenderEngine.h"
#include "RendererBase.h"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/transform.hpp"
#include <corecrt_math_defines.h>


namespace vkmmc
{
    namespace rdbg
    {
        struct LineVertex
        {
            glm::vec4 Position;
            glm::vec4 Color;
        };

        struct LineBatch
        {
            static constexpr uint32_t MaxLines = 500;
            LineVertex LineArray[MaxLines];
            uint32_t Index = 0;
        } GLineBatch;

        void PushLineVertex(LineBatch& batch, const glm::vec3& pos, const glm::vec3& color)
        {
			if (batch.Index < LineBatch::MaxLines)
			{
                batch.LineArray[batch.Index++] = { glm::vec4(pos, 1.f), glm::vec4(color, 1.f) };
			}
			else
				Logf(LogLevel::Error, "DebugRender line overflow. Increase MaxLines (Current: %u)\n", LineBatch::MaxLines);
        }

        void PushLine(LineBatch& batch, const glm::vec3& init, const glm::vec3& end, const glm::vec3& color)
        {
            PushLineVertex(batch, init, color);
            PushLineVertex(batch, end, color);
        }

        void DeferredDrawLine(const glm::vec3& init, const glm::vec3& end, const glm::vec3& color)
        {
            PushLine(GLineBatch, init, end, color);
        }

        void DeferredDrawAxis(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl)
        {
			glm::mat4 tras = glm::translate(glm::mat4{ 1.f }, pos);
            glm::mat4 rotMat = glm::toMat4(glm::quat(rot));
            glm::mat4 sclMat = glm::scale(scl);
            DeferredDrawAxis(tras * rotMat * sclMat);
        }

        void DeferredDrawAxis(const glm::mat4& transform)
        {
            DeferredDrawLine(transform[3], transform[3] + transform[0], glm::vec3(1.f, 0.f, 0.f));
            DeferredDrawLine(transform[3], transform[3] + transform[1], glm::vec3(0.f, 1.f, 0.f));
            DeferredDrawLine(transform[3], transform[3] + transform[2], glm::vec3(0.f, 0.f, 1.f));
        }

        void DeferredDrawSphere(const glm::vec3& pos, float radius, const glm::vec3& color, uint32_t vertices)
        {
            const float deltaAngle = 2.f * (float)M_PI / (float)vertices;
            for (uint32_t i = 0; i < vertices; ++i)
            {
                float c0 = radius * cosf(deltaAngle * i);
                float c1 = radius * cosf(deltaAngle * (i+1));
                float s0 = radius * sinf(deltaAngle * i);
                float s1 = radius * sinf(deltaAngle * (i+1));
                glm::vec3 pos0 = pos + glm::vec3{ c0, 0.f, s0 };
                glm::vec3 pos1 = pos + glm::vec3{ c1, 0.f, s1 };
                DeferredDrawLine(pos0, pos1, color);
                pos0 = pos + glm::vec3{ c0, s0, 0.f };
                pos1 = pos + glm::vec3{ c1, s1, 0.f };
                DeferredDrawLine(pos0, pos1, color);
                pos0 = pos + glm::vec3{ 0.f, c0, s0 };
                pos1 = pos + glm::vec3{ 0.f, c1, s1 };
                DeferredDrawLine(pos0, pos1, color);
            }
        }
    }

    void DebugRenderer::Init(const RendererCreateInfo& info)
    {
		/**********************************/
		/** Pipeline layout and pipeline **/
		/**********************************/
        ShaderDescription descriptions[] =
        {
            {.Filepath = globals::LineVertexShader, .Stage = VK_SHADER_STAGE_VERTEX_BIT},
            {.Filepath = globals::LineFragmentShader, .Stage = VK_SHADER_STAGE_FRAGMENT_BIT}
        };
        uint32_t descriptionCount = sizeof(descriptions) / sizeof(ShaderDescription);

        VertexInputLayout inputLayout = VertexInputLayout::BuildVertexInputLayout({ EAttributeType::Float4, EAttributeType::Float4 });
        m_renderPipeline = RenderPipeline::Create(info.RContext, info.RenderPass, 0,
            descriptions, descriptionCount, inputLayout, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

        // VertexBuffer
        BufferCreateInfo vbInfo;
        vbInfo.Size = sizeof(rdbg::LineVertex) * rdbg::LineBatch::MaxLines;
        vbInfo.Data = nullptr;
        m_lineVertexBuffer.Init(info.RContext, vbInfo);
    }

    void DebugRenderer::Destroy(const RenderContext& renderContext)
    {
        m_lineVertexBuffer.Destroy(renderContext);
        m_renderPipeline.Destroy(renderContext);
    }

    void DebugRenderer::RecordCommandBuffer(const RenderContext& renderContext, RenderFrameContext& renderFrameContext)
    {
        PROFILE_SCOPE(DebugPass);
        if (rdbg::GLineBatch.Index > 0)
        {
            // Flush lines to vertex buffer
            GPUBuffer::SubmitBufferToGpu(m_lineVertexBuffer, &rdbg::GLineBatch.LineArray, sizeof(rdbg::LineVertex) * rdbg::GLineBatch.Index);

            vkCmdBindPipeline(renderFrameContext.GraphicsCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderPipeline.GetPipelineHandle());
            VkDescriptorSet sets[] = { renderFrameContext.CameraDescriptorSet };
            uint32_t setCount = sizeof(sets) / sizeof(VkDescriptorSet);
            vkCmdBindDescriptorSets(renderFrameContext.GraphicsCommand, VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_renderPipeline.GetPipelineLayoutHandle(), 0, setCount, sets, 0, nullptr);
            ++GRenderStats.SetBindingCount;
            m_lineVertexBuffer.Bind(renderFrameContext.GraphicsCommand);
            vkCmdDraw(renderFrameContext.GraphicsCommand, rdbg::GLineBatch.Index, 1, 0, 0);
            rdbg::GLineBatch.Index = 0;
            ++GRenderStats.DrawCalls;
        }
    }

}
