// src file for vkmmc project 

#include "Shader.h"

#include <spirv-cross-main/spirv_glsl.hpp>

#include "RenderTypes.h"
#include "GenericUtils.h"
#include "Debug.h"
#include "InitVulkanTypes.h"
#include "RenderDescriptor.h"
#include "RenderContext.h"

namespace vkutils
{
	VkBufferUsageFlags GetVulkanBufferUsage(VkDescriptorType type)
	{
		switch (type)
		{
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: 
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}
		return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
	}

	const char* GetVulkanShaderStageName(VkShaderStageFlagBits shaderStage)
	{
		switch (shaderStage)
		{
		case VK_SHADER_STAGE_VERTEX_BIT: return "Vertex";
		case VK_SHADER_STAGE_FRAGMENT_BIT: return "Fragment";
		}
		return "Unknown";
	}

	const char* GetVulkanDescriptorTypeName(VkDescriptorType type)
	{
		switch (type)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLER: return "SAMPLER";
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return "COMBINED_IMAGE_SAMPLER";
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return "SAMPLED_IMAGE";
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return "STORAGE_IMAGE";
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return "UNIFORM_TEXEL_BUFFER";
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return "STORAGE_TEXEL_BUFFER";
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return "UNIFORM_BUFFER";
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: return "STORAGE_BUFFER";
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return "UNIFORM_BUFFER_DYNAMIC";
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return "STORAGE_BUFFER_DYNAMIC";
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return "INPUT_ATTACHMENT";
		case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: return "INLINE_UNIFORM_BLOCK";
		case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return "ACCELERATION_STRUCTURE_KHR";
		case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: return "ACCELERATION_STRUCTURE_NV";
		case VK_DESCRIPTOR_TYPE_MUTABLE_VALVE: return "MUTABLE_VALVE";
		case VK_DESCRIPTOR_TYPE_MAX_ENUM:
		default:
			check(false && "Invalid descriptor type.");
			break;
		}
		return "";
	}
}

namespace vkmmc
{
	ShaderCompiler::ShaderCompiler(const RenderContext& renderContext) : m_renderContext(renderContext)
	{}

	ShaderCompiler::~ShaderCompiler()
	{
		ClearCachedData();
	}

	void ShaderCompiler::ClearCachedData()
	{
		for (auto it : m_cachedBinarySources)
		{
			vkDestroyShaderModule(m_renderContext.Device, it.second.CompiledModule, nullptr);
		}
		m_cachedBinarySources.clear();
		m_cachedLayoutArray.clear();
		m_cachedPushConstantArray.clear();
	}

	bool ShaderCompiler::ProcessShaderFile(const char* filepath, VkShaderStageFlagBits shaderStage)
	{
		Logf(LogLevel::Ok, "Compiling shader: [%s: %s]\n", vkutils::GetVulkanShaderStageName(shaderStage), filepath);
		check(!m_cachedBinarySources.contains(shaderStage));
		CachedBinaryData data;
		data.ShaderStage = shaderStage;
		check(CheckShaderFileExtension(filepath, shaderStage));
		check(CacheSourceFromFile(filepath, data.BinarySource));
		ProcessCachedSource(data);
		data.CompiledModule = Compile(data.BinarySource, data.ShaderStage);
		m_cachedBinarySources[shaderStage] = data;
		Logf(LogLevel::Ok, "Shader compiled and cached successfully: [%s: %s]\n", vkutils::GetVulkanShaderStageName(shaderStage), filepath);
		return true;
	}

	VkShaderModule ShaderCompiler::GetCompiledModule(VkShaderStageFlagBits shaderStage) const
	{
		return m_cachedBinarySources.contains(shaderStage) ? m_cachedBinarySources.at(shaderStage).CompiledModule : VK_NULL_HANDLE;
	}

