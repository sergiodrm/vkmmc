// Autogenerated code for vkmmc project
// Header file

#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "VulkanBuffer.h"

struct SDL_Window;

namespace vkmmc
{
	struct TransferContext
	{
		VkFence Fence;
		VkCommandPool CommandPool;
		VkCommandBuffer CommandBuffer;
	};

	struct RenderContext
	{
		SDL_Window* Window;
		VkInstance Instance;
		VkPhysicalDevice GPUDevice;
		VkSurfaceKHR Surface;
		VkDebugUtilsMessengerEXT DebugMessenger;
		VkPhysicalDeviceProperties GPUProperties;
		VkDevice Device;
		VmaAllocator Allocator;
		VkQueue GraphicsQueue;
		uint32_t GraphicsQueueFamily;
		// Viewport size. TODO: Find another way to communicate these data.
		uint32_t Width;
		uint32_t Height;

		TransferContext TransferContext;
	};

	struct RenderFrameContext
	{
		// Sync vars
		VkFence RenderFence{};
		VkSemaphore RenderSemaphore{};
		VkSemaphore PresentSemaphore{};

		// Commands
		VkCommandPool GraphicsCommandPool{};
		VkCommandBuffer GraphicsCommand{};

		// Descriptors
		VkDescriptorSet CameraDescriptorSet{};
		UniformBuffer GlobalBuffer{};

		// Push constants
		const void* PushConstantData{ nullptr };
		uint32_t PushConstantSize{ 0 };

		// Scene to draw
		class Scene* Scene;
		struct CameraData* CameraData;
		// Frame
		uint32_t FrameIndex;
	};

}
