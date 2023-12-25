
#include "Mesh.h"
#include "Debug.h"

namespace vkmmc
{
	void RenderResource::SetHandle(RenderHandle handle)
	{
		check(!m_pipelineHandle.IsValid());
		m_pipelineHandle = handle;
	}

	void Mesh::SetVertices(const Vertex* data, size_t count)
	{
		check(!GetHandle().IsValid());
		m_vertices.clear();
		m_vertices.resize(count);
		memcpy_s(m_vertices.data(), sizeof(Vertex) * count, data, sizeof(Vertex) * count);
	}

	void Mesh::SetIndices(const uint32_t* data, size_t count)
	{
		check(!GetHandle().IsValid());
		m_indices.clear();
		m_indices.resize(count);
		memcpy_s(m_indices.data(), sizeof(uint32_t) * count, data, sizeof(uint32_t) * count);
	}

}