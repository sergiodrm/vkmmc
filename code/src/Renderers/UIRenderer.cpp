// Autogenerated code for vkmmc project
// Source file
#include "UIRenderer.h"
#include "Debug.h"
#include "InitVulkanTypes.h"
#include "Shader.h"
#include "Globals.h"
#include "VulkanRenderEngine.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

namespace vkmmc
{
    void UIRenderer::Init(const RendererCreateInfo& info)
    {
		// Create descriptor pool for imgui
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;
		poolInfo.maxSets = 1000;
		poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize);
		poolInfo.pPoolSizes = poolSizes;

		vkcheck(vkCreateDescriptorPool(info.RContext.Device, &poolInfo, nullptr, &m_uiPool));

		// Init imgui lib
		ImGui::CreateContext();
		ImGui_ImplSDL2_InitForVulkan(info.RContext.Window->WindowInstance);
		ImGui_ImplVulkan_InitInfo initInfo
		{
			.Instance = info.RContext.Instance,
			.PhysicalDevice = info.RContext.GPUDevice,
			.Device = info.RContext.Device,
			.Queue = info.RContext.GraphicsQueue,
			.DescriptorPool = m_uiPool,
			.Subpass = 0,
			.MinImageCount = 3,
			.ImageCount = 3,
			.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		};
		ImGui_ImplVulkan_Init(&initInfo, info.RenderPassArray[RENDER_PASS_COLOR]);

		// Execute gpu command to upload imgui font textures
		utils::CmdSubmitTransfer(info.RContext, 
			[&](VkCommandBuffer cmd)
			{
				ImGui_ImplVulkan_CreateFontsTexture(cmd);
			});
		ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    void UIRenderer::Destroy(const RenderContext& renderContext)
    {
		vkDestroyDescriptorPool(renderContext.Device, m_uiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
    }

	void UIRenderer::PrepareFrame(const RenderContext& renderContext, RenderFrameContext& renderFrameContext)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame(renderContext.Window->WindowInstance);
		ImGui::NewFrame();
	}

    void UIRenderer::RecordCmd(const RenderContext& renderContext, const RenderFrameContext& renderFrameContext, uint32_t attachmentIndex)
    {
		CPU_PROFILE_SCOPE(ImGuiPass);
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderFrameContext.GraphicsCommand);
    }

}
