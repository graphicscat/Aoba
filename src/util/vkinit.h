#pragma once

#include<vulkan/vulkan.h>
#include<pch.h>

namespace vkinit
{
    struct CameraUBO
    {
        public:
        alignas(64) glm::mat4 project;
        alignas(64) glm::mat4 view;
        alignas(16) glm::vec4 camPos;

        CameraUBO() = default;
        ~CameraUBO() = default;
    };

    struct GrassData
    {
        alignas(16) glm::vec3 pos;
        alignas(16) glm::vec2 uv;
    };

    inline VkImageViewCreateInfo imageview_begin_info(VkImage image,VkFormat format,VkImageAspectFlagBits aspect)
    {
        VkImageViewCreateInfo createInfo{};
        VkComponentMapping mapping{};
        VkImageSubresource range{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = aspect;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        return createInfo;
    }

    inline VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule)
    {
        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.pNext = nullptr;

        //shader stage
        info.stage = stage;
        //module containing the code for this shader stage
        info.module = shaderModule;
        //the entry point of the shader
        info.pName = "main";
        return info;
    }

    inline VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo() {
        VkPipelineVertexInputStateCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        info.pNext = nullptr;

        //no vertex bindings or attributes
        info.vertexBindingDescriptionCount = 0;
        info.vertexAttributeDescriptionCount = 0;
        return info;
    }

    inline VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo(VkPrimitiveTopology topology) {
        VkPipelineInputAssemblyStateCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        info.pNext = nullptr;

        info.topology = topology;
        //we are not going to use primitive restart on the entire tutorial so leave it on false
        info.primitiveRestartEnable = VK_FALSE;
        return info;
    }

    inline VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(VkPolygonMode polygonMode,VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE)
    {
        VkPipelineRasterizationStateCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        info.pNext = nullptr;

        info.depthClampEnable = VK_FALSE;
        //rasterizer discard allows objects with holes, default to no
        info.rasterizerDiscardEnable = VK_FALSE;

        info.polygonMode = polygonMode;
        info.lineWidth = 1.0f;
        //no backface cull
        info.cullMode = VK_CULL_MODE_NONE;
        info.frontFace = frontFace;
        //no depth bias
        info.depthBiasEnable = VK_FALSE;
        info.depthBiasConstantFactor = 0.0f;
        info.depthBiasClamp = 0.0f;
        info.depthBiasSlopeFactor = 0.0f;

        return info;
    }

    inline VkPipelineMultisampleStateCreateInfo multisamplingStateCreateInfo()
    {
        VkPipelineMultisampleStateCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        info.pNext = nullptr;

        info.sampleShadingEnable = VK_FALSE;
        //multisampling defaulted to no multisampling (1 sample per pixel)
        info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        info.minSampleShading = 1.0f;
        info.pSampleMask = nullptr;
        info.alphaToCoverageEnable = VK_FALSE;
        info.alphaToOneEnable = VK_FALSE;
        return info;
    }

    inline VkPipelineColorBlendAttachmentState colorBlendAttachmentState() {
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        return colorBlendAttachment;
    }

