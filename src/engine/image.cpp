#include <engine/image.h>
#include<util/vkinit.h>
#include<engine/buffer.h>

//#define STB_IMAGE_IMPLEMENTATION
#include<tinygltf/stb_image.h>
// ImageBuilder& ImageBuilder::setExtent(uint32_t w, uint32_t h)
// {
//     m_width = w;
//     m_height = h;
//     return *this;
// }

// ImageBuilder& ImageBuilder::setImageType(VkImageType type)
// {
//     m_type = type;
//     return *this;
// }

// ImageBuilder& ImageBuilder::setLayerCount(uint32_t layerCount)
// {
//     m_layerCount = layerCount;
//     return *this;
// }

// ImageBuilder& ImageBuilder::setMipMapLevel(uint32_t mipmaplevel)
// {
//     m_mipmapLevel = mipmaplevel;
//     return *this;
// }

// ImageBuilder& ImageBuilder::setMemProperty(VkMemoryPropertyFlags memprop)
// {
//     m_memProperty = memprop;
//     return *this;
// }

// ImageBuilder& ImageBuilder::setFormat(VkFormat format)
// {
//     m_format = format;
//     return *this;
// }

// ImageBuilder& ImageBuilder::setUsage(VkImageUsageFlags usages)
// {
//     m_usage = usages;
//     return *this;
// }

// bool ImageBuilder::isComplete() const
// {
//     if(m_height == 0 || m_width == 0)
//     {
//         LOG_ERROR("ImageBuilder inComplete");
//         return false;
//     }
    
//     return true;
// }

// Image::Image(uint32_t w,uint32_t h, VkFormat format,uint32_t lc,uint32_t ml)
// :m_width(w),m_height(h),m_format(format),m_layerCount(lc),m_mipmapLevel(ml){};

Image::Image(const VkImageCreateInfo& createinfo,VkImageAspectFlags aspect,VkMemoryPropertyFlags property)
{
    init(createinfo,aspect,property);
    if(createinfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT)
    {
        createSampler();
        
        LOG_INFO("Image Need sampler");
    }

	m_currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	m_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    m_descriptor.imageView = m_view;
}

void Image::setDescriptorImageLayout(VkImageLayout layout)
{
	m_descriptor.imageLayout = layout;
}

Image::~Image()
{
    vkDestroyImageView(VulkanContext::get()->getDevice(),m_view,nullptr);
    vkDestroyImage(VulkanContext::get()->getDevice(),m_image,nullptr);
    vkFreeMemory(VulkanContext::get()->getDevice(),m_mem,nullptr);

    if(m_sampled) vkDestroySampler(VulkanContext::get()->getDevice(),m_sampler,nullptr);
}

uint32_t Image::getWidth()
{
    return m_createInfo.extent.width;
}

uint32_t Image::getHeight()
{
    return m_createInfo.extent.height;
}

