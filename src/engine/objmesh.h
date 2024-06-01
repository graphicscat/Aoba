#pragma once
#include<pch.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include<engine/buffer.h>

struct VertexInputDescription {
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};


struct Vertex {

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;
	static VertexInputDescription get_vertex_description();
	

	bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal && uv == other.uv;
    }
}; 

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.uv) << 1);
        }
    };
}
struct AABB
{
	AABB() = default;
	~AABB() = default;

	glm::vec3 MaxPoint;
	glm::vec3 MinPoint;
};

struct Mesh {
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	std::shared_ptr<Buffer> m_vertexBuffer;
	std::shared_ptr<Buffer> m_indexBuffer;

	struct PushConst
	{
		glm::vec4 objectColor = glm::vec4(0.0f);
		glm::mat4 model = glm::mat4(1.0);
	}m_pc;
	

	AABB bounding_box;

	bool load_from_obj(const char* filename);

	void release();

    void init();

    Mesh() = default;
    ~Mesh();

	Mesh(const char* );

	std::vector<glm::vec3> m_bounding_box_vertices;
	std::vector<uint32_t> m_bounding_box_indices;

	std::shared_ptr<Buffer> m_bounding_box_vertexBuffer;
	std::shared_ptr<Buffer> m_bounding_box_indexBuffer;

	void initBoundingBoxData();

};