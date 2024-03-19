// Autogenerated code for vkmmc project
// Source file
#include "SceneImpl.h"
#include "Debug.h"

#define CGLTF_IMPLEMENTATION
#pragma warning(disable:4996)
#include <gltf/cgltf.h>
#undef CGLTF_IMPLEMENTATION

#ifdef VKMMC_MEM_MANAGEMENT
// TODO: wtf windows declare these macros and project does not compile when VKMMC_MEM_MANAGEMENT is defined. WTF!?
#undef min
#undef max  
#endif // VKMMC_MEM_MANAGEMENT

#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>
#include "Mesh.h"
#include "GenericUtils.h"
#include "VulkanRenderEngine.h"
#include <algorithm>
#include <imgui.h>
#include "Renderers/DebugRenderer.h"

//#define VKMMC_ENABLE_LOADER_LOG

namespace gltf_api
{
	void HandleError(cgltf_result result, const char* filepath)
	{
		switch (result)
		{
		case cgltf_result_success:
			break;
		case cgltf_result_data_too_short:
			break;
		case cgltf_result_unknown_format:
			break;
		case cgltf_result_invalid_json:
			break;
		case cgltf_result_invalid_gltf:
			break;
		case cgltf_result_invalid_options:
			break;
		case cgltf_result_file_not_found:
			break;
		case cgltf_result_io_error:
			break;
		case cgltf_result_out_of_memory:
			break;
		case cgltf_result_legacy_gltf:
			break;
		case cgltf_result_max_enum:
			break;
		default:
			break;
		}
	}

	float Length2(const glm::vec3& vec)
	{
		return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
	}

	float Length(const glm::vec3& vec) { return sqrtf(Length2(vec)); }

	uint32_t GetElementCountFromType(cgltf_type type)
	{
		switch (type)
		{
		case cgltf_type_scalar: return 1;
		case cgltf_type_vec2: return 2;
		case cgltf_type_vec3: return 3;
		case cgltf_type_vec4: return 4;
		case cgltf_type_mat2: return 2 * 2;
		case cgltf_type_mat3: return 3 * 3;
		case cgltf_type_mat4: return 4 * 4;
		case cgltf_type_invalid:
		case cgltf_type_max_enum:
		default:
			check(false && "Invalid cgltf_type.");
			break;
		}
		return 0;
	}

	void ReadValue(void* dst, const cgltf_float* data, uint32_t count)
	{
		memcpy_s(dst, sizeof(float) * count, data, sizeof(float) * count);
	}

	void ToMat4(glm::mat4* mat, const cgltf_float* cgltfMat4)
	{
		ReadValue(mat, cgltfMat4, 16);
	}

	void ToVec2(glm::vec2& v, const cgltf_float* data)
	{
		ReadValue(&v, data, 2);
	}

	void ToVec3(glm::vec3& v, const cgltf_float* data)
	{
		ReadValue(&v, data, 3);
	}

	void ToVec4(glm::vec4& v, const cgltf_float* data)
	{
		ReadValue(&v, data, 4);
	}

	void ToQuat(glm::quat& q, const cgltf_float* data)
	{
		q = glm::quat(data[3], data[0], data[1], data[2]);
	}

	void ReadNodeLocalTransform(const cgltf_node& node, glm::mat4& t)
	{
		if (node.has_matrix)
		{
			gltf_api::ToMat4(&t, node.matrix);
		}
		else
		{
			if (node.has_translation)
			{
				glm::vec3 pos;
				gltf_api::ToVec3(pos, node.translation);
				t = glm::translate(t, pos);
			}
			if (node.has_rotation)
			{
				glm::quat quat;
				gltf_api::ToQuat(quat, node.rotation);
				t *= glm::toMat4(quat);
			}
			if (node.has_scale)
			{
				glm::vec3 scl;
				gltf_api::ToVec3(scl, node.scale);
				t = glm::scale(t, scl);
			}
		}
	}

	void ReadValues(std::vector<float>& values, const cgltf_accessor& accessor)
	{
		uint32_t elementCount = GetElementCountFromType(accessor.type);
		values.resize(accessor.count * elementCount);
		for (uint32_t i = 0; i < accessor.count; ++i)
			cgltf_accessor_read_float(&accessor, i, &values[i * elementCount], elementCount);
	}

	void ReadAttribute(vkmmc::Vertex& vertex, const float* source, uint32_t index, cgltf_attribute_type type)
	{
		const char* attributeName = nullptr;
		const float* data = &source[index];
		switch (type)
		{
		case cgltf_attribute_type_position:
			ToVec3(vertex.Position, data);
			break;
		case cgltf_attribute_type_texcoord:
			ToVec2(vertex.TexCoords, data);
			break;
		case cgltf_attribute_type_normal:
			ToVec3(vertex.Normal, data);
			if (Length2(vertex.Normal) < 1e-5f)
				vertex.Normal = glm::vec3{ 0.f, 1.f, 0.f };
			else
				vertex.Normal = glm::normalize(vertex.Normal);
			break;
		case cgltf_attribute_type_color:
			ToVec3(vertex.Color, data);
			break;
		case cgltf_attribute_type_tangent: 
			ToVec3(vertex.Tangent, data);
			break;
		case cgltf_attribute_type_joints: 
#ifdef VKMMC_ENABLE_LOADER_LOG
			attributeName = "joints";
#endif // VKMMC_ENABLE_LOADER_LOG

			break;
		case cgltf_attribute_type_weights: 
#ifdef VKMMC_ENABLE_LOADER_LOG
			attributeName = "weights";
#endif // VKMMC_ENABLE_LOADER_LOG
			break;
		case cgltf_attribute_type_invalid:
		case cgltf_attribute_type_custom:
		case cgltf_attribute_type_max_enum:
			check(false && "Invalid attribute type to read.");
		default:
			break;
		}
#ifdef VKMMC_ENABLE_LOADER_LOG
		if (attributeName)
			vkmmc::Logf(vkmmc::LogLevel::Error, "gltf loader: Attribute type not suported yet [%s].\n", attributeName);
#endif // VKMMC_ENABLE_LOADER_LOG

	}