void Image::init(const VkImageCreateInfo& createinfo,VkImageAspectFlags aspect,VkMemoryPropertyFlags property)
{
    m_createInfo = createinfo;

    VK_CHECK(vkCreateImage(VulkanContext::get()->getDevice(),&createinfo,nullptr,&m_image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(VulkanContext::get()->getDevice(), m_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vkinit::findMemoryType(memRequirements.memoryTypeBits, property);

    VK_CHECK(vkAllocateMemory(VulkanContext::get()->getDevice(),&allocInfo,nullptr,&m_mem))
    VK_CHECK(vkBindImageMemory(VulkanContext::get()->getDevice(),m_image,m_mem,0))

    VkImageViewCreateInfo viewCreateInfo{};

    VkImageSubresourceRange range{};
    VkImageView  imageView{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = m_image;
    viewCreateInfo.format = m_createInfo.format;
	if(createinfo.flags == VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    else viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    range.aspectMask = aspect;
    range.baseArrayLayer = 0;
    range.baseMipLevel = 0;
    range.layerCount = m_createInfo.arrayLayers;
    range.levelCount = m_createInfo.mipLevels;
    viewCreateInfo.subresourceRange = range;
    VK_CHECK(vkCreateImageView(VulkanContext::get()->getDevice(),&viewCreateInfo,nullptr,&m_view))
    
}

void Image::release()
{
    vkDestroyImageView(VulkanContext::get()->getDevice(),m_view,nullptr);
    vkDestroyImage(VulkanContext::get()->getDevice(),m_image,nullptr);
    vkFreeMemory(VulkanContext::get()->getDevice(),m_mem,nullptr);
    //vkDestroySampler(VulkanContext::get()->getDevice(),m_sampler,nullptr);
}

void Image::createImage(VkImageCreateInfo createinfo)
{
    // VK_CHECK(vkCreateImage(VulkanContext::get()->getDevice(),
    // &createinfo,
    // nullptr,
    // &m_image));

    // VkMemoryRequirements memRequirements;
    // vkGetImageMemoryRequirements(VulkanContext::get()->getDevice(), m_image, &memRequirements);

    // VkMemoryAllocateInfo allocInfo{};
    // allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    // allocInfo.allocationSize = memRequirements.size;
    // allocInfo.memoryTypeIndex = vkinit::findMemoryType(memRequirements.memoryTypeBits, property);

    // VK_CHECK(vkAllocateMemory(VulkanContext::get()->getDevice(),&allocInfo,nullptr,&m_mem))
    // VK_CHECK(vkBindImageMemory(VulkanContext::get()->getDevice(),m_image,m_mem,0))

}

void Image::createImageView()
{
//     VkImageViewCreateInfo createInfo{};
//     VkImageSubresourceRange range{};
//     VkImageView  imageView{};
//     createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//     createInfo.image = m_image;
//     createInfo.format = m_format;
//     createInfo.viewType = viewType;
//     range.aspectMask = aspect;
//     range.baseArrayLayer = 0;
//     range.baseMipLevel = 0;
//     range.layerCount = m_layerCount;
//     range.levelCount = 1;
//     createInfo.subresourceRange = range;
//     VK_CHECK(vkCreateImageView(VulkanContext::get()->getDevice(),&createInfo,nullptr,&imageView))
// }
}

void Image::createSampler()
{
	m_sampled = true;

    VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.anisotropyEnable = VK_FALSE;
	createInfo.addressModeU =  VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV =  VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW =  VK_SAMPLER_ADDRESS_MODE_REPEAT;

	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	createInfo.unnormalizedCoordinates = VK_FALSE;
	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_NEVER;

	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = static_cast<float>(m_createInfo.mipLevels);

	VK_CHECK(vkCreateSampler(VulkanContext::get()->getDevice(), &createInfo, nullptr, &m_sampler));
	//return sampler;
    m_descriptor.sampler = m_sampler;
}

void Image::transitionImaglayout(VkImageLayout oldLayout,VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer{};

    vkinit::allocateCmdBuffer(commandBuffer,true);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = m_image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = m_createInfo.mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = m_createInfo.arrayLayers;
	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0;

	VkPipelineStageFlags sourceStage{};
	VkPipelineStageFlags destinationStage{};

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else {//if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	// else {
	// 	//throw std::invalid_argument("unsupported layout transition!");
	// 	std::cout << "error" << std::endl;
	// }

	
	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	vkinit::flushCmdBuffer(commandBuffer);

    m_descriptor.imageLayout = newLayout;

	m_currentLayout = newLayout;
}

void Image::transitionImaglayout(VkImageLayout oldLayout,VkImageLayout newLayout,VkCommandBuffer cmd)
{

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = m_image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = m_createInfo.mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = m_createInfo.arrayLayers;
	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0;

	VkPipelineStageFlags sourceStage{};
	VkPipelineStageFlags destinationStage{};

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else {//if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	// else {
	// 	//throw std::invalid_argument("unsupported layout transition!");
	// 	std::cout << "error" << std::endl;
	// }

	
	vkCmdPipelineBarrier(
		cmd,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

    m_descriptor.imageLayout = newLayout;
	m_currentLayout = newLayout;
}

void Image::generateMipmaps()
{
	transitionImaglayout(m_currentLayout,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkCommandBuffer commandBuffer {};
    vkinit::allocateCmdBuffer(commandBuffer,true);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_image;
	barrier.subresourceRange.layerCount = m_createInfo.arrayLayers;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;

	int32_t width = m_createInfo.extent.width;
	int32_t height = m_createInfo.extent.height;

	for(int i = 1;i<m_createInfo.mipLevels;i++)
	{
		barrier.subresourceRange.baseMipLevel = i-1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
		
		VkImageBlit blit{};
		blit.srcOffsets[0] = {0,0,0};
		blit.srcOffsets[1] = {width,height,1};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = m_createInfo.arrayLayers;
		blit.srcSubresource.mipLevel = i-1;

		blit.dstOffsets[0] = {0,0,0};
		blit.dstOffsets[1] = {width>1?width/2:1, height>1?height/2:1,1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = m_createInfo.arrayLayers;
		blit.dstSubresource.mipLevel = i;

		vkCmdBlitImage(commandBuffer,
		m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &blit,
		VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (width > 1) width /= 2;
    	if (height > 1) height /= 2;
	}

	barrier.subresourceRange.baseMipLevel = m_createInfo.mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	vkinit::flushCmdBuffer(commandBuffer);

	m_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

}

void Image::generateMipmaps(VkCommandBuffer cmd)
{
	transitionImaglayout(m_currentLayout,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,cmd);

	// VkCommandBuffer commandBuffer {};
    // vkinit::allocateCmdBuffer(commandBuffer,true);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_image;
	barrier.subresourceRange.layerCount = m_createInfo.arrayLayers;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;

	int32_t width = m_createInfo.extent.width;
	int32_t height = m_createInfo.extent.height;

	for(int i = 1;i<m_createInfo.mipLevels;i++)
	{
		barrier.subresourceRange.baseMipLevel = i-1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
		
		VkImageBlit blit{};
		blit.srcOffsets[0] = {0,0,0};
		blit.srcOffsets[1] = {width,height,1};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = m_createInfo.arrayLayers;
		blit.srcSubresource.mipLevel = i-1;

		blit.dstOffsets[0] = {0,0,0};
		blit.dstOffsets[1] = {width>1?width/2:1, height>1?height/2:1,1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = m_createInfo.arrayLayers;
		blit.dstSubresource.mipLevel = i;

		vkCmdBlitImage(cmd,
		m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &blit,
		VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (width > 1) width /= 2;
    	if (height > 1) height /= 2;
	}

	barrier.subresourceRange.baseMipLevel = m_createInfo.mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	//vkinit::flushCmdBuffer(commandBuffer);

	m_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void Image::writeToSet(VkDescriptorSet set, VkDescriptorType type, uint32_t binding)
{
    VkWriteDescriptorSet writer = vkinit::descriptorWriterInfo(set,m_descriptor,binding,type);
    vkUpdateDescriptorSets(VulkanContext::get()->getDevice(),1,&writer,0,nullptr);
}

Image::Image(const char* path,VkFormat format, bool mipmap)
{
	bool hdr = false;
	if(format == VK_FORMAT_R32G32B32A32_SFLOAT || format == VK_FORMAT_R16G16B16A16_SFLOAT) hdr = true;
	loadFromFile(path,format,hdr,mipmap);
}

void Image::loadFromFile(const char* path,VkFormat imageFormat,bool hdr,bool mipmap)
{
    void* tex = nullptr;
    int texWidth, texHeight, texChannels;
    

    uint32_t imageSize  = 0;
	if(hdr) 
	{
		tex = stbi_loadf(path,&texWidth,&texHeight,&texChannels,STBI_rgb_alpha);
		imageSize = static_cast<uint32_t>(texWidth * texHeight * 16);
	}
    else
	{
		tex = stbi_load(path,&texWidth,&texHeight,&texChannels,STBI_rgb_alpha);
		imageSize = static_cast<uint32_t>(texWidth * texHeight * 4);
	} 

    if (!tex) {
		LOG_ERROR("Failed to load texture file");
		//std::cout << "Failed to load texture file " << file << std::endl;
        return;
	}
    
    std::shared_ptr<Buffer>stagingBuffer = std::make_shared<Buffer>(imageSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    stagingBuffer->map(tex);
    stagingBuffer->unmap();

    stbi_image_free(tex);

    
	VkExtent3D imageExtent;
	imageExtent.width = static_cast<uint32_t>(texWidth);
	imageExtent.height = static_cast<uint32_t>(texHeight);
	imageExtent.depth = 1;

	uint32_t miplevels = mipmap?static_cast<uint32_t>(floor(log2(std::max(texWidth,texHeight))))+1:1;

    VkImageCreateInfo ci = vkinit::imageCreateInfo(imageFormat,VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT,imageExtent,
	1,VK_IMAGE_TYPE_2D,miplevels);

    m_createInfo = ci;

    init(ci,VK_IMAGE_ASPECT_COLOR_BIT);

	m_descriptor.imageView = m_view;

    transitionImaglayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer->m_buffer);
    transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    stagingBuffer.reset();

    createSampler();
}

void Image::copyBufferToImage(VkBuffer buffer)
{
    uint32_t width = getWidth();
    uint32_t height = getHeight();

    VkCommandBuffer commandBuffer {};
    vkinit::allocateCmdBuffer(commandBuffer,true);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		m_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	vkinit::flushCmdBuffer(commandBuffer);
}

void Image::copyBufferToImage(VkBuffer buffer, std::vector<VkBufferImageCopy>& regions)
{
	uint32_t width = getWidth();
    uint32_t height = getHeight();

    VkCommandBuffer commandBuffer {};
    vkinit::allocateCmdBuffer(commandBuffer,true);

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		m_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(regions.size()),
		regions.data()
	);

	vkinit::flushCmdBuffer(commandBuffer);
}

ktxResult Image::loadKTXFile(std::string filename, ktxTexture **target)
{
	ktxResult result = KTX_SUCCESS;

	result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);

	return result;	
}

void Image::loadKTXFromFile(std::string filename,VkFormat format, VkImageLayout imageLayout)
{
	ktxTexture* ktxTexture;
	ktxResult result = loadKTXFile(filename, &ktxTexture);
	assert(result == KTX_SUCCESS);

	uint32_t width = ktxTexture->baseWidth;
	uint32_t height = ktxTexture->baseHeight;
	uint32_t depth = ktxTexture->baseDepth;

	uint32_t mipLevels = ktxTexture->numLevels;

	std::cout<<"image Levels:"<<mipLevels<<std::endl;

	ktx_uint8_t *ktxTextureData = ktxTexture_GetData(ktxTexture);
	ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

	uint32_t texSize = static_cast<uint32_t>(ktxTextureSize);

	VkExtent3D imageExtent = {width, height, depth};

	VkImageCreateInfo ci = vkinit::imageCreateInfo(format,
	VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	imageExtent,1,
	VK_IMAGE_TYPE_2D,mipLevels);

	std::shared_ptr<Buffer>stagingBuffer = std::make_shared<Buffer>(texSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    stagingBuffer->map(ktxTextureData);
    stagingBuffer->unmap();

	m_createInfo = ci;

    init(ci,VK_IMAGE_ASPECT_COLOR_BIT);

	m_descriptor.imageView = m_view;

	std::vector<VkBufferImageCopy> bufferCopyRegions;

	for (uint32_t i = 0; i < mipLevels; i++)
	{
		ktx_size_t offset;
		KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
		assert(result == KTX_SUCCESS);

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = i;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = std::max(1u, ktxTexture->baseWidth >> i);
		bufferCopyRegion.imageExtent.height = std::max(1u, ktxTexture->baseHeight >> i);
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);
	}

    transitionImaglayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer->m_buffer,bufferCopyRegions);
    transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,imageLayout);

    stagingBuffer.reset();

    createSampler();
}