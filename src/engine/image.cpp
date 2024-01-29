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

    m_descriptor.imageView = m_view;
    
    
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
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    range.aspectMask = aspect;
    range.baseArrayLayer = 0;
    range.baseMipLevel = 0;
    range.layerCount = m_createInfo.arrayLayers;
    range.levelCount = 1;
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
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = 0.0f;

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
	barrier.subresourceRange.levelCount = 1;
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
}

void Image::writeToSet(VkDescriptorSet set, VkDescriptorType type, uint32_t binding)
{
    VkWriteDescriptorSet writer = vkinit::descriptorWriterInfo(set,m_descriptor,binding,type);
    vkUpdateDescriptorSets(VulkanContext::get()->getDevice(),1,&writer,0,nullptr);
}

void Image::loadFromFile(const char* path,VkFormat imageFormat,bool hdr)
{
    void* tex = nullptr;
    int texWidth, texHeight, texChannels;
    tex = stbi_load(path,&texWidth,&texHeight,&texChannels,STBI_rgb_alpha);

    uint32_t imageSize  = 0;
    imageSize = static_cast<uint32_t>(texWidth * texHeight * 4);

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

    VkImageCreateInfo ci = vkinit::imageCreateInfo(imageFormat,VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,imageExtent);

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