	// Attributes are an continuous array of positions, normals, uvs...
	// We have to map from struct of arrays to our format, array of structs (std::vector<vkmmc::Vertex>)
	void ReadAttributeArray(vkmmc::Vertex* vertices, const cgltf_attribute& attribute, const cgltf_node* nodes, uint32_t nodeCount)
	{
		const cgltf_accessor* accessor = attribute.data;
		uint32_t accessorCount = (uint32_t)accessor->count;
		// Get how many values has current attribute
		uint32_t elementCount = GetElementCountFromType(accessor->type);
		// Read data from accessor
		std::vector<float> values;
		ReadValues(values, *accessor);
		// Map to internal format
		for (uint32_t i = 0; i < accessorCount; ++i)
		{
			vkmmc::Vertex& vertex = vertices[i];
			uint32_t indexValue = i * elementCount;
			ReadAttribute(vertex, values.data(), indexValue, attribute.type);
		}
	}

	void FreeData(cgltf_data* data)
	{
		cgltf_free(data);
	}

	cgltf_data* ParseFile(const char* filepath)
	{
		cgltf_options options;
		memset(&options, 0, sizeof(cgltf_options));
		cgltf_data* data{ nullptr };
		cgltf_result result = cgltf_parse_file(&options, filepath, &data);
		if (result != cgltf_result_success)
		{
			HandleError(result, filepath);
			return nullptr;
		}
		result = cgltf_load_buffers(&options, data, filepath);
		if (result != cgltf_result_success)
		{
			HandleError(result, filepath);
			return nullptr;
		}
		result = cgltf_validate(data);
		if (result != cgltf_result_success)
		{
			HandleError(result, filepath);
			FreeData(data);
			return nullptr;
		}
		return data;
	}

	void LoadVertices(std::vector<vkmmc::Vertex>& vertices, const cgltf_primitive* primitive, const cgltf_node* nodes, uint32_t nodeCount)
	{
		uint32_t attributeCount = (uint32_t)primitive->attributes_count;
		uint32_t vertexOffset = (uint32_t)vertices.size();
		uint32_t vertexCount = (uint32_t)primitive->attributes[0].data->count;
		vertices.resize(vertexCount + vertexOffset);
		for (uint32_t i = 0; i < attributeCount; ++i)
		{
			const cgltf_attribute& attribute = primitive->attributes[i];
			check(attribute.data->count == vertexCount);
			ReadAttributeArray(vertices.data() + vertexOffset, attribute, nodes, nodeCount);
		}
	}

	void LoadIndices(std::vector<uint32_t>& indices, const cgltf_primitive* primitive, uint32_t offset)
	{
		check(primitive->indices);
		uint32_t indexCount = (uint32_t)primitive->indices->count;
		uint32_t indexOffset = (uint32_t)indices.size();
		indices.resize(indexCount + indexOffset);
		for (uint32_t i = 0; i < indexCount; ++i)
			indices[i + indexOffset] = (uint32_t)cgltf_accessor_read_index(primitive->indices, i) + offset;
	}

	vkmmc::RenderHandle LoadTexture(vkmmc::Scene* scene, const char* rootAssetPath, const cgltf_texture_view& texView)
	{
		char texturePath[512];
		sprintf_s(texturePath, "%s%s", rootAssetPath, texView.texture->image->uri);
		vkmmc::RenderHandle handle = scene->LoadTexture(texturePath);
		return handle;
	}

	void LoadMaterial(vkmmc::Scene* scene, const char* rootAssetPath, vkmmc::Material& material, const cgltf_material& mtl)
	{
		if (mtl.pbr_metallic_roughness.base_color_texture.texture)
		{
			vkmmc::RenderHandle diffHandle = LoadTexture(scene,
				rootAssetPath,
				mtl.pbr_metallic_roughness.base_color_texture);
			material.SetDiffuseTexture(diffHandle);
		}
#ifdef VKMMC_ENABLE_LOADER_LOG
		else
			vkmmc::Log(vkmmc::LogLevel::Warn, "Diffuse material texture not found.\n");
#endif // VKMMC_ENABLE_LOADER_LOG

		if (mtl.pbr_specular_glossiness.diffuse_texture.texture)
		{
			vkmmc::RenderHandle handle = LoadTexture(scene,
				rootAssetPath,
				mtl.pbr_specular_glossiness.diffuse_texture);
			material.SetSpecularTexture(handle);
			
			vkmmc::Log(vkmmc::LogLevel::Warn, "Specular.\n");
		}
#ifdef VKMMC_ENABLE_LOADER_LOG
		else
			vkmmc::Log(vkmmc::LogLevel::Warn, "Specular material texture not found.\n");
#endif // VKMMC_ENABLE_LOADER_LOG

		if (mtl.normal_texture.texture)
		{
			vkmmc::RenderHandle handle = LoadTexture(scene,
				rootAssetPath,
				mtl.normal_texture);
			material.SetNormalTexture(handle);
		}
#ifdef VKMMC_ENABLE_LOADER_LOG
		else
			vkmmc::Log(vkmmc::LogLevel::Warn, "Normal material texture not found.\n");
#endif // VKMMC_ENABLE_LOADER_LOG
	}
}

namespace vkmmc
{
	void MeshRenderData::BindBuffers(VkCommandBuffer cmd) const
	{
		VertexBuffer.Bind(cmd);
		IndexBuffer.Bind(cmd);
	}

	VkDescriptorSetLayout MaterialRenderData::GetDescriptorSetLayout(const RenderContext& renderContext, DescriptorLayoutCache& layoutCache)
	{
		VkDescriptorSetLayout layout;
		DescriptorSetLayoutBuilder::Create(layoutCache)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3)
			.Build(renderContext, &layout);
		return layout;
	}

	void MaterialRenderData::Init(const RenderContext& renderContext, DescriptorAllocator& descAllocator, DescriptorLayoutCache& layoutCache)
	{
		if (Layout == VK_NULL_HANDLE)
		{
			Layout = GetDescriptorSetLayout(renderContext, layoutCache);
		}
		if (Set == VK_NULL_HANDLE)
		{
			descAllocator.Allocate(&Set, Layout);
		}
		if (Sampler == VK_NULL_HANDLE)
		{
			VkSamplerCreateInfo samplerCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
				.pNext = nullptr,
				.magFilter = VK_FILTER_LINEAR,
				.minFilter = VK_FILTER_LINEAR,
				.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
				.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
				.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT
			};
			vkcheck(vkCreateSampler(renderContext.Device, &samplerCreateInfo, nullptr, &Sampler));
		}
	}

	void MaterialRenderData::Destroy(const RenderContext& renderContext)
	{
		vkDestroySampler(renderContext.Device, Sampler, nullptr);
	}


	EnvironmentData::EnvironmentData() :
		AmbientColor(0.02f, 0.02f, 0.02f),
		ActiveSpotLightsCount(0.f),
		ViewPosition(0.f),
		ActiveLightsCount(0.f)
	{
		DirectionalLight.Color = { 0.05f, 0.01f, 0.1f };
		DirectionalLight.Position = { 0.f, 0.f, 1.f };
		DirectionalLight.ShadowMapIndex = -1.f;
		DirectionalLight.Compression = 0.5f;
		for (uint32_t i = 0; i < MaxLights; ++i)
		{
			Lights[i] = DirectionalLight;
			Lights[i].Radius = 50.f;
			SpotLights[i].Direction = { 1.f, 0.f, 0.f };
			SpotLights[i].Position = { 0.f, 0.f, 0.f };
			SpotLights[i].Color = { 1.f, 0.f, 0.f };
			SpotLights[i].InnerCutoff = 1.f;
			SpotLights[i].OuterCutoff = 0.5f;
			SpotLights[i].ShadowMapIndex = -1.f;
		}
	}


	IScene* IScene::CreateScene(IRenderEngine* engine)
	{
		check(!engine->GetScene());
		Scene* scene = new Scene(engine);
		engine->SetScene(scene);
		return scene;
	}

	IScene* IScene::LoadScene(IRenderEngine* engine, const char* sceneFilepath)
	{
		Scene* scene = static_cast<Scene*>(CreateScene(engine));
		if (!scene->LoadModel(sceneFilepath))
		{
			Logf(LogLevel::Error, "Error loading scene from: %s.\n", sceneFilepath);
			DestroyScene(scene);
			scene = nullptr;
		}
		return scene;
	}

	void IScene::DestroyScene(IScene* scene)
	{
		scene->Destroy();
		delete scene;
	}

	Scene::Scene(IRenderEngine* engine) : IScene(), m_engine(static_cast<VulkanRenderEngine*>(engine))
	{
		// Create root scene
		CreateRenderObject(RenderObject::InvalidId);
	}

	Scene::~Scene()
	{}

	void Scene::Init()
	{}

	void Scene::Destroy()
	{
		m_localTransforms.clear();
		m_globalTransforms.clear();
		m_hierarchy.clear();
		m_meshArray.clear();
		m_componentMap.clear();
		m_names.clear();
		for (uint32_t i = 0; i < MaxNodeLevel; ++i)
			m_dirtyNodes[i].clear();

		const RenderContext& renderContext = m_engine->GetContext();
		for (auto& it : m_renderData.Meshes)
		{
			it.second.VertexBuffer.Destroy(renderContext);
			it.second.IndexBuffer.Destroy(renderContext);
		}
		for (auto& it : m_renderData.Materials)
		{
			it.second.Destroy(m_engine->GetContext());
		}
		for (auto& it : m_renderData.Textures)
		{
			it.second.Destroy(m_engine->GetContext());
		}
	}

	RenderObject Scene::CreateRenderObject(RenderObject parent)
	{
		// Generate new node in all basics structures
		RenderObject node = (uint32_t)m_hierarchy.size();
		m_localTransforms.push_back(glm::mat4(1.f));
		m_globalTransforms.push_back(glm::mat4(1.f));
		char buff[64];
		sprintf_s(buff, "RenderObject_%u", node.Id);
		m_names.push_back(buff);
		m_hierarchy.push_back({ .Parent = parent });

		// Connect siblings
		if (parent.IsValid())
		{
			check(parent.Id < (uint32_t)m_hierarchy.size());
			RenderObject firstSibling = m_hierarchy[parent].Child;
			if (firstSibling.IsValid())
			{
				RenderObject sibling;
				for (sibling = firstSibling; m_hierarchy[sibling].Sibling.IsValid(); sibling = m_hierarchy[sibling].Sibling);
				m_hierarchy[sibling].Sibling = node;
			}
			else
			{
				m_hierarchy[parent].Child = node;
			}
		}

		m_hierarchy[node].Level = parent.IsValid() ? m_hierarchy[parent].Level + 1 : 0;
		m_hierarchy[node].Child = RenderObject::InvalidId;
		m_hierarchy[node].Sibling = RenderObject::InvalidId;
		return node;
	}

	bool Scene::LoadModel(const char* filepath)
	{
		cgltf_data* data = gltf_api::ParseFile(filepath);
		char rootAssetPath[512];
		io::GetRootDir(filepath, rootAssetPath, 512);
		if (!data)
		{
			Logf(LogLevel::Error, "Cannot open file to load scene model: %s.\n", filepath);
			return false;
		}

		std::unordered_map<cgltf_material*, uint32_t> materialIndexMap;
		for (uint32_t i = 0; i < data->materials_count; ++i)
		{
			Material m;
			gltf_api::LoadMaterial(this, rootAssetPath, m, data->materials[i]);
			materialIndexMap[&data->materials[i]] = GetMaterialCount();
			SubmitMaterial(m);
		}

		uint32_t nodesCount = (uint32_t)data->nodes_count;
		uint32_t renderObjectOffset = GetRenderObjectCount();
		for (uint32_t i = 0; i < nodesCount; ++i)
		{
			const cgltf_node& node = data->nodes[i];
			RenderObject parent = GetRoot();
			if (node.parent)
			{
				for (uint32_t j = 0; j < i; ++j)
				{
					if (&data->nodes[j] == node.parent)
					{
						parent = j + renderObjectOffset;
						break;
					}
				}
			}
			RenderObject renderObject = CreateRenderObject(parent);

			// Process transform
			glm::mat4 localTransform(1.f);
			gltf_api::ReadNodeLocalTransform(node, localTransform);
			SetTransform(renderObject, localTransform);

			// Process mesh
			if (node.mesh)
			{
				// Load geometry data
				std::vector<Vertex> vertices;
				std::vector<uint32_t> indices;
				std::vector<PrimitiveMeshData> pmd(node.mesh->primitives_count);

				for (uint32_t j = 0; j < node.mesh->primitives_count; ++j)
				{
					const cgltf_primitive& primitive = node.mesh->primitives[j];
					uint32_t vertexOffset = (uint32_t)vertices.size();
					pmd[j].FirstIndex = (uint32_t)indices.size();
					gltf_api::LoadIndices(indices, &primitive, vertexOffset);
					gltf_api::LoadVertices(vertices, &primitive, data->nodes, (uint32_t)data->nodes_count);
					pmd[j].Count = (uint32_t)indices.size() - pmd[j].FirstIndex;
					pmd[j].MaterialIndex = materialIndexMap[primitive.material];
					check(primitive.indices->count == pmd[j].Count);
				}

				// Sort primitives by index
				std::sort(pmd.begin(), pmd.end(),
					[](const PrimitiveMeshData& a, const PrimitiveMeshData& b) {return a.MaterialIndex < b.MaterialIndex; }
				);

				// Submit data
				Mesh mesh;
				mesh.MoveIndicesFrom(indices);
				mesh.MoveVerticesFrom(vertices);
				SubmitMesh(mesh);

				// TODO: find out a better way to assign primitives to mesh render data.
				MeshRenderData& mrd = GetMeshRenderData(mesh.GetHandle());
				mrd.PrimitiveArray = std::move(pmd);
				mrd.IndexCount = (uint32_t)mesh.GetIndexCount();

				SetMesh(renderObject, mesh);

				if (node.mesh->name && *node.mesh->name)
					SetRenderObjectName(renderObject, node.mesh->name);
			}
		}
		gltf_api::FreeData(data);
		return true;
	}

	void Scene::DestroyRenderObject(RenderObject object)
	{
		check(false && "not implemented yet" && __FUNCTION__ && __FILE__ && __LINE__);
	}

	const Mesh* Scene::GetMesh(RenderObject renderObject) const
	{
		check(IsValid(renderObject));
		const Mesh* mesh = nullptr;
		if (m_componentMap.contains(renderObject))
		{
			uint32_t index = m_componentMap.at(renderObject).MeshIndex;
			check(index < (uint32_t)m_meshArray.size());
			mesh = &m_meshArray.at(index);
		}
		return mesh;
	}

	void Scene::SetMesh(RenderObject renderObject, const Mesh& mesh)
	{
		check(IsValid(renderObject));
		if (m_componentMap.contains(renderObject))
		{
			RenderObjectComponents& components = m_componentMap[renderObject];
			if (components.MeshIndex >= (uint32_t)m_meshArray.size())
			{
				components.MeshIndex = (uint32_t)m_meshArray.size();
				m_meshArray.push_back(mesh);
			}
			else
				m_meshArray[components.MeshIndex] = mesh;
		}
		else
		{
			uint32_t index = (uint32_t)m_meshArray.size();
			m_meshArray.push_back(mesh);
			m_componentMap[renderObject] = { .MeshIndex = index };
		}
	}

	const char* Scene::GetRenderObjectName(RenderObject object) const
	{
		return IsValid(object) ? m_names[object.Id].c_str() : nullptr;
	}

	bool Scene::IsValid(RenderObject object) const
	{
		return object.Id < (uint32_t)m_hierarchy.size();
	}

	uint32_t Scene::GetRenderObjectCount() const
	{
		return (uint32_t)m_hierarchy.size();
	}

	RenderObject Scene::GetRoot() const
	{
		static RenderObject root = 0;
		//for (; m_hierarchy[root].Parent.IsValid(); root = m_hierarchy[root].Parent);
		return root;
	}

	void Scene::SetRenderObjectName(RenderObject renderObject, const char* name)
	{
		check(IsValid(renderObject));
		m_names[renderObject] = name;
	}

	const glm::mat4& Scene::GetTransform(RenderObject renderObject) const
	{
		check(IsValid(renderObject));
		return m_localTransforms[renderObject.Id];
	}

	void Scene::SetTransform(RenderObject renderObject, const glm::mat4& transform)
	{
		check(IsValid(renderObject));
		m_localTransforms[renderObject] = transform;
		MarkAsDirty(renderObject);
	}

	const Light* Scene::GetLight(RenderObject renderObject) const
	{
		check(IsValid(renderObject));
		check(m_componentMap.contains(renderObject));
		uint32_t index = m_componentMap.at(renderObject).LightIndex;
		check(index < (uint32_t)m_lightArray.size());
		return &m_lightArray[index];
	}

	void Scene::SetLight(RenderObject renderObject, const Light& light)
	{
		check(IsValid(renderObject));
		if (m_componentMap.contains(renderObject))
		{
			RenderObjectComponents& components = m_componentMap[renderObject];
			if (components.LightIndex >= (uint32_t)m_lightArray.size())
			{
				components.LightIndex = (uint32_t)m_lightArray.size();
				m_lightArray.push_back(light);
			}
			else
				m_lightArray[components.LightIndex] = light;
		}
		else
		{
			uint32_t index = (uint32_t)m_lightArray.size();
			m_componentMap[renderObject] = { .LightIndex = index };
			m_lightArray.push_back(light);
		}
	}

	void Scene::SubmitMesh(Mesh& mesh)
	{
		check(!mesh.GetHandle().IsValid());
		check(mesh.GetVertexCount() > 0 && mesh.GetIndexCount() > 0);
		// Prepare buffer creation
		const uint32_t vertexBufferSize = (uint32_t)mesh.GetVertexCount() * sizeof(Vertex);

		MeshRenderData mrd{};
		// Create vertex buffer
		mrd.VertexBuffer.Init(m_engine->GetContext(), { .Size = vertexBufferSize });
		GPUBuffer::SubmitBufferToGpu(mrd.VertexBuffer, mesh.GetVertices(), vertexBufferSize);

		uint32_t indexBufferSize = (uint32_t)(mesh.GetIndexCount() * sizeof(uint32_t));
		mrd.IndexBuffer.Init(m_engine->GetContext(), { .Size = indexBufferSize });
		GPUBuffer::SubmitBufferToGpu(mrd.IndexBuffer, mesh.GetIndices(), indexBufferSize);

		// Register new buffer
		RenderHandle handle = GenerateRenderHandle();
		mesh.SetHandle(handle);
		m_renderData.Meshes[handle] = mrd;
	}

	void Scene::SubmitMaterial(Material& material)
	{
		check(!material.GetHandle().IsValid());

		RenderHandle h = GenerateRenderHandle();
		MaterialRenderData mrd;
		mrd.Init(m_engine->GetContext(), m_engine->GetDescriptorAllocator(), m_engine->GetDescriptorSetLayoutCache());

		auto submitTexture = [this](RenderHandle texHandle, MaterialRenderData& mrd, uint32_t binding, uint32_t arrayIndex)
			{
				Texture texture;
				if (texHandle.IsValid())
				{
					texture = m_renderData.Textures[texHandle];
				}
				else
				{
					RenderHandle defTex = m_engine->GetDefaultTexture();
					texture = m_renderData.Textures[defTex];
				}
				texture.Bind(m_engine->GetContext(), mrd.Set, mrd.Sampler, binding, arrayIndex);
			};
		submitTexture(material.GetDiffuseTexture(), mrd, 0, 0);
		submitTexture(material.GetNormalTexture(), mrd, 0, 1);
		submitTexture(material.GetSpecularTexture(), mrd, 0, 2);

		material.SetHandle(h);
		m_renderData.Materials[h] = mrd;
		m_materialArray.push_back(material);
	}

	RenderHandle Scene::LoadTexture(const char* texturePath)
	{
		// Load texture from file
		io::TextureRaw texData;
		if (!io::LoadTexture(texturePath, texData))
		{
			Logf(LogLevel::Error, "Failed to load texture from %s.\n", texturePath);
			return InvalidRenderHandle;
		}

		// Create gpu buffer with texture specifications
		Texture texture;
		texture.Init(m_engine->GetContext(), texData);
		RenderHandle h = GenerateRenderHandle();
		m_renderData.Textures[h] = texture;

		// Free raw texture data
		io::FreeTexture(texData.Pixels);
		return h;
	}

	void Scene::MarkAsDirty(RenderObject renderObject)
	{
		check(IsValid(renderObject));
		int32_t level = m_hierarchy[renderObject].Level;
		m_dirtyNodes[level].push_back(renderObject);
		for (RenderObject child = m_hierarchy[renderObject].Child; child.IsValid(); child = m_hierarchy[child].Sibling)
			MarkAsDirty(child);
	}

	void Scene::RecalculateTransforms()
	{
		// Process root level first
		if (!m_dirtyNodes[0].empty())
		{
			for (uint32_t i = 0; i < m_dirtyNodes[0].size(); ++i)
			{
				uint32_t nodeIndex = m_dirtyNodes[0][i];
				m_globalTransforms[nodeIndex] = m_localTransforms[nodeIndex];
			}
			m_dirtyNodes[0].clear();
		}
		// Iterate over the deeper levels
		for (uint32_t level = 1; level < MaxNodeLevel && !m_dirtyNodes[level].empty(); ++level)
		{
			for (uint32_t nodeIndex = 0; nodeIndex < m_dirtyNodes[level].size(); ++nodeIndex)
			{
				int32_t node = m_dirtyNodes[level][nodeIndex];
				int32_t parentNode = m_hierarchy[node].Parent;
				m_globalTransforms[node] = m_globalTransforms[parentNode] * m_localTransforms[node];
			}
			m_dirtyNodes[level].clear();
		}
	}

	const Mesh* Scene::GetMeshArray() const
	{
		return m_meshArray.data();
	}

	uint32_t Scene::GetMeshCount() const
	{
		return (uint32_t)m_meshArray.size();
	}

	const Light* Scene::GetLightArray() const
	{
		return m_lightArray.data();
	}

	uint32_t Scene::GetLightCount() const
	{
		return (uint32_t)m_lightArray.size();
	}

	const Material* Scene::GetMaterialArray() const
	{
		return m_materialArray.data();
	}

	uint32_t Scene::GetMaterialCount() const
	{
		return (uint32_t)m_materialArray.size();
	}

	const MeshRenderData& Scene::GetMeshRenderData(RenderHandle handle) const
	{
		return m_renderData.Meshes.at(handle);
	}

	MeshRenderData& Scene::GetMeshRenderData(RenderHandle handle)
	{
		return m_renderData.Meshes.at(handle);
	}

	const MaterialRenderData& Scene::GetMaterialRenderData(RenderHandle handle) const
	{
		return m_renderData.Materials.at(handle);
	}

	MaterialRenderData& Scene::GetMaterialRenderData(RenderHandle handle)
	{
		return m_renderData.Materials.at(handle);
	}

	void Scene::Draw(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t materialSetIndex, uint32_t modelSetIndex, VkDescriptorSet modelSet) const
	{
		CPU_PROFILE_SCOPE(Scene_Draw);
		// Iterate scene graph to render models.
		uint32_t lastMaterialIndex = UINT32_MAX;
		const Mesh* lastMesh = nullptr;
		uint32_t nodeCount = GetRenderObjectCount();
		for (uint32_t i = 0; i < nodeCount; ++i)
		{
			RenderObject renderObject = i;
			const Mesh* mesh = GetMesh(renderObject);
			if (mesh)
			{
				const MeshRenderData& mrd = GetMeshRenderData(mesh->GetHandle());

				// BaseOffset in buffer is already setted when descriptor was created.
				uint32_t modelDynamicOffset = i * sizeof(glm::mat4);
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineLayout, modelSetIndex, 1, &modelSet, 1, &modelDynamicOffset);
				++vkmmc_profiling::GRenderStats.SetBindingCount;

				// Bind vertex/index buffers just if needed
				if (lastMesh != mesh)
				{
					check(mesh->GetHandle().IsValid());
					lastMesh = mesh;
					mrd.BindBuffers(cmd);
				}
				// Iterate primitives of current mesh
				for (uint32_t j = 0; j < (uint32_t)mrd.PrimitiveArray.size(); ++j)
				{
					const PrimitiveMeshData& drawData = mrd.PrimitiveArray[j];
					// TODO: material by default if there is no material.
					if (lastMaterialIndex != drawData.MaterialIndex)
					{
						lastMaterialIndex = drawData.MaterialIndex;
						const Material* material = &GetMaterialArray()[drawData.MaterialIndex];
						check(material && material->GetHandle().IsValid());
						const MaterialRenderData& mtl = GetMaterialRenderData(material->GetHandle());
						vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
							pipelineLayout, materialSetIndex, 1, &mtl.Set, 0, nullptr);
						++vkmmc_profiling::GRenderStats.SetBindingCount;
					}
					vkCmdDrawIndexed(cmd, drawData.Count, 1, drawData.FirstIndex, 0, 0);
					++vkmmc_profiling::GRenderStats.DrawCalls;
					vkmmc_profiling::GRenderStats.TrianglesCount += drawData.Count / 3;
				}
			}
		}
	}

	void Scene::Draw(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t modelSetIndex, VkDescriptorSet modelSet) const
	{
		CPU_PROFILE_SCOPE(Scene_Draw);
		// Iterate scene graph to render models.
		uint32_t lastMaterialIndex = UINT32_MAX;
		const Mesh* lastMesh = nullptr;
		uint32_t nodeCount = GetRenderObjectCount();
		for (uint32_t i = 0; i < nodeCount; ++i)
		{
			RenderObject renderObject = i;
			const Mesh* mesh = GetMesh(renderObject);
			if (mesh)
			{
				const MeshRenderData& mrd = GetMeshRenderData(mesh->GetHandle());

				// BaseOffset in buffer is already setted when descriptor was created.
				uint32_t modelDynamicOffset = i * sizeof(glm::mat4);
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineLayout, modelSetIndex, 1, &modelSet, 1, &modelDynamicOffset);
				++vkmmc_profiling::GRenderStats.SetBindingCount;

				// Bind vertex/index buffers just if needed
				if (lastMesh != mesh)
				{
					check(mesh->GetHandle().IsValid());
					lastMesh = mesh;
					mrd.BindBuffers(cmd);
				}
				// TODO: index buffer is ordered the whole buffer or just by primitive?
				vkCmdDrawIndexed(cmd, mesh->GetIndexCount(), 1, 0, 0, 0);
#if 0
				for (uint32_t j = 0; j < (uint32_t)mrd.PrimitiveArray.size(); ++j)
				{
					const PrimitiveMeshData& drawData = mrd.PrimitiveArray[j];
					vkCmdDrawIndexed(cmd, drawData.Count, 1, drawData.FirstIndex, 0, 0);
					++GRenderStats.DrawCalls;
					GRenderStats.TrianglesCount += drawData.Count / 3;
				}
#endif // 0

			}
		}
	}

	void Scene::ImGuiDraw(bool createWindow)
	{
		if (createWindow)
			ImGui::Begin("SceneGraph");
		auto utilDragFloat = [](const char* label, uint32_t id, float* data, uint32_t count, bool asColor, float diff = 1.f, float minLimit = 0.f, float maxLimit = 0.f)
			{
				ImGui::Columns(2);
				ImGui::Text(label);
				ImGui::NextColumn();
				char buff[64];
				sprintf_s(buff, "##%s%u", label, id);
				bool r = false;
				switch (count)
				{
				case 1: r = ImGui::DragFloat(buff, data, diff, minLimit, maxLimit); break;
				case 2: r = ImGui::DragFloat2(buff, data, diff, minLimit, maxLimit); break;
				case 3: r = asColor ? ImGui::ColorEdit3(buff, data) : ImGui::DragFloat3(buff, data, diff, minLimit, maxLimit); break;
				case 4: r = asColor ? ImGui::ColorEdit4(buff, data) : ImGui::DragFloat4(buff, data, diff, minLimit, maxLimit); break;
				}
				ImGui::NextColumn();
				ImGui::Columns();
				return r;
			};
		auto utilShadowCheckbox = [](const char* label, uint32_t id, float& shadowMapIndex)
			{
				bool projectShadow = shadowMapIndex >= 0.f;
				char buff[64];
				sprintf_s(buff, "##%s%u", label, id);
				ImGui::Columns(2);
				ImGui::Text("%s", label);
				ImGui::NextColumn();
				if (ImGui::Checkbox(buff, &projectShadow))
					shadowMapIndex = projectShadow ? 0.f : -1.f;
				ImGui::Columns();
			};

		utilDragFloat("Ambient color", 0, &m_environmentData.AmbientColor[0], 3, true);
		uint32_t shadowIdLabel = 0;
		if (ImGui::CollapsingHeader("Directional light"))
		{
			utilShadowCheckbox("Project shadows", shadowIdLabel++, m_environmentData.DirectionalLight.ShadowMapIndex);
			utilDragFloat("Direction", EnvironmentData::MaxLights, &m_environmentData.DirectionalLight.Direction[0], 3, false, 0.02f, -1.f, 1.f);
			utilDragFloat("Color", EnvironmentData::MaxLights, &m_environmentData.DirectionalLight.Color[0], 3, true);
			debugrender::DrawLine3D(glm::vec3(0.f), m_environmentData.DirectionalLight.Direction, glm::vec3(1.f));
		}
		if (ImGui::CollapsingHeader("Point lights"))
		{
			int32_t activeLightsCount = (int32_t)m_environmentData.ActiveLightsCount;
			if (ImGui::SliderInt("Active point lights", &activeLightsCount, 0, (int32_t)EnvironmentData::MaxLights, "%d"))
				m_environmentData.ActiveLightsCount = (float)activeLightsCount;
			for (int32_t i = 0; i < activeLightsCount; ++i)
			{
				char buff[32];
				sprintf_s(buff, "PointLight_%u", i);
				if (ImGui::CollapsingHeader(buff))
				{
					utilDragFloat("Position", i, &m_environmentData.Lights[i].Position[0], 3, false);
					utilDragFloat("Color", i, &m_environmentData.Lights[i].Color[0], 3, true);
					utilDragFloat("Radius", i, &m_environmentData.Lights[i].Radius, 1, false, 0.5f, 0.f, FLT_MAX);
					utilDragFloat("Compression", i, &m_environmentData.Lights[i].Compression, 1, false, 0.5f, 0.5f, FLT_MAX);

					debugrender::DrawAxis(m_environmentData.Lights[i].Position, glm::vec3(0.f), glm::vec3(1.f));
					debugrender::DrawSphere(m_environmentData.Lights[i].Position, m_environmentData.Lights[i].Radius, m_environmentData.Lights[i].Color);
				}
			}
		}
		if (ImGui::CollapsingHeader("Spot lights"))
		{
			int32_t activeLightsCount = (int32_t)m_environmentData.ActiveSpotLightsCount;
			if (ImGui::SliderInt("Active spot lights", &activeLightsCount, 0, (int32_t)EnvironmentData::MaxLights, "%d"))
				m_environmentData.ActiveSpotLightsCount = (float)activeLightsCount;

			for (uint32_t i = 0; i < (uint32_t)activeLightsCount; ++i)
			{
				char buff[32];
				sprintf_s(buff, "SpotLight_%u", i);
				if (ImGui::CollapsingHeader(buff))
				{
					SpotLightData& data = m_environmentData.SpotLights[i];
					utilShadowCheckbox("Project shadows", shadowIdLabel++, data.ShadowMapIndex);
					utilDragFloat("Position", i, &data.Position[0], 3, false);
					glm::vec3 dir = glm::normalize(data.Direction);

					// dir vector to roll pitch yaw (x:pitch, y:yaw, z:roll)
					glm::vec3 pyr = math::ToRot(data.Direction);
					if (utilDragFloat("Direction", i, &pyr[0], 3, false, 0.02f, -(float)M_PI, (float)M_PI))
					{
						// Transform to rot matrix
						glm::mat4 m = math::PitchYawRollToMat4(pyr);
						// Forward direction (0, 0, 1) in rotation space.
						data.Direction = glm::vec3(m * glm::vec4(0.f, 0.f, 1.f, 1.f));
					}
					utilDragFloat("Color", i, &data.Color[0], 3, true);
					utilDragFloat("InnerCutoff", i, &data.InnerCutoff, 1, false, 0.01f, 0.f, FLT_MAX);
					utilDragFloat("OuterCutoff", i, &data.OuterCutoff, 1, false, 0.01f, 0.f, FLT_MAX);

					constexpr float scl = 1.f; // meters
					debugrender::DrawAxis(data.Position, pyr, glm::vec3(scl));
				}
			}
		}
		if (createWindow)
			ImGui::End();
	}

	void Scene::UpdateRenderData(const RenderContext& renderContext, UniformBuffer* buffer, const glm::vec3& viewPosition)
	{
		ProcessEnvironmentData(viewPosition);
		RecalculateTransforms();

		check(buffer->SetUniform(renderContext, UNIFORM_ID_SCENE_ENV_DATA, &m_environmentData, sizeof(EnvironmentData)));
		check(buffer->SetUniform(renderContext, UNIFORM_ID_SCENE_MODEL_TRANSFORM_ARRAY, GetRawGlobalTransforms(), GetRenderObjectCount() * sizeof(glm::mat4)));
	}

	const glm::mat4* Scene::GetRawGlobalTransforms() const
	{
		// Dirty check, must be clean
		for (uint32_t i = 0; i < MaxNodeLevel; ++i)
			check(m_dirtyNodes[i].empty());
		return m_globalTransforms.data();
	}

	void Scene::ProcessEnvironmentData(const glm::vec3& view)
	{
		m_environmentData.ViewPosition = view;
#if 0
		m_environmentData.ActiveLightsCount = 0.f;
		m_environmentData.ActiveSpotLightsCount = 0.f;
		for (auto& it : m_componentMap)
		{
			const RenderObjectComponents& comp = it.second;
			if (comp.LightIndex != UINT32_MAX)
			{
				check(it.first < GetRenderObjectCount());
				check(comp.LightIndex < GetLightCount());
				const Light& light = GetLightArray()[comp.LightIndex];
				const glm::mat4& transform = GetRawGlobalTransforms()[it.first];
				const glm::vec3 pos = math::GetPos(transform);
				const glm::vec3 dir = math::GetDir(transform);
				switch (light.Type)
				{
				case ELightType::Point:
					m_environmentData.Lights[(uint32_t)m_environmentData.ActiveLightsCount].Color = light.Color;
					m_environmentData.Lights[(uint32_t)m_environmentData.ActiveLightsCount].Compression = light.Compression;
					m_environmentData.Lights[(uint32_t)m_environmentData.ActiveLightsCount].Position = pos;
					m_environmentData.Lights[(uint32_t)m_environmentData.ActiveLightsCount].Radius = light.Radius;
					++m_environmentData.ActiveLightsCount;
					break;
				case ELightType::Directional:
					m_environmentData.DirectionalLight.Color = light.Color;
					m_environmentData.DirectionalLight.Direction = dir;
					break;
				case ELightType::Spot:
					m_environmentData.SpotLights[(uint32_t)m_environmentData.ActiveSpotLightsCount].Color = light.Color;
					m_environmentData.SpotLights[(uint32_t)m_environmentData.ActiveSpotLightsCount].Position = pos;
					m_environmentData.SpotLights[(uint32_t)m_environmentData.ActiveSpotLightsCount].Direction = dir;
					m_environmentData.SpotLights[(uint32_t)m_environmentData.ActiveSpotLightsCount].InnerCutoff = light.InnerCutoff;
					m_environmentData.SpotLights[(uint32_t)m_environmentData.ActiveSpotLightsCount].OuterCutoff = light.OuterCutoff;
					++m_environmentData.ActiveSpotLightsCount;
					break;
				}
			}
		}
#endif // 0

	}
	
}
