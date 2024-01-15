// Autogenerated code for vkmmc project
// Header file

#pragma once
#include "RendererBase.h"

namespace vkmmc
{
	class UIRenderer : public IRendererBase
	{
	public:
		virtual void Init(const RendererCreateInfo& info) override;
		virtual void Destroy(const RenderContext& renderContext) override;
		virtual void BeginFrame(const RenderContext& renderContext) override;
		virtual void RecordCommandBuffer(const RenderFrameContext& renderFrameContext,
			const Model* models, uint32_t modelCount) override;
	private:
		VkDescriptorPool m_uiPool;
	};
}