	bool ShaderCompiler::GenerateResources(DescriptorLayoutCache& layoutCache)
	{
		check(m_cachedLayoutArray.empty() && m_cachedPushConstantArray.empty() && "Compiler already has cached data. Compile all shaders before generate resources.");
		m_cachedLayoutArray.resize(m_reflectionProperties.DescriptorSetInfoArray.size());
		for (uint32_t i = 0; i < (uint32_t)m_reflectionProperties.DescriptorSetInfoArray.size(); ++i)
		{
			VkDescriptorSetLayout layout = GenerateDescriptorSetLayout(m_reflectionProperties.DescriptorSetInfoArray[i], layoutCache);
			m_cachedLayoutArray[i] = layout;
		}
		for (const auto& it : m_reflectionProperties.PushConstantMap)
			m_cachedPushConstantArray.push_back(GeneratePushConstantInfo(it.second));

		return true;
	}

	VkShaderModule ShaderCompiler::Compile(const std::vector<uint32_t>& binarySource, VkShaderStageFlagBits flag) const
	{
		check(!binarySource.empty());
		VkShaderModuleCreateInfo info{ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .pNext = nullptr };
		info.codeSize = binarySource.size() * sizeof(uint32_t);
		info.pCode = binarySource.data();
		VkShaderModule shaderModule{ VK_NULL_HANDLE };
		vkcheck(vkCreateShaderModule(m_renderContext.Device, &info, nullptr, &shaderModule));
		return shaderModule;
	}

	bool ShaderCompiler::CacheSourceFromFile(const char* file, std::vector<uint32_t>& outCachedData)
	{
		outCachedData.clear();
		return io::ReadFile(file, outCachedData);
	}

	void ShaderCompiler::ProcessCachedSource(CachedBinaryData& cachedData)
	{
		auto processSpirvResource = [this](const spirv_cross::CompilerGLSL& compiler, const spirv_cross::Resource& resource, VkShaderStageFlagBits shaderStage, VkDescriptorType descriptorType)
			{
				ShaderBindingDescriptorInfo bufferInfo;
				bufferInfo.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				bufferInfo.Name = resource.name.c_str();
				if (descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				{
					const spirv_cross::SPIRType& bufferType = compiler.get_type(resource.base_type_id);
					bufferInfo.Size = (uint32_t)compiler.get_declared_struct_size(bufferType);
					bufferInfo.ArrayCount = 1;
				}
				else
				{
					const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
					bufferInfo.Size = 0;
					bufferInfo.ArrayCount = type.array.size() > 0 ? type.array[0] : 1;
				}
				bufferInfo.Type = descriptorType;
				bufferInfo.Stage = shaderStage;

				uint32_t setIndex = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				ShaderDescriptorSetInfo& descriptorSetInfo = FindOrCreateDescriptorSet(setIndex);
				if (descriptorSetInfo.BindingArray.size() < bufferInfo.Binding + 1)
					descriptorSetInfo.BindingArray.resize(bufferInfo.Binding + 1);
				descriptorSetInfo.BindingArray[bufferInfo.Binding] = bufferInfo;

				Logf(LogLevel::Debug, "> %s [ShaderStage: %s; Name:%s; Set: %u; Binding: %u; Size: %u; ArrayCount: %u;]\n", vkutils::GetVulkanDescriptorTypeName(descriptorType),
					vkutils::GetVulkanShaderStageName(shaderStage), bufferInfo.Name.c_str(), setIndex, bufferInfo.Binding, bufferInfo.Size, bufferInfo.ArrayCount);
				return setIndex;
			};

		const std::vector<uint32_t>& cachedSource = cachedData.BinarySource;
		check(!cachedSource.empty() && "Invalid cached source.");
		spirv_cross::CompilerGLSL compiler(cachedSource);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		Log(LogLevel::Debug, "Processing shader reflection...\n");
		
		for (const spirv_cross::Resource& resource : resources.uniform_buffers)
			processSpirvResource(compiler, resource, cachedData.ShaderStage, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

		for (const spirv_cross::Resource& resource : resources.storage_buffers)
			processSpirvResource(compiler, resource, cachedData.ShaderStage, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		for (const spirv_cross::Resource& resource : resources.sampled_images)
			processSpirvResource(compiler, resource, cachedData.ShaderStage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		for (const spirv_cross::Resource& resource : resources.push_constant_buffers)
		{
			check(!m_reflectionProperties.PushConstantMap.contains(cachedData.ShaderStage));
			ShaderPushConstantBufferInfo info;
			info.Name = resource.name.c_str();
			info.Offset = compiler.get_decoration(resource.id, spv::DecorationOffset);
			const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
			info.Size = (uint32_t)compiler.get_declared_struct_size(type);
			info.ShaderStage = cachedData.ShaderStage;
			m_reflectionProperties.PushConstantMap[cachedData.ShaderStage] = info;
			Logf(LogLevel::Debug, "> PUSH_CONSTANT [ShaderStage: %s; Name: %s; Offset: %zd; Size: %zd]\n",
				vkutils::GetVulkanShaderStageName(cachedData.ShaderStage), info.Name.c_str(), info.Offset, info.Size);
		}

		Log(LogLevel::Debug, "End shader reflection.\n");
	}

	ShaderDescriptorSetInfo& ShaderCompiler::FindOrCreateDescriptorSet(uint32_t set)
	{
		for (uint32_t i = 0; i < (uint32_t)m_reflectionProperties.DescriptorSetInfoArray.size(); ++i)
		{
			if (m_reflectionProperties.DescriptorSetInfoArray[i].SetIndex == set)
				return m_reflectionProperties.DescriptorSetInfoArray[i];
		}

		ShaderDescriptorSetInfo info;
		info.SetIndex = set;
		m_reflectionProperties.DescriptorSetInfoArray.push_back(info);
		return m_reflectionProperties.DescriptorSetInfoArray.back();
	}

	const ShaderDescriptorSetInfo& ShaderCompiler::GetDescriptorSet(uint32_t set) const
	{
		for (uint32_t i = 0; i < (uint32_t)m_reflectionProperties.DescriptorSetInfoArray.size(); ++i)
		{
			if (m_reflectionProperties.DescriptorSetInfoArray[i].SetIndex == set)
				return m_reflectionProperties.DescriptorSetInfoArray[i];
		}
		check(false && "ShaderDescriptorSetInfo set index not found.");
		return m_reflectionProperties.DescriptorSetInfoArray[0];
	}

	VkDescriptorSetLayout ShaderCompiler::GenerateDescriptorSetLayout(const ShaderDescriptorSetInfo& setInfo, DescriptorLayoutCache& layoutCache) const
	{
		VkDescriptorSetLayout setLayout{ VK_NULL_HANDLE };
		std::vector<VkDescriptorSetLayoutBinding> bindingArray(setInfo.BindingArray.size());
		for (uint32_t i = 0; i < setInfo.BindingArray.size(); ++i)
		{
			bindingArray[i].binding = i;
			bindingArray[i].descriptorCount = setInfo.BindingArray[i].ArrayCount;
			bindingArray[i].descriptorType = setInfo.BindingArray[i].Type;
			bindingArray[i].pImmutableSamplers = nullptr;
			bindingArray[i].stageFlags = setInfo.BindingArray[i].Stage;
		}
		VkDescriptorSetLayoutCreateInfo createInfo = vkinit::DescriptorSetLayoutCreateInfo(bindingArray.data(), (uint32_t)bindingArray.size());
		setLayout = layoutCache.CreateLayout(createInfo);
		return setLayout;
	}

	VkPushConstantRange ShaderCompiler::GeneratePushConstantInfo(const ShaderPushConstantBufferInfo& pushConstantInfo) const
	{
		VkPushConstantRange range;
		range.offset = pushConstantInfo.Offset;
		range.size = pushConstantInfo.Size;
		range.stageFlags = pushConstantInfo.ShaderStage;
		return range;
	}

	bool ShaderCompiler::CheckShaderFileExtension(const char* filepath, VkShaderStageFlagBits shaderStage)
	{
		bool res = false;
		if (filepath && *filepath)
		{
			const char* desiredExt;
			switch (shaderStage)
			{
			case VK_SHADER_STAGE_FRAGMENT_BIT: desiredExt = ".frag.spv"; break;
			case VK_SHADER_STAGE_VERTEX_BIT: desiredExt = ".vert.spv"; break;
			default:
				return res;
			}
			size_t filepathLen = strlen(filepath);
			size_t desiredExtLen = strlen(desiredExt);
			if (filepathLen > desiredExtLen)
			{
				size_t ext = filepathLen - desiredExtLen;
				res = !strcmp(&filepath[ext], desiredExt);
			}
		}
		return res;
	}

}
