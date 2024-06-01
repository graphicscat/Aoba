#include <assimp/scene.h>
#include<pch.h>

struct AssVertexInputDescription {
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;
};

struct AssVertex {

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;

	static AssVertexInputDescription get_vertex_description();
}; 

class Buffer;
class AssMesh {
public:
	AssMesh() = default;
	~AssMesh();

	void loadAssimpMesh(const aiMesh* mesh);
	void initBuffer();

public:
	std::vector<AssVertex> m_vertices;
	std::vector<uint32_t> m_indices;
	std::shared_ptr<Buffer> m_vertexBuffer;
	std::shared_ptr<Buffer> m_indexBuffer;
};

class AssMaterial
{
	public:
	AssMaterial() = default;
	~AssMaterial() = default;
};

class AssObject
{
	public:
	AssObject() = default;
	~AssObject() = default;

	std::shared_ptr<AssMesh> m_mesh;
	std::shared_ptr<AssMaterial> m_material;
};

class AssScene
{
	public:
	AssScene() = default;
	~AssScene() = default;

	void loadScene(const char* path);

	std::vector<std::shared_ptr<AssObject>> m_objects;

	glm::mat4 m_model{1.0};
};