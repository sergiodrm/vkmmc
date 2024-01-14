
#pragma once

#include "RenderEngine.h"
#include "RenderPass.h"
#include "RenderHandle.h"
#include "RenderPipeline.h"
#include "RenderObject.h"
#include "RenderTexture.h"
#include "VulkanBuffer.h"
#include "Debug.h"
#include "Swapchain.h"
#include "Mesh.h"
#include "FunctionStack.h"
#include "Framebuffer.h"
#include "Scene.h"
#include <cstdio>

#include <vk_mem_alloc.h>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <string.h>

namespace vkmmc
{
	class Framebuffer;
	class IRendererBase;
	struct ShaderModuleLoadDescription;

	struct Window
	{
		SDL_Window* WindowInstance;
		uint32_t Width;
		uint32_t Height;
		char Title[32];

		Window() : WindowInstance(nullptr), Width(1920), Height(1080)
		{
			*Title = 0;
		}

		static Window Create(uint32_t width, uint32_t height, const char* title);
	};

	struct UploadContext
	{
		VkFence Fence;
		VkCommandPool CommandPool;
		VkCommandBuffer CommandBuffer;
	};

	struct GPUCamera
	{
		glm::mat4 View;
		glm::mat4 Projection;
		glm::mat4 ViewProjection;
	};

	struct MaterialRenderData
	{
		enum ESamplerIndex
		{
			SAMPLER_INDEX_DIFFUSE,
			SAMPLER_INDEX_NORMAL,
			SAMPLER_INDEX_SPECULAR,

			SAMPLER_INDEX_COUNT
		};

		VkDescriptorSet Set;
		VkSampler ImageSamplers[SAMPLER_INDEX_COUNT];

		MaterialRenderData() : Set(VK_NULL_HANDLE) { for (uint32_t i = 0; i < SAMPLER_INDEX_COUNT; ++i) ImageSamplers[i] = VK_NULL_HANDLE; }
	};

	struct MeshRenderData
	{
		VertexBuffer VertexBuffer;
		IndexBuffer IndexBuffer;
	};

	class VulkanRenderEngine : public IRenderEngine
	{
		static constexpr size_t MaxOverlappedFrames = 2;
		static constexpr size_t MaxRenderObjects = 1000;
	public:
		/**
		 * IRenderEngine interface
		 */
		virtual bool Init(const InitializationSpecs& initSpec) override;
		virtual void RenderLoop() override;
		virtual bool RenderProcess() override;
		virtual void Shutdown() override;

		virtual void UpdateSceneView(const glm::mat4& view, const glm::mat4& projection) override;

		virtual void UploadMesh(Mesh& mesh) override;
		virtual void UploadMaterial(Material& material) override;
		virtual RenderHandle LoadTexture(const char* filename) override;

		virtual Scene& GetScene() override { return m_scene; }
		virtual const Scene& GetScene() const override { return m_scene; }
		virtual uint32_t GetObjectCount() const { return m_scene.Count(); }
		virtual void AddImGuiCallback(std::function<void()>&& fn) { m_imguiCallbackArray.push_back(fn); }

		virtual RenderHandle GetDefaultTexture() const;
		virtual Material GetDefaultMaterial() const;

		void ImmediateSubmit(std::function<void(VkCommandBuffer)>&& fn);
		const RenderContext& GetContext() const { return m_renderContext; }
		VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout layout);
		inline DescriptorLayoutCache& GetDescriptorSetLayoutCache() { return m_descriptorLayoutCache; }
		inline DescriptorAllocator& GetDescriptorAllocator() { return m_descriptorAllocator; }
	protected:
		void Draw();
		void DrawScene(VkCommandBuffer cmd, const Scene& scene);
		void WaitFence(VkFence fence, uint64_t timeoutSeconds = 1e9);
		RenderFrameContext& GetFrameContext();
		void ImGuiNewFrame();
		void ImGuiProcessEvent(const SDL_Event& e);

		// Initializations
		bool InitVulkan();
		bool InitCommands();
		bool InitFramebuffers();
		bool InitSync();
		bool InitPipeline();
		bool InitImGui();

		bool InitMaterial(const Material& materialHandle, MaterialRenderData& material);
		void SubmitMaterialTexture(MaterialRenderData& material, MaterialRenderData::ESamplerIndex sampler, const VkImageView& imageView);

		void UpdateBufferData(GPUBuffer* buffer, const void* data, uint32_t size);

	private:

		Window m_window;
		
		RenderContext m_renderContext;
		
		Swapchain m_swapchain;
		VkRenderPass m_renderPass;
		RenderPipeline m_renderPipeline;

		std::vector<VkFramebuffer> m_framebuffers;
		std::vector<Framebuffer> m_framebufferArray;

		RenderFrameContext m_frameContextArray[MaxOverlappedFrames];
		size_t m_frameCounter;

		DescriptorAllocator m_descriptorAllocator;
		DescriptorLayoutCache m_descriptorLayoutCache;

		enum EDescriptorLayoutType
		{
			DESCRIPTOR_SET_GLOBAL_LAYOUT,
			DESCRIPTOR_SET_TEXTURE_LAYOUT,

			DESCRIPTOR_SET_COUNT
		};
		VkDescriptorSetLayout m_descriptorLayouts[DESCRIPTOR_SET_COUNT];

		template <typename RenderResourceType>
		using ResourceMap = std::unordered_map<RenderHandle, RenderResourceType, RenderHandle::Hasher>;
		ResourceMap<RenderTexture> m_textures;
		ResourceMap<MeshRenderData> m_meshRenderData;
		ResourceMap<MaterialRenderData> m_materials;

		std::vector<IRendererBase*> m_renderers;

		Scene m_scene;
		bool m_dirtyCachedCamera;
		GPUCamera m_cachedCameraData;

		UploadContext m_immediateSubmitContext;

		FunctionStack m_shutdownStack;
		typedef std::function<void()> ImGuiCallback;
		std::vector<ImGuiCallback> m_imguiCallbackArray;
	};
}