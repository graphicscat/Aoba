#include <engine/objmesh.h>
#include <tinyobjloader/tiny_obj_loader.h>
#include <iostream>
#include<util/vkinit.h>

VertexInputDescription Vertex::get_vertex_description()
{
	VertexInputDescription description;

	//we will have just 1 vertex buffer binding, with a per-vertex rate
	VkVertexInputBindingDescription mainBinding = {};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(Vertex);
	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(mainBinding);

	//Position will be stored at Location 0
	VkVertexInputAttributeDescription positionAttribute = {};
	positionAttribute.binding = 0;
	positionAttribute.location = 0;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = offsetof(Vertex, position);

	//Normal will be stored at Location 1
	VkVertexInputAttributeDescription normalAttribute = {};
	normalAttribute.binding = 0;
	normalAttribute.location = 1;
	normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalAttribute.offset = offsetof(Vertex, normal);

	//Position will be stored at Location 2
	VkVertexInputAttributeDescription colorAttribute = {};
	colorAttribute.binding = 0;
	colorAttribute.location = 2;
	colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	colorAttribute.offset = offsetof(Vertex, color);

	VkVertexInputAttributeDescription uvAttribute = {};
	uvAttribute.binding = 0;
	uvAttribute.location = 3;
	uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
	uvAttribute.offset = offsetof(Vertex, uv);

	description.attributes.push_back(positionAttribute);
	description.attributes.push_back(normalAttribute);
	description.attributes.push_back(colorAttribute);
	description.attributes.push_back(uvAttribute);

	// VkPipelineVertexInputStateCreateInfo ci = vkinit::vertexInputStateCreateInfo();
	// ci.vertexAttributeDescriptionCount = description.attributes.size();
	// ci.pVertexAttributeDescriptions = description.attributes.data();
	// ci.vertexBindingDescriptionCount = description.bindings.size();
	// ci.pVertexBindingDescriptions = description.bindings.data();

	return description;
}

bool Mesh::load_from_obj(const char* filename)
{
	//attrib will contain the vertex arrays of the file
	tinyobj::attrib_t attrib;
	//shapes contains the info for each separate object in the file
	std::vector<tinyobj::shape_t> shapes;
	//materials contains the information about the material of each shape, but we wont use it.
	std::vector<tinyobj::material_t> materials;

	//error and warning output from the load function
	std::string warn;
	std::string err;

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	//load the OBJ file
	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename,
		nullptr);
	//make sure to output the warnings to the console, in case there are issues with the file
	if (!warn.empty()) {
		std::cout << "WARN: " << warn << std::endl;
	}
	//if we have any error, print it to the console, and break the mesh loading. 
	//This happens if the file cant be found or is malformed
	if (!err.empty()) {
		std::cerr << err << std::endl;
		return false;
	}

	  for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
				bounding_box.MaxPoint = vkinit::MaxVec(vertex.position,bounding_box.MaxPoint);
				bounding_box.MinPoint = vkinit::MinVec(vertex.position,bounding_box.MinPoint);

                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                    m_vertices.push_back(vertex);
                }

                m_indices.push_back(uniqueVertices[vertex]);
            }
        }

    init();

	return true;
}

void Mesh::release()
{
	// vkFreeMemory(VulkanContext::get()->getDevice(),m_vertexBuffer->m_mem,nullptr);
	// vkDestroyBuffer(VulkanContext::get()->getDevice(),m_vertexBuffer->m_buffer,nullptr);

	// vkFreeMemory(VulkanContext::get()->getDevice(),m_indexBuffer->m_mem,nullptr);
	// vkDestroyBuffer(VulkanContext::get()->getDevice(),m_indexBuffer->m_buffer,nullptr);
}

void Mesh::init()
{
    size_t vertexBufferSize = m_vertices.size()*sizeof(Vertex);
    size_t indexBufferSize = m_indices.size()*sizeof(uint32_t);

    std::unique_ptr<Buffer> stagingVertBuffer = std::make_unique<Buffer>(vertexBufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    std::unique_ptr<Buffer> stagingIndexBuffer = std::make_unique<Buffer>(indexBufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    stagingVertBuffer->map(m_vertices.data());
    stagingVertBuffer->unmap();

    stagingIndexBuffer->map(m_indices.data());
    stagingIndexBuffer->unmap();

    m_vertexBuffer = std::make_shared<Buffer>(vertexBufferSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_indexBuffer = std::make_shared<Buffer>(indexBufferSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer cmd{};
    vkinit::allocateCmdBuffer(cmd,true);
    VkBufferCopy bufferCopy{};
    bufferCopy.size = vertexBufferSize;
    bufferCopy.dstOffset = 0;
    bufferCopy.srcOffset = 0;
    vkCmdCopyBuffer(cmd,stagingVertBuffer->m_buffer,m_vertexBuffer->m_buffer,1,&bufferCopy);

    bufferCopy.size = indexBufferSize;

    vkCmdCopyBuffer(cmd,stagingIndexBuffer->m_buffer,m_indexBuffer->m_buffer,1,&bufferCopy);

    vkinit::flushCmdBuffer(cmd);

    stagingVertBuffer.reset();
    stagingIndexBuffer.reset();

}

Mesh::~Mesh()
{
    release();
}

Mesh::Mesh(const char* path)
{
	float f = (float)0xfffff;
	bounding_box.MaxPoint = glm::vec3(-f);
	bounding_box.MinPoint = glm::vec3(f);
	load_from_obj(path);

	initBoundingBoxData();
	m_pc.model = glm::mat4(1.0);
}

void Mesh::initBoundingBoxData()
{
	glm::vec3 x = (bounding_box.MaxPoint.x - bounding_box.MinPoint.x)*glm::vec3(1.0f,0.0f,0.0f);
	glm::vec3 z = (bounding_box.MaxPoint.z - bounding_box.MinPoint.z)*glm::vec3(0.0f,0.0f,1.0f);

	glm::vec3 p1 = bounding_box.MaxPoint;

	//
	glm::vec3 p2 = p1 - z;
	glm::vec3 p3 = p2 - x;
	glm::vec3 p4 = p3 + z;
	glm::vec3 p7 = bounding_box.MinPoint;
	glm::vec3 p8 = p7 + z;glm::vec3 p6 = p7+x;glm::vec3 p5 = p6+z;

	m_bounding_box_vertices.push_back(p1);
	m_bounding_box_vertices.push_back(p2);
	m_bounding_box_vertices.push_back(p3);
	m_bounding_box_vertices.push_back(p4);

	m_bounding_box_vertices.push_back(p5);
	m_bounding_box_vertices.push_back(p6);
	m_bounding_box_vertices.push_back(p7);
	m_bounding_box_vertices.push_back(p8);

	m_bounding_box_indices = {0,1,1,2,2,3,3,0,
							  0,4,1,5,2,6,3,7,
							  4,5,5,6,6,7,7,4};
	
	size_t vertexBufferSize = m_bounding_box_vertices.size()*sizeof(glm::vec3);
    size_t indexBufferSize = m_bounding_box_indices.size()*sizeof(uint32_t);

    std::unique_ptr<Buffer> stagingVertBuffer = std::make_unique<Buffer>(vertexBufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    std::unique_ptr<Buffer> stagingIndexBuffer = std::make_unique<Buffer>(indexBufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    stagingVertBuffer->map(m_bounding_box_vertices.data());
    stagingVertBuffer->unmap();

    stagingIndexBuffer->map(m_bounding_box_indices.data());
    stagingIndexBuffer->unmap();

    m_bounding_box_vertexBuffer = std::make_shared<Buffer>(vertexBufferSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_bounding_box_indexBuffer = std::make_shared<Buffer>(indexBufferSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer cmd{};
    vkinit::allocateCmdBuffer(cmd,true);
    VkBufferCopy bufferCopy{};
    bufferCopy.size = vertexBufferSize;
    bufferCopy.dstOffset = 0;
    bufferCopy.srcOffset = 0;
    vkCmdCopyBuffer(cmd,stagingVertBuffer->m_buffer,m_bounding_box_vertexBuffer->m_buffer,1,&bufferCopy);

    bufferCopy.size = indexBufferSize;

    vkCmdCopyBuffer(cmd,stagingIndexBuffer->m_buffer,m_bounding_box_indexBuffer->m_buffer,1,&bufferCopy);

    vkinit::flushCmdBuffer(cmd);

    stagingVertBuffer.reset();
    stagingIndexBuffer.reset();


}