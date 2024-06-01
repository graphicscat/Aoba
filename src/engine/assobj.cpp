#include<engine/assobj.h>
#include<engine/buffer.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include<util/vkinit.h>

AssVertexInputDescription AssVertex::get_vertex_description()
{
    AssVertexInputDescription description;

	//we will have just 1 vertex buffer binding, with a per-vertex rate
	VkVertexInputBindingDescription mainBinding = {};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(AssVertex);
	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(mainBinding);

	//Position will be stored at Location 0
	VkVertexInputAttributeDescription positionAttribute = {};
	positionAttribute.binding = 0;
	positionAttribute.location = 0;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = offsetof(AssVertex, position);

	//Normal will be stored at Location 1
	VkVertexInputAttributeDescription normalAttribute = {};
	normalAttribute.binding = 0;
	normalAttribute.location = 1;
	normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalAttribute.offset = offsetof(AssVertex, normal);

	//Position will be stored at Location 2
	// VkVertexInputAttributeDescription colorAttribute = {};
	// colorAttribute.binding = 0;
	// colorAttribute.location = 2;
	// colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	// colorAttribute.offset = offsetof(Vertex, color);

	VkVertexInputAttributeDescription uvAttribute = {};
	uvAttribute.binding = 0;
	uvAttribute.location = 2;
	uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
	uvAttribute.offset = offsetof(AssVertex, uv);

	description.attributes.push_back(positionAttribute);
	description.attributes.push_back(normalAttribute);
	// description.attributes.push_back(colorAttribute);
	description.attributes.push_back(uvAttribute);

	return description;
}

void AssMesh::initBuffer()
{
    size_t vertexBufferSize = m_vertices.size()*sizeof(AssVertex);
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

void AssMesh::loadAssimpMesh(const aiMesh* mesh)
{
    bool hasNormal = mesh->HasNormals();
    bool hasTexCoord = mesh->HasTextureCoords(0);
    
    m_vertices.reserve(mesh->mNumVertices);
    for(int i = 0; i< mesh->mNumVertices;i++)
    {
        AssVertex vertex;
        aiVector3D pos = mesh->mVertices[i];
		vertex.position = glm::vec3(pos.x,pos.y,pos.z);
        if(hasNormal)
        {
            aiVector3D normal = mesh->mNormals[i];
            vertex.normal = glm::vec3(normal.x,normal.y,normal.z);
        }
        if(hasTexCoord)
        {
            aiVector3D coord = mesh->mTextureCoords[0][i];
            vertex.uv = glm::vec2(coord.x,coord.y);
        }
        m_vertices.push_back(vertex);
    }

    m_indices.reserve(3*mesh->mNumFaces);
	for (unsigned int i=0; i<mesh->mNumFaces; i++) {
		m_indices.push_back(mesh->mFaces[i].mIndices[0]);
		m_indices.push_back(mesh->mFaces[i].mIndices[1]);
		m_indices.push_back(mesh->mFaces[i].mIndices[2]);
	}
    
    initBuffer();
   
}

AssMesh:: ~AssMesh()
{

}

void AssScene::loadScene(const char* path)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices);

    if(scene)
    {
        for(int i = 0 ;i<scene->mNumMeshes;i++)
        {
            std::shared_ptr<AssObject>obj = std::make_shared<AssObject>();
            std::shared_ptr<AssMesh> mesh = std::make_shared<AssMesh>();
            mesh->loadAssimpMesh(scene->mMeshes[i]);
            obj->m_mesh = mesh;
            m_objects.push_back(obj);
        }
    }
}