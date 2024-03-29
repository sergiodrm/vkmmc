
#pragma once

#include "RenderEngine.h"
#include "RenderPass.h"
#include "RenderHandle.h"
#include "RenderPipeline.h"
#include "RenderObject.h"
#include "RenderContext.h"
#include "RenderDescriptor.h"
#include "Texture.h"
#include "VulkanBuffer.h"
#include "Debug.h"
#include "Swapchain.h"
#include "Mesh.h"
#include "FunctionStack.h"
#include "Framebuffer.h"
#include "Scene.h"
#include <cstdio>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <string.h>
#include <chrono>
#include "Globals.h"

namespace vkmmc
{
	class Framebuffer;
	class IRendererBase;
	struct ShaderModuleLoadDescription;

	struct ProfilingTimer
	{
		std::chrono::high_resolution_clock::time_point m_start;

		void Start();
		double Stop();
	};

	struct ProfilerItem
	{
		double m_elapsed;
	};

	struct Profiler
	{
		std::unordered_map<std::string, ProfilerItem> m_items;
	};

	struct ScopedTimer
	{
		ScopedTimer(const char* nameId, Profiler* profiler);
		~ScopedTimer();

		std::string m_nameId;
		Profiler* m_profiler;
		ProfilingTimer m_timer;
	};

	struct RenderStats
	{
		Profiler Profiler;
		uint32_t TrianglesCount{ 0 };
		uint32_t DrawCalls{ 0 };
		uint32_t SetBindingCount{ 0 };
		void Reset();
	};
	extern RenderStats GRenderStats;
#define PROFILE_SCOPE(name) vkmmc::ScopedTimer __timer##name(#name, &vkmmc::GRenderStats.Profiler)

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

	struct CameraData
	{
		glm::mat4 View;
		glm::mat4 Projection;
		glm::mat4 ViewProjection;
	};

	

	struct RenderPassAttachment
	{
		AllocatedImage Image;
		std::vector<VkFramebuffer> FramebufferArray;
		std::vector<VkImageView> ImageViewArray;

		void Destroy(const RenderContext& renderContext);
	};
	
	class VulkanRenderEngine : public IRenderEngine
	{
	public:
		/**
		 * IRenderEngine interface
		 */
		virtual bool Init(const InitializationSpecs& initSpec) override;
		virtual bool RenderProcess() override;
		virtual void Shutdown() override;

		virtual void UpdateSceneView(const glm::mat4& view, const glm::mat4& projection) override;

		virtual IScene* GetScene() override;
		virtual const IScene* GetScene() const override;
		virtual void SetScene(IScene* scene);
		virtual void AddImGuiCallback(std::function<void()>&& fn) { m_imguiCallbackArray.push_back(fn); }
		virtual void SetAppEventCallback(std::function<void(void*)>&& fn) override { m_eventCallback = fn; }

		virtual RenderHandle GetDefaultTexture() const;
		virtual Material GetDefaultMaterial() const;
		const RenderContext& GetContext() const { return m_renderContext; }
		VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout layout);
		inline DescriptorLayoutCache& GetDescriptorSetLayoutCache() { return m_descriptorLayoutCache; }
		inline DescriptorAllocator& GetDescriptorAllocator() { return m_descriptorAllocator; }
		inline uint32_t GetFrameIndex() const { return m_frameCounter % globals::MaxOverlappedFrames; }
		inline uint32_t GetFrameCounter() const { return m_frameCounter; }
	protected:
		void BeginFrame();
		void Draw();
		void ImGuiDraw();
		void WaitFence(VkFence fence, uint64_t timeoutSeconds = 1e9);
		RenderFrameContext& GetFrameContext();

		// Initializations
		bool InitVulkan();
		bool InitCommands();
		bool InitRenderPass();
		bool InitFramebuffers();
		bool InitSync();
		bool InitPipeline();

	private:

		Window m_window;
		
		RenderContext m_renderContext;

		Swapchain m_swapchain;

		RenderPass m_renderPassArray[RENDER_PASS_COUNT];
		// One shadow map attachment per overlapped frames
		RenderPassAttachment m_shadowMapAttachments[globals::MaxOverlappedFrames];
		// One framebuffer attachment per swapchain image.
		std::vector<RenderPassAttachment> m_swapchainAttachments;

		RenderFrameContext m_frameContextArray[globals::MaxOverlappedFrames];
		uint32_t m_frameCounter;

		DescriptorAllocator m_descriptorAllocator;
		DescriptorLayoutCache m_descriptorLayoutCache;

		VkDescriptorSetLayout m_globalDescriptorLayout;

		std::vector<IRendererBase*> m_renderers[RENDER_PASS_COUNT];

		Scene* m_scene;
		CameraData m_cameraData;

		FunctionStack m_shutdownStack;
		typedef std::function<void()> ImGuiCallback;
		std::vector<ImGuiCallback> m_imguiCallbackArray;
		typedef std::function<void(void*)> EventCallback;
		EventCallback m_eventCallback;
	};

	extern RenderHandle GenerateRenderHandle();
}