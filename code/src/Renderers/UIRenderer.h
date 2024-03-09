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
		virtual void PrepareFrame(const RenderContext& renderContext, RenderFrameContext& renderFrameContext) override;
		virtual void RecordCmd(const RenderContext& renderContext, const RenderFrameContext& renderFrameContext, uint32_t attachmentIndex) override;
		virtual void ImGuiDraw() override {}
	private:
		VkDescriptorPool m_uiPool;
	};
}
