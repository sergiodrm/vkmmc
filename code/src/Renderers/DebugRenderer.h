// Autogenerated code for vkmmc project
// Header file

#pragma once
#include "RendererBase.h"

namespace vkmmc
{
	class DebugRenderer : public IRendererBase
	{
	public:
		virtual void Init(const RendererCreateInfo& info) override;
		virtual void Destroy(const RenderContext& renderContext) override;
		virtual void RecordCommandBuffer(const RenderFrameContext& renderFrameContext,
			const Model* models, uint32_t modelCount) override;
	protected:
		// Render State
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorSet m_uniformSet;
		AllocatedBuffer m_uniformBuffer;
		RenderPipeline m_renderPipeline;
		VertexBuffer m_lineVertexBuffer;
	};
}
