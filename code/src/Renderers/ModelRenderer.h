// Autogenerated code for vkmmc project
// Header file

#pragma once
#include "RendererBase.h"
#include <glm/glm.hpp>

namespace vkmmc
{
	struct LightData
	{
		glm::vec3 Position;
		float Radius;
		glm::vec3 Color;
		float Compression;
	};

	struct EnvironmentData
	{
		glm::vec4 AmbientColor;
		glm::vec3 ViewPosition;
		float ActiveLightsCount;
		static constexpr uint32_t MaxLights = 8;
		LightData Lights[MaxLights];
	};

	class ModelRenderer : public IRendererBase
	{
		struct RendererFrameData
		{
			// Camera, models and environment
			VkDescriptorSet GlobalSet;
		};
	public:
		ModelRenderer();
		virtual void Init(const RendererCreateInfo& info) override;
		virtual void Destroy(const RenderContext& renderContext) override;
		virtual void BeginFrame(const RenderContext& renderContext) override {}
		virtual void RecordCommandBuffer(const RenderContext& renderContext, RenderFrameContext& renderFrameContext) override;
		virtual void ImGuiDraw() override;

	protected:
		// Render State
		RenderPipeline m_renderPipeline;

		std::vector<RendererFrameData> m_frameData;
		int32_t m_activeLightsCount;
		EnvironmentData m_environmentData;
	};
}