    inline VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp)
    {
        VkPipelineDepthStencilStateCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        info.pNext = nullptr;

        info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
        info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
        info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
        info.depthBoundsTestEnable = VK_FALSE;
        info.minDepthBounds = 0.0f; // Optional
        info.maxDepthBounds = 1.0f; // Optional
        info.stencilTestEnable = VK_FALSE;

        return info;
    }

    inline VkFramebufferCreateInfo framebufferCreateInfo(VkRenderPass renderPass, VkExtent2D extent)
    {
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.pNext = nullptr;

        info.renderPass = renderPass;
        info.attachmentCount = 1;
        info.width = extent.width;
        info.height = extent.height;
        info.layers = 1;

        return info;
    }

    inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count /*= 1*/, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
    {
        VkCommandBufferAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext = nullptr;

        info.commandPool = pool;
        info.commandBufferCount = count;
        info.level = level;
        return info;
    }

    inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags /*= 0*/)
    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;

        info.flags = flags;

        return info;
    }

    inline VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0)
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = flags;
        return info;
    }

    inline VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0)
    {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.pNext = nullptr;

        //info.pInheritanceInfo = nullptr;
        info.flags = flags;
        return info;
    }

    inline VkDescriptorSetLayoutBinding descriptorsetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding)
    {
        VkDescriptorSetLayoutBinding setbind = {};
        setbind.binding = binding;
        setbind.descriptorCount = 1;
        setbind.descriptorType = type;
        setbind.pImmutableSamplers = nullptr;
        setbind.stageFlags = stageFlags;

        return setbind;
    }

    inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>&bindings)
    {
        VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo {};
        descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayoutInfo.flags = 0;
        descriptorLayoutInfo.pBindings = bindings.data();
        descriptorLayoutInfo.bindingCount = uint32_t(bindings.size());
        return descriptorLayoutInfo;
    }

    inline VkImageCreateInfo imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent,uint32_t arrayLayers = 1,VkImageType imageType = VK_IMAGE_TYPE_2D,uint32_t miplevel = 1)
    {
        VkImageCreateInfo info = { };
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = nullptr;

        info.imageType = imageType;

        info.format = format;
        info.extent = extent;

        info.mipLevels = miplevel;
        info.arrayLayers = arrayLayers;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = usageFlags;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        return info;
    }

    inline VkAttachmentDescription createAttachmentDescription(VkFormat format,bool depth,
    VkSampleCountFlagBits sampleCounts = VK_SAMPLE_COUNT_1_BIT)
    {
        VkAttachmentDescription des{};
        des.flags = 0;
        des.format = format;
        des.samples = sampleCounts;
        des.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        des.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        des.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if(depth)
        {
            des.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            des.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        }
        else
        {
            des.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            des.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        des.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        return des;
    }

    inline VkWriteDescriptorSet descriptorWriterInfo(VkDescriptorSet set, VkDescriptorImageInfo descriptor,uint32_t binding)
    {
        VkWriteDescriptorSet writer{};
        writer.descriptorCount = 1;
        writer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writer.dstBinding = binding;
        writer.dstSet = set;
        writer.pImageInfo = &descriptor;
        writer.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        return writer;
    }

    inline VkWriteDescriptorSet descriptorWriterInfo(VkDescriptorSet set, VkDescriptorImageInfo descriptor,uint32_t binding,VkDescriptorType type)
    {
        VkWriteDescriptorSet writer{};
        writer.descriptorCount = 1;
        writer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writer.dstBinding = binding;
        writer.dstSet = set;
        writer.pImageInfo = &descriptor;
        writer.descriptorType = type;
        return writer;
    }

    inline VkWriteDescriptorSet descriptorWriterInfo(VkDescriptorSet set, VkDescriptorBufferInfo descriptor,uint32_t binding,VkDescriptorType type)
    {
        VkWriteDescriptorSet writer{};
        writer.descriptorCount = 1;
        writer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writer.dstBinding = binding;
        writer.dstSet = set;
        writer.pBufferInfo = &descriptor;
        writer.descriptorType = type;
        return writer;
    }


    inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
			VkColorComponentFlags colorWriteMask,
			VkBool32 blendEnable)
    {
        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState {};
        pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
        pipelineColorBlendAttachmentState.blendEnable = blendEnable;
        return pipelineColorBlendAttachmentState;
    }

    inline VkVertexInputBindingDescription vertexInputBindingDescription(
			uint32_t binding,
			uint32_t stride,
			VkVertexInputRate inputRate)
		{
			VkVertexInputBindingDescription vInputBindDescription {};
			vInputBindDescription.binding = binding;
			vInputBindDescription.stride = stride;
			vInputBindDescription.inputRate = inputRate;
			return vInputBindDescription;
		}

    inline VkVertexInputAttributeDescription vertexInputAttributeDescription(
			uint32_t binding,
			uint32_t location,
			VkFormat format,
			uint32_t offset)
		{
			VkVertexInputAttributeDescription vInputAttribDescription {};
			vInputAttribDescription.location = location;
			vInputAttribDescription.binding = binding;
			vInputAttribDescription.format = format;
			vInputAttribDescription.offset = offset;
			return vInputAttribDescription;
		}

        inline int findMemoryType(int typeFilter,VkMemoryPropertyFlags properties)
        {
            VkPhysicalDeviceMemoryProperties memProperties{};
            vkGetPhysicalDeviceMemoryProperties(VulkanContext::get()->getGPU(),&memProperties);

        for(uint32_t i = 0; i < memProperties.memoryTypeCount;i++)
        {
            if(typeFilter & ( 1 << i ) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i ;
        }

            return 0;
        }


    inline bool loadShaderModule(const char* filePath , VkShaderModule* outShaderModule)
    {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            std::cout<<"cant find file";
            return false;
        }

        //find what the size of the file is by looking up the location of the cursor
        //because the cursor is at the end, it gives the size directly in bytes
        size_t fileSize = (size_t)file.tellg();

        //spirv expects the buffer to be on uint32, so make sure to reserve a int vector big enough for the entire file
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

        //put file cursor at beggining
        file.seekg(0);

        //load the entire file into the buffer
        file.read((char*)buffer.data(), fileSize);

        //now that the file is loaded into the buffer, we can close it
        file.close();

        //create a new shader module, using the buffer we loaded
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;

        //codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
        createInfo.codeSize = buffer.size() * sizeof(uint32_t);
        createInfo.pCode = buffer.data();

        //check that the creation goes well.
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(VulkanContext::get()->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            return false;
            }
        *outShaderModule = shaderModule;
        return true;
    }

    inline void allocateCmdBuffer(VkCommandBuffer& cmd,bool begin = false)
    {
        auto* ctx = VulkanContext::get();
        VkCommandBufferAllocateInfo info = commandBufferAllocateInfo(ctx->getCommandPool(),1);
        VK_CHECK(vkAllocateCommandBuffers(ctx->getDevice(),&info,&cmd));

        if(begin)
        {
            VkCommandBufferBeginInfo commandBufferBI{};
			commandBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			VK_CHECK(vkBeginCommandBuffer(cmd, &commandBufferBI));
        }
    }

    inline void flushCmdBuffer(VkCommandBuffer cmd)
    {
        auto* ctx = VulkanContext::get();
        vkEndCommandBuffer(cmd);
        VkSubmitInfo submit{};
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        vkQueueSubmit(ctx->getQueue(),1,&submit,VK_NULL_HANDLE);
        vkDeviceWaitIdle(ctx->getDevice());
        vkFreeCommandBuffers(ctx->getDevice(),ctx->getCommandPool(),1,&cmd);
    }

    inline VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo(uint32_t patchControlPoints)
    {
        VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo {};
        pipelineTessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        pipelineTessellationStateCreateInfo.patchControlPoints = patchControlPoints;
        return pipelineTessellationStateCreateInfo;
    }

    inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo()
    {
        VkGraphicsPipelineCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        return ci;
    }
}