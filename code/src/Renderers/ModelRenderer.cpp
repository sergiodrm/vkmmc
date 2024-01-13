// Autogenerated code for vkmmc project
// Source file
#include "ModelRenderer.h"
#include "Debug.h"
#include "InitVulkanTypes.h"
#include "Shader.h"
#include "Globals.h"
#include "VulkanRenderEngine.h"

namespace vkmmc
{
    void ModelRenderer::Init(const RendererCreateInfo& info)
    {
		/***************************/
		/** Descriptor set layout **/
		/***************************/
        VkDescriptorSetLayoutBinding bindings[] =
        {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 3,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = nullptr
            }
        };
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = sizeof(bindings) / sizeof(VkDescriptorSetLayoutBinding),
            .pBindings = bindings
        };
        vkcheck(vkCreateDescriptorSetLayout(info.RContext.Device, &layoutCreateInfo, nullptr, &m_descriptorSetLayout));
        VkDescriptorSetLayout setLayouts[] = { info.GlobalDescriptorSetLayout, m_descriptorSetLayout };
        const uint32_t setLayoutCount = sizeof(setLayouts) / sizeof(VkDescriptorSetLayout);

		/**********************************/
		/** Pipeline layout and pipeline **/
		/**********************************/
		ShaderModuleLoadDescription shaderStageDescs[] =
		{
			{.ShaderFilePath = globals::BasicVertexShader, .Flags = VK_SHADER_STAGE_VERTEX_BIT},
			{.ShaderFilePath = globals::BasicFragmentShader, .Flags = VK_SHADER_STAGE_FRAGMENT_BIT}
		};

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vkinit::PipelineLayoutCreateInfo();
        pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
        pipelineLayoutCreateInfo.pSetLayouts = setLayouts;
        pipelineLayoutCreateInfo.pushConstantRangeCount = info.ConstantRangeCount;
        pipelineLayoutCreateInfo.pPushConstantRanges = info.ConstantRange;
        m_renderPipeline = RenderPipeline::Create(
            info.RContext,
            info.RenderPass,
            0,
            shaderStageDescs,
            sizeof(shaderStageDescs) / sizeof(ShaderModuleLoadDescription),
            pipelineLayoutCreateInfo,
            VertexInputLayout::GetStaticMeshVertexLayout()
        );
    }

    void ModelRenderer::Destroy(const RenderContext& renderContext)
    {
        m_renderPipeline.Destroy(renderContext);
        vkDestroyDescriptorSetLayout(renderContext.Device, m_descriptorSetLayout, nullptr);
    }

    void ModelRenderer::RecordCommandBuffer(const RenderFrameContext& renderFrameContext, const Model* models, uint32_t modelCount)
    {
        VkCommandBuffer cmd = renderFrameContext.GraphicsCommand;
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderPipeline.GetPipelineHandle());
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderPipeline.GetPipelineLayoutHandle(),
            0, 1, &renderFrameContext.GlobalDescriptorSet, 0, nullptr);

        if (renderFrameContext.PushConstantData)
        {
            check(renderFrameContext.PushConstantSize);
            vkCmdPushConstants(cmd, m_renderPipeline.GetPipelineLayoutHandle(),
                VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                renderFrameContext.PushConstantSize,
                renderFrameContext.PushConstantData);
        }

		Material lastMaterial;
		Mesh lastMesh;
		for (uint32_t i = 0; i < modelCount; ++i)
		{
			const Model& model = models[i];
			for (uint32_t meshIndex = 0; meshIndex < model.m_meshArray.size(); ++meshIndex)
			{
				Mesh mesh = model.m_meshArray[meshIndex];
				Material material = model.m_materialArray[meshIndex];
				if (mesh.GetHandle().IsValid() && material.GetHandle().IsValid())
				{
					if (material != lastMaterial || !lastMaterial.GetHandle().IsValid())
					{
						lastMaterial = material;
						RenderHandle mtlHandle = material.GetHandle();
						if (mtlHandle.IsValid())
						{
                            const void* internalData = material.GetInternalData();
							const MaterialRenderData& texData = *static_cast<const MaterialRenderData*>(internalData);
							vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
								m_renderPipeline.GetPipelineLayoutHandle(), 1, 1, &texData.Set,
								0, nullptr);
						}
					}

					if (lastMesh != mesh)
					{
						check(mesh.GetHandle().IsValid());
                        const void* internalData = mesh.GetInternalData();
						const MeshRenderData& meshRenderData = *static_cast<const MeshRenderData*>(internalData);
						meshRenderData.VertexBuffer.Bind(cmd);
						meshRenderData.IndexBuffer.Bind(cmd);
						lastMesh = mesh;
					}
					vkCmdDrawIndexed(cmd, (uint32_t)lastMesh.GetIndexCount(), 1, 0, 0, i);
					//vkmmc_debug::GRenderStats.m_drawCalls++;
					//vkmmc_debug::GRenderStats.m_trianglesCount += (uint32_t)mesh.GetVertexCount() / 3;
				}
			}
		}

    }

}
