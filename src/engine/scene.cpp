#include<engine/scene.h>
#include<engine/camera.h>

// void Scene::init()
// {
//     // m_renderPass.getPresentRenderPass();

//     // m_framebuffer.init(m_renderPass);

//     // m_descriptor.update();

//     // m_graphicsPipeline = std::make_unique<GraphicsPipeline>("../../shaders/test.vert.spv","../../shaders/test.frag.spv",m_descriptor.getLayout(),m_renderPass.m_renderPass);
//     // m_graphicsPipeline->create();

//     // initSyn();

//     // initMainCmdBuffer();

// }

Scene::Scene()
{
    initSyn();

    initMainCmdBuffer();

    m_renderPass = std::make_shared<RenderPass>();
    m_renderPass->getPresentRenderPass();

    //m_framebuffer.init(m_renderPass);

    m_framebuffer = std::make_unique<Framebuffer>(m_renderPass);
}

Scene::~Scene()
{
    vkDestroySemaphore(VulkanContext::get()->getDevice(),m_renderSemaphore,nullptr);
    vkDestroySemaphore(VulkanContext::get()->getDevice(),m_presentSemaphore,nullptr);

    m_framebuffer.reset();

    m_renderPass.reset();

    LOG_WARN("Scene delete");
}

void Scene::initSyn()
{
    VkFenceCreateInfo fenceCreateInfo = vkinit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	// m_fences.resize(2);
	// for(int i = 0; i < 2; i++)
	// {
	// 	VK_CHECK(vkCreateFence(VulkanContext::get()->getDevice(), &fenceCreateInfo, nullptr, &m_fences[i]));
	// }

    VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphoreCreateInfo();

    // m_presentSemaphores.resize(1);
	// m_renderSemaphores.resize(1);

	VK_CHECK(vkCreateSemaphore(VulkanContext::get()->getDevice(), &semaphoreCreateInfo, nullptr, &m_presentSemaphore));
    VK_CHECK(vkCreateSemaphore(VulkanContext::get()->getDevice(), &semaphoreCreateInfo, nullptr, &m_renderSemaphore));
    LOG_TRACE("Init SyncStruct Success");
}

void Scene::initMainCmdBuffer()
{
    m_mainCmdBuffers.resize(2);

    for(int i = 0;i<m_mainCmdBuffers.size();i++)
    {
        vkinit::allocateCmdBuffer(m_mainCmdBuffers[i]);
    }
}

void ShaderToy::init()
{
    // m_renderPass = std::make_shared<RenderPass>();
    // m_renderPass->getPresentRenderPass();

    // //m_framebuffer.init(m_renderPass);

    // m_framebuffer = std::make_unique<Framebuffer>(m_renderPass);

    m_descriptor = std::make_shared<Descriptor>();
    m_descriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT,0);
    m_descriptor->update();

    m_graphicsPipeline = std::make_unique<GraphicsPipeline>("../../shaders/shaderToy/shadertoy.vert.spv","../../shaders/shaderToy/shadertoy.frag.spv",m_descriptor->getLayout(),m_renderPass->m_renderPass,sizeof(PushConstantData));
    m_dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    m_graphicsPipeline->setDynamicState(m_dynamicStates);
    //m_graphicsPipeline->bindPushConstant(sizeof(PushConstantData));

    m_graphicsPipeline->create();

    float w = VulkanContext::get()->getWidth();
    float h = VulkanContext::get()->getHeight();

    m_inputUniform = std::make_shared<ShaderToyInputData>();

    m_inputUniform->iDate = glm::vec4(2024.0f, 1.0f, 9.0f, 0.0f);
    m_inputUniform->iFrame = m_frameNumber;
    m_inputUniform->iResolution = glm::vec3(w,h,1.0f);
    m_inputUniform->iFrameRate = 0;
    m_inputUniform->iMouse = glm::vec4(0.0f);
    m_inputUniform->iTime = 0;
    m_inputUniform->iTimeDelta = 0;

    m_inputUniformBuffer = std::make_shared<Buffer>(sizeof(ShaderToyInputData));
    m_inputUniformBuffer->writeToSet(m_descriptor->getSet(),0);

    m_pcd.width = w;
    m_pcd.height = h;

    buildCmd();
    
}

void ShaderToy::update(float delta)
{
    m_inputUniform->iTime += delta;
    m_inputUniform->iTimeDelta = delta;
    
    double xpos, ypos;
    static double last_click_x = 0, last_click_y = 0;
    glfwGetCursorPos(VulkanContext::get()->getWindow(), &xpos, &ypos);
    static bool btn1_down;
    auto btn1 = glfwGetMouseButton(VulkanContext::get()->getWindow(), GLFW_MOUSE_BUTTON_1);
    m_inputUniform->iMouse.z = 0;
    m_inputUniform->iMouse.w = 0;
    if (!btn1_down && btn1 == GLFW_PRESS) {
        btn1_down = true;
    } else if (btn1_down && btn1 == GLFW_RELEASE) {
        btn1_down = false;
        last_click_x = xpos;
        last_click_y = ypos;
        m_inputUniform->iMouse.w = 1;
    }
    if (btn1_down) {
        m_inputUniform->iMouse.x = xpos;
        m_inputUniform->iMouse.y = ypos;
        m_inputUniform->iMouse.z = 1;
    }

    m_inputUniformBuffer->map(m_inputUniform.get());
}

void ShaderToy::buildCmd()
{
    for(size_t currentFrame = 0;currentFrame<MAX_IN_FLIGHT;currentFrame++)
    {
        VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        uint32_t w = VulkanContext::get()->getWidth();
        uint32_t h = VulkanContext::get()->getHeight();

        VkClearValue clearValues[2];
        VkViewport viewport{};
        VkRect2D scissor{};
        viewport = vkinit::viewport((float)w, (float)h, 0.0f, 1.0f);
        scissor = vkinit::rect2D(w, h, 0, 0);
        //float flash = std::abs(std::sin(_frameNumber / 120.f));
        clearValues[0].color = { { 0.75f, 0.85f, 1.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f , 0};
        
        VK_CHECK(vkBeginCommandBuffer(m_mainCmdBuffers[currentFrame], &cmdBeginInfo));
        vkCmdSetViewport(m_mainCmdBuffers[currentFrame],0,1,&viewport);
        vkCmdSetScissor(m_mainCmdBuffers[currentFrame],0,1,&scissor);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass->m_renderPass;
        renderPassInfo.framebuffer = m_framebuffer->m_framebuffers[currentFrame];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {w,h};
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;
        vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        VkDeviceSize offset = 0;

        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->getPipeline());
        vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_graphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_graphicsPipeline->getLayout(),0,1,&m_descriptor->getSet(),0,nullptr);

        vkCmdDraw(m_mainCmdBuffers[currentFrame],3,1,0,0);

        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));
    }
}

void ShaderToy::windowResize()
{
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
    VulkanContext::get()->getSwapchain().release();
    m_framebuffer.reset();

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(VulkanContext::get()->getWindow(), &width, &height);
    VulkanContext::get()->getSwapchain().init(width,height);

    m_framebuffer = std::make_unique<Framebuffer>(m_renderPass);

    m_inputUniform->iResolution = glm::vec3(width,height,1.0f);

    buildCmd();
}

void ShaderToy::tick()
{
    // uint32_t currentFrame = m_frameNumber % 2;
    uint32_t currentFrame = 0;
	// VK_CHECK(vkWaitForFences(VulkanContext::get()->getDevice(),1,&m_fences[currentFrame],VK_TRUE,UINT64_MAX));
	// VK_CHECK(vkResetFences(VulkanContext::get()->getDevice(),1,&m_fences[currentFrame]));
    auto result2 = vkAcquireNextImageKHR(VulkanContext::get()->getDevice(),VulkanContext::get()->getSwapchain().m_swapchain,UINT64_MAX,m_presentSemaphore,VK_NULL_HANDLE,&currentFrame);

    if ((result2 == VK_ERROR_OUT_OF_DATE_KHR) || (result2 == VK_SUBOPTIMAL_KHR)) {
		
		if(result2 == VK_ERROR_OUT_OF_DATE_KHR)
        {
            windowResize();
            LOG_TRACE("SwapChain Reset");
        }
	}
	else {
		VK_CHECK(result2);
	}

	// vkResetCommandBuffer(m_mainCmdBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &m_presentSemaphore;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_renderSemaphore;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pImageIndices = &currentFrame;
    present.swapchainCount = 1;
    present.pSwapchains = &VulkanContext::get()->getSwapchain().m_swapchain;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &m_renderSemaphore;
    auto result = vkQueuePresentKHR(VulkanContext::get()->getQueue(),&present);
	//m_frameNumber ++;
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
		windowResize();
        LOG_TRACE("SwapChain Reset");
		
	}
	else {
		VK_CHECK(result);
	}
}

ShaderToy::~ShaderToy()
{
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
    m_graphicsPipeline.reset();
    m_descriptor.reset();
    //m_framebuffer.release();
    m_inputUniformBuffer.reset();
    m_inputUniform.reset();   
}

void Atmosphere::init()
{
    initDescriptors();
    
    initPipelines();

    initResources();

    precomputeTransmit();

    buildSkyViewCmd();

    buildCmd();

}

void Atmosphere::windowResize()
{

}

void Atmosphere::buildCmd()
{
    for(size_t currentFrame = 0;currentFrame<MAX_IN_FLIGHT;currentFrame++)
    {
        VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkClearValue clearValues[2];
        VkViewport viewport{};
        VkRect2D scissor{};
        //float flash = std::abs(std::sin(_frameNumber / 120.f));
        clearValues[0].color = { { 0.75f, 0.85f, 1.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f , 0};
        
        VK_CHECK(vkBeginCommandBuffer(m_mainCmdBuffers[currentFrame], &cmdBeginInfo));

        //barrier
        // VkImageMemoryBarrier imageMemoryBarrier = {};
        // imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        // // We won't be changing the layout of the image
        // imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        // imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        // imageMemoryBarrier.image = m_skyViewLUT->m_image;
        // imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        // imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        // imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        // imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        // imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        // vkCmdPipelineBarrier(
        //     m_mainCmdBuffers[currentFrame],
        //     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        //     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        //     0,
        //     0, nullptr,
        //     0, nullptr,
        //     1, &imageMemoryBarrier);

        //

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass->m_renderPass;
        renderPassInfo.framebuffer = m_framebuffer->m_framebuffers[currentFrame];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {VulkanContext::get()->getWidth(),VulkanContext::get()->getHeight()};
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;
        vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        VkDeviceSize offset = 0;

        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->getPipeline());
        //vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_graphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_graphicsPipeline->getLayout(),0,1,&m_presentDescriptor->getSet(),0,nullptr);

        vkCmdDraw(m_mainCmdBuffers[currentFrame],3,1,0,0);

        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));
    }
}

void Atmosphere::buildSkyViewCmd()
{
    uint32_t w = m_skyViewLUT->getWidth();
    uint32_t h = m_skyViewLUT->getHeight();

    vkResetCommandBuffer(m_skyViewCmd,0);
    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    
    VK_CHECK(vkBeginCommandBuffer(m_skyViewCmd, &cmdBeginInfo));
    vkCmdBindPipeline(m_skyViewCmd,VK_PIPELINE_BIND_POINT_COMPUTE,m_skyViewPipeline->getPipeline());
    vkCmdBindDescriptorSets(m_skyViewCmd,VK_PIPELINE_BIND_POINT_COMPUTE,
    m_skyViewPipeline->getLayout(),0,1,&m_skyViewDescriptor->getSet(),0,nullptr);

    vkCmdDispatch(m_skyViewCmd, w / 8, h / 8, 1);

    VK_CHECK(vkEndCommandBuffer(m_skyViewCmd));

    auto info = vkinit::semaphoreCreateInfo();
    VK_CHECK(vkCreateSemaphore(VulkanContext::get()->getDevice(),&info,nullptr,&m_skyViewComputeSemaphore));
    VK_CHECK(vkCreateSemaphore(VulkanContext::get()->getDevice(),&info,nullptr,&m_skyViewGraphicsSemaphore));

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_skyViewGraphicsSemaphore;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    vkQueueWaitIdle(VulkanContext::get()->getQueue());
}

void Atmosphere::tick()
{
    // uint32_t currentFrame = m_frameNumber % 2;

    // Wait for rendering finished
    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    uint32_t currentFrame = 0;
    VK_CHECK(vkAcquireNextImageKHR(VulkanContext::get()->getDevice(),VulkanContext::get()->getSwapchain().m_swapchain,UINT64_MAX,m_presentSemaphore,VK_NULL_HANDLE,&currentFrame))
    static bool firstDraw = true;

    VkSubmitInfo skyviewsubmit{};
    skyviewsubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    skyviewsubmit.commandBufferCount = 1;
    skyviewsubmit.pCommandBuffers = &m_skyViewCmd;
    skyviewsubmit.waitSemaphoreCount = 1;
    skyviewsubmit.pWaitSemaphores = &m_skyViewGraphicsSemaphore;
    skyviewsubmit.signalSemaphoreCount = 1;
    skyviewsubmit.pSignalSemaphores = &m_skyViewComputeSemaphore;
    skyviewsubmit.pWaitDstStageMask = &waitStageMask;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&skyviewsubmit,VK_NULL_HANDLE))

    VkSemaphore graphicsWaitSemaphores[] = { m_skyViewComputeSemaphore, m_presentSemaphore };
    VkSemaphore graphicsSignalSemaphores[] = { m_renderSemaphore, m_skyViewGraphicsSemaphore };
    VkPipelineStageFlags graphicsWaitStageMasks[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submit.waitSemaphoreCount = 2;
    submit.pWaitSemaphores = graphicsWaitSemaphores;
    submit.signalSemaphoreCount = 2;
    submit.pSignalSemaphores = graphicsSignalSemaphores;
    submit.pWaitDstStageMask = graphicsWaitStageMasks;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pImageIndices = &currentFrame;
    present.swapchainCount = 1;
    present.pSwapchains = &VulkanContext::get()->getSwapchain().m_swapchain;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &m_renderSemaphore;
    VK_CHECK(vkQueuePresentKHR(VulkanContext::get()->getQueue(),&present))
}

void Atmosphere::update(float delta)
{
    m_atmosphereParam.iTime += delta;
    m_atmosphereParamBuffer->map(&m_atmosphereParam);

}

void Atmosphere::precomputeTransmit()
{
    uint32_t currentFrame = 0;

    uint32_t w = m_storageImage->getWidth();
    uint32_t h = m_storageImage->getHeight();

    vkResetCommandBuffer(m_transmittanceCmd,0);
    VkCommandBufferBeginInfo compCmdBeginInfo = vkinit::commandBufferBeginInfo();
    VK_CHECK(vkBeginCommandBuffer(m_transmittanceCmd, &compCmdBeginInfo));
    vkCmdBindPipeline(m_transmittanceCmd,VK_PIPELINE_BIND_POINT_COMPUTE,m_computePipeline->getPipeline());
    vkCmdBindDescriptorSets(m_transmittanceCmd,VK_PIPELINE_BIND_POINT_COMPUTE,
    m_computePipeline->getLayout(),0,1,&m_transmitDescriptor->getSet(),0,nullptr);

    vkCmdDispatch(m_transmittanceCmd, w / 8, h / 8, 1);

    VK_CHECK(vkEndCommandBuffer(m_transmittanceCmd));

    VkSubmitInfo submitCP{};
    submitCP.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitCP.commandBufferCount = 1;
    submitCP.pCommandBuffers = &m_transmittanceCmd;
    submitCP.waitSemaphoreCount = 0;
    //submitCP.pWaitSemaphores = &m_presentSemaphores[currentFrame];
    submitCP.signalSemaphoreCount = 0;
    //submitCP.pSignalSemaphores = &m_renderSemaphores[currentFrame];
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submitCP,VK_NULL_HANDLE))
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
}

Atmosphere::~Atmosphere()
{
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
    m_graphicsPipeline.reset();
    m_computePipeline.reset();

    m_transmitDescriptor.reset();
    m_presentDescriptor.reset();

    m_storageImage.reset();

    release();
}

void Atmosphere::initPipelines()
{
    m_graphicsPipeline = std::make_unique<GraphicsPipeline>(
    "../../shaders/atmosphere/present.vert.spv",
    "../../shaders/atmosphere/present.frag.spv",
    m_presentDescriptor->getLayout(),
    m_renderPass->m_renderPass
    );

    m_graphicsPipeline->create();
    
    m_computePipeline = std::make_unique<ComputePipeline>(
        "../../shaders/atmosphere/transmit.comp.spv",
        m_transmitDescriptor->getLayout()
    );

    m_skyViewPipeline = std::make_unique<ComputePipeline>
    (
        "../../shaders/atmosphere/sky_view_lut.comp.spv",
        m_skyViewDescriptor->getLayout()
    );
}

void Atmosphere::initDescriptors()
{
    m_presentDescriptor = std::make_shared<Descriptor>();
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT,0);
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1);
    m_presentDescriptor->update();

    m_transmitDescriptor = std::make_shared<Descriptor>();
    m_transmitDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT,0);
    m_transmitDescriptor->bind(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,VK_SHADER_STAGE_COMPUTE_BIT,1);
    m_transmitDescriptor->update();

    //sky-view
    m_skyViewDescriptor = std::make_shared<Descriptor>();
    m_skyViewDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT,0);
    //read transmittance lut
    m_skyViewDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_COMPUTE_BIT,1);
    //write skyview lut
    m_skyViewDescriptor->bind(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,VK_SHADER_STAGE_COMPUTE_BIT,2);
    m_skyViewDescriptor->update();

}

void Atmosphere::initResources()
{
    vkinit::allocateCmdBuffer(m_skyViewCmd);
    vkinit::allocateCmdBuffer(m_transmittanceCmd);

    m_atmosphereParam.transSkyLUTRes = glm::vec4(256.0f,144.0f,256.0f,144.0f);
    m_atmosphereParam.rayleighScatteringBase = glm::vec3(5.802f, 13.558f, 33.1f);
    m_atmosphereParam.rayleighScaleHeight = 8.0f;
    m_atmosphereParam.groundRadiusMM = 6.360f;
    m_atmosphereParam.atmosphereRadiusMM = 6.460f;
    m_atmosphereParam.iTime = 0.0f;

    

    // uint32_t w = VulkanContext::get()->getWidth();
    // uint32_t h = VulkanContext::get()->getHeight();

    uint32_t w = m_atmosphereParam.transSkyLUTRes.x;
    uint32_t h = m_atmosphereParam.transSkyLUTRes.y;

    VkImageCreateInfo info{};
    info = vkinit::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT,VK_IMAGE_USAGE_STORAGE_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,{w,h,1});
    m_storageImage = std::make_shared<Image>(info,VK_IMAGE_ASPECT_COLOR_BIT);

    m_storageImage->transitionImaglayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_GENERAL);

    //m_storageImage->writeToSet(m_presentDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);

    m_storageImage->writeToSet(m_transmitDescriptor->getSet(),VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,1);

    //skyview read
    m_storageImage->writeToSet(m_skyViewDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);

    m_atmosphereParamBuffer = std::make_shared<Buffer>(sizeof(AtmosphereParam));
    m_atmosphereParamBuffer->writeToSet(m_transmitDescriptor->getSet(),0);
    m_atmosphereParamBuffer->writeToSet(m_skyViewDescriptor->getSet(),0);
    m_atmosphereParamBuffer->writeToSet(m_presentDescriptor->getSet(),0);

    m_atmosphereParamBuffer->map(&m_atmosphereParam);
    //m_atmosphereParamBuffer->unmap();

    w = m_atmosphereParam.transSkyLUTRes.z;
    h = m_atmosphereParam.transSkyLUTRes.w;

    info.extent.width = w;
    info.extent.height = h;

    m_skyViewLUT = std::make_shared<Image>(info,VK_IMAGE_ASPECT_COLOR_BIT);
    m_skyViewLUT->transitionImaglayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_GENERAL);
    m_skyViewLUT->writeToSet(m_skyViewDescriptor->getSet(),VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,2);

    m_skyViewLUT->writeToSet(m_presentDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);

}

void Atmosphere::release()
{
    vkDestroySemaphore(VulkanContext::get()->getDevice(),m_skyViewGraphicsSemaphore,nullptr);
    vkDestroySemaphore(VulkanContext::get()->getDevice(),m_skyViewComputeSemaphore,nullptr);
}

void OBJScene::init()
{
    initDescriptors();

    initPipelines();

    initResources();
    
    precomputeGrassPos();

    buildWindCmd();

    buildCmd();
}

void OBJScene::windowResize()
{

}

void OBJScene::initPipelines()
{
    m_GPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/obj/present.vert.spv",
        "../../shaders/obj/present.frag.spv",
        m_presentDescriptor->getLayout(),
        m_renderPass->m_renderPass
        );
    VertexInputDescription des = Vertex::get_vertex_description();
    des.bindings.push_back(vkinit::vertexInputBindingDescription(1 /*Instance*/, sizeof(vkinit::GrassData), VK_VERTEX_INPUT_RATE_INSTANCE));
    des.attributes.push_back(vkinit::vertexInputAttributeDescription(1, 4, VK_FORMAT_R32G32B32A32_SFLOAT, 0));
    des.attributes.push_back(vkinit::vertexInputAttributeDescription(1, 5, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4));
    VkPipelineVertexInputStateCreateInfo ci = vkinit::vertexInputStateCreateInfo();

    ci.vertexAttributeDescriptionCount = des.attributes.size();
	ci.pVertexAttributeDescriptions = des.attributes.data();
	ci.vertexBindingDescriptionCount = des.bindings.size();
	ci.pVertexBindingDescriptions = des.bindings.data();

    m_GPipeline->setVertexInputState(ci);
    m_GPipeline->create();

    //precompute grass pos
    m_grassposCPipeline = std::make_unique<ComputePipeline>(
        "../../shaders/obj/grasspos.comp.spv",
        m_grassposDescriptor->getLayout()
        );

    m_windCPipeline = std::make_unique<ComputePipeline>(
        "../../shaders/obj/windtex.comp.spv",
        m_windDescriptor->getLayout()
        );
}

void OBJScene::initDescriptors()
{
    m_presentDescriptor = std::make_shared<Descriptor>();

    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_VERTEX_BIT,1);
    m_presentDescriptor->update();

    //grass pos comp
    m_grassposDescriptor = std::make_shared<Descriptor>();
    m_grassposDescriptor->bind(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT,0);
    m_grassposDescriptor->update();

    //wind pass
    m_windDescriptor = std::make_shared<Descriptor>();
    m_windDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT,0);
    m_windDescriptor->bind(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,VK_SHADER_STAGE_COMPUTE_BIT,1);
    m_windDescriptor->update();


}

void OBJScene::initResources()
{
    vkinit::allocateCmdBuffer(m_windCmd);

    m_grass = std::make_shared<Mesh>();

    m_grass->load_from_obj("../../assets/grass_blade.obj");

    m_cam = VulkanContext::get()->getCamera();

    m_camUBO = std::make_shared<Buffer>(sizeof(vkinit::CameraUBO));
    m_camUBO->writeToSet(m_presentDescriptor->getSet(),0);

    m_camParam = std::make_shared<vkinit::CameraUBO>();

    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1280.f / 720.f, 0.1f, 200.0f);
	projection[1][1] *= -1;

    m_camParam->view = m_cam->GetViewMatrix();
    m_camParam->project = projection;
    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);

    m_camUBO->map(m_camParam.get());

    //grass pos buffer
    uint32_t grassposBufferSize = sizeof(vkinit::GrassData)*GRASS_COUNT;
    m_grassposBuffer = std::make_shared<Buffer>(grassposBufferSize,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_grassposBuffer->writeToSet(m_grassposDescriptor->getSet(),0,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    //wind tex 
    m_time = 0.0f;
    uint32_t windTimeBufferSize = sizeof(float);
    m_windTimeBuffer = std::make_shared<Buffer>(windTimeBufferSize);
    m_windTimeBuffer->writeToSet(m_windDescriptor->getSet(),0);
    m_windTimeBuffer->map(&m_time);

    VkImageCreateInfo windtexci = vkinit::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT,
    VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,{WIND_TEX_DIM,WIND_TEX_DIM,1});
    m_windTex = std::make_shared<Image>(windtexci,VK_IMAGE_ASPECT_COLOR_BIT);
    m_windTex->transitionImaglayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_GENERAL);
    m_windTex->writeToSet(m_windDescriptor->getSet(),VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,1);
    m_windTex->writeToSet(m_presentDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);

}

void OBJScene::buildCmd()
{
    for(size_t currentFrame = 0;currentFrame<MAX_IN_FLIGHT;currentFrame++)
    {
        VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkClearValue clearValues[2];
        VkViewport viewport{};
        VkRect2D scissor{};
        //float flash = std::abs(std::sin(_frameNumber / 120.f));
        clearValues[0].color = { { 0.75f, 0.85f, 1.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f , 0};
        
        VK_CHECK(vkBeginCommandBuffer(m_mainCmdBuffers[currentFrame], &cmdBeginInfo));

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass->m_renderPass;
        renderPassInfo.framebuffer = m_framebuffer->m_framebuffers[currentFrame];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {VulkanContext::get()->getWidth(),VulkanContext::get()->getHeight()};
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;
        vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        VkDeviceSize offset = 0;

        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GPipeline->getPipeline());
        //vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_GPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_GPipeline->getLayout(),0,1,&m_presentDescriptor->getSet(),0,nullptr);

       
        vkCmdBindVertexBuffers(m_mainCmdBuffers[currentFrame], 0, 1, &m_grass->m_vertexBuffer->m_buffer, &offset);
        vkCmdBindVertexBuffers(m_mainCmdBuffers[currentFrame], 1, 1, &m_grassposBuffer->m_buffer, &offset);

        vkCmdBindIndexBuffer(m_mainCmdBuffers[currentFrame], m_grass->m_indexBuffer->m_buffer, 0, VK_INDEX_TYPE_UINT32);

        //vkCmdDraw(m_mainCmdBuffers[currentFrame], mesh->_vertices.size(), 1, 0, 0);

        vkCmdDrawIndexed(m_mainCmdBuffers[currentFrame],uint32_t(m_grass->m_indices.size()),GRASS_COUNT,0,0,0);

        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));
    }
}

void OBJScene::tick()
{
    uint32_t currentFrame = 0;
	
    VK_CHECK(vkAcquireNextImageKHR(VulkanContext::get()->getDevice(),VulkanContext::get()->getSwapchain().m_swapchain,UINT64_MAX,m_presentSemaphore,VK_NULL_HANDLE,&currentFrame))

    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    VkSubmitInfo windsubmit{};
    windsubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    windsubmit.commandBufferCount = 1;
    windsubmit.pCommandBuffers = &m_windCmd;
    windsubmit.waitSemaphoreCount = 1;
    windsubmit.pWaitSemaphores = &m_windGraphicsSemaphore;
    windsubmit.signalSemaphoreCount = 1;
    windsubmit.pSignalSemaphores = &m_windComputeSemaphore;
    windsubmit.pWaitDstStageMask = &waitStageMask;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&windsubmit,VK_NULL_HANDLE))

    VkSemaphore graphicsWaitSemaphores[] = { m_windComputeSemaphore, m_presentSemaphore };
    VkSemaphore graphicsSignalSemaphores[] = { m_renderSemaphore, m_windGraphicsSemaphore };
    VkPipelineStageFlags graphicsWaitStageMasks[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submit.waitSemaphoreCount = 2;
    submit.pWaitSemaphores = graphicsWaitSemaphores;
    submit.signalSemaphoreCount = 2;
    submit.pSignalSemaphores = graphicsSignalSemaphores;
    submit.pWaitDstStageMask = graphicsWaitStageMasks;

    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pImageIndices = &currentFrame;
    present.swapchainCount = 1;
    present.pSwapchains = &VulkanContext::get()->getSwapchain().m_swapchain;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &m_renderSemaphore;
    VK_CHECK(vkQueuePresentKHR(VulkanContext::get()->getQueue(),&present))
}

void OBJScene::update(float delta) 
{
    m_camParam->view = m_cam->GetViewMatrix();

    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);

    m_camUBO->map(m_camParam.get());

    m_time += delta;
    m_windTimeBuffer->map(&m_time);
}

void OBJScene::precomputeGrassPos()
{
    size_t currentFrame = 0;
    vkResetCommandBuffer(m_mainCmdBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

    //compute 
    VkCommandBufferBeginInfo compCmdBeginInfo = vkinit::commandBufferBeginInfo();
    VK_CHECK(vkBeginCommandBuffer(m_mainCmdBuffers[currentFrame], &compCmdBeginInfo));
    vkCmdBindPipeline(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_COMPUTE,m_grassposCPipeline->getPipeline());
    vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_COMPUTE,
    m_grassposCPipeline->getLayout(),0,1,&m_grassposDescriptor->getSet(),0,nullptr);

    vkCmdDispatch(m_mainCmdBuffers[currentFrame], GRASS_COUNT / 256, 1, 1);

    VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));

    VkSubmitInfo submitCP{};
    submitCP.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitCP.commandBufferCount = 1;
    submitCP.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submitCP.waitSemaphoreCount = 0;
    //submitCP.pWaitSemaphores = &m_presentSemaphores[currentFrame];
    submitCP.signalSemaphoreCount = 0;
    //submitCP.pSignalSemaphores = &m_renderSemaphores[currentFrame];
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submitCP,VK_NULL_HANDLE))
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
}

void OBJScene::buildWindCmd()
{
    vkResetCommandBuffer(m_windCmd,0);
    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    
    VK_CHECK(vkBeginCommandBuffer(m_windCmd, &cmdBeginInfo));
    vkCmdBindPipeline(m_windCmd,VK_PIPELINE_BIND_POINT_COMPUTE,m_windCPipeline->getPipeline());
    vkCmdBindDescriptorSets(m_windCmd,VK_PIPELINE_BIND_POINT_COMPUTE,
    m_windCPipeline->getLayout(),0,1,&m_windDescriptor->getSet(),0,nullptr);

    vkCmdDispatch(m_windCmd, WIND_TEX_DIM / 8, WIND_TEX_DIM / 8, 1);

    VK_CHECK(vkEndCommandBuffer(m_windCmd));

    auto info = vkinit::semaphoreCreateInfo();
    VK_CHECK(vkCreateSemaphore(VulkanContext::get()->getDevice(),&info,nullptr,&m_windComputeSemaphore));
    VK_CHECK(vkCreateSemaphore(VulkanContext::get()->getDevice(),&info,nullptr,&m_windGraphicsSemaphore));

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_windGraphicsSemaphore;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    vkQueueWaitIdle(VulkanContext::get()->getQueue());
}

OBJScene::~OBJScene()
{
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());

    m_GPipeline.reset();
    m_presentDescriptor.reset();
    m_camUBO.reset();
    m_camParam.reset();

    vkDestroySemaphore(VulkanContext::get()->getDevice(),m_windComputeSemaphore,nullptr);
    vkDestroySemaphore(VulkanContext::get()->getDevice(),m_windGraphicsSemaphore,nullptr);

}

void Terrain::init()
{
    initHeightMap();

    initPatchData();

    initDescriptors();

    initPipelines();

    initResources();

    initGrass();

    buildCmd();
}

void Terrain::windowResize()
{

}

void Terrain::initHeightMap()
{
    m_heightMap = std::make_shared<Image>();
    m_heightMap->loadFromFile("../../assets/terrain/hm2.png");

    m_terrainAlbedo = std::make_shared<Image>();
    m_terrainAlbedo->loadFromFile("../../assets/terrain/terraintex.png");

    m_terrainNormalMap = std::make_shared<Image>();
    m_terrainNormalMap->loadFromFile("../../assets/terrain/terrainnormal.png");
}

void Terrain::initPatchData()
{
    float scale = 0.3f;
    float w = static_cast<float>(m_heightMap->getWidth())*scale;
    float h = static_cast<float>(m_heightMap->getHeight())*scale;

    std::cout<<w<<std::endl;

    float leftTopCornerX = -w/2.0f;
    float leftTopCornerZ = -h/2.0f;

    m_terrain = std::make_shared<Mesh>();

    Vertex v;
    v.normal = glm::vec3(1.0);
    v.color = glm::vec3(1.0);
    for(int i = 0 ; i<=PATCH_SIZE; i++)
    {
        for(int j = 0 ; j<=PATCH_SIZE; j++)
        {
           v.position = glm::vec3(leftTopCornerX,0.0f,leftTopCornerZ) + glm::vec3(w*i/(float)PATCH_SIZE, 0.0f,h*j/(float)PATCH_SIZE);
           v.uv = glm::vec2((float)i/(float)PATCH_SIZE,(float)j/(float)PATCH_SIZE);
           m_terrain->m_vertices.push_back(v);
        }
    }

    uint32_t strip = static_cast<uint32_t>(PATCH_SIZE + 1);
    for(int i = 0 ; i<PATCH_SIZE; i++)
    {
        for(int j = 0 ; j<PATCH_SIZE; j++)
        {
          m_terrain->m_indices.push_back(i*strip+j);
          m_terrain->m_indices.push_back((i+1)*strip + j);
          m_terrain->m_indices.push_back((i+1)*strip + j+1);
          m_terrain->m_indices.push_back(i*strip + j+1);
          
        }
    }

    m_terrain->init();

}

void Terrain::initDescriptors()
{
    m_presentDescriptor = std::make_shared<Descriptor>();
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,0);
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,1);
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2);
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,3);
    m_presentDescriptor->update();
}

void Terrain::initPipelines()
{
    m_presentGPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/terrain/present.vert.spv",
        "../../shaders/terrain/present.frag.spv",
        m_presentDescriptor->getLayout(),
        m_renderPass->m_renderPass
    );

    VertexInputDescription des = Vertex::get_vertex_description();

    VkPipelineVertexInputStateCreateInfo ci = vkinit::vertexInputStateCreateInfo();

    ci.vertexAttributeDescriptionCount = des.attributes.size();
	ci.pVertexAttributeDescriptions = des.attributes.data();
	ci.vertexBindingDescriptionCount = des.bindings.size();
	ci.pVertexBindingDescriptions = des.bindings.data();

    m_presentGPipeline->setVertexInputState(ci);

    VkPipelineInputAssemblyStateCreateInfo iaci = vkinit::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
    m_presentGPipeline->setInputAssemblyState(iaci);

    VkPipelineRasterizationStateCreateInfo rci = vkinit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
    m_presentGPipeline->setRasterizationState(rci);

    m_presentGPipeline->setTessellationShaders("../../shaders/terrain/terrain.tesc.spv","../../shaders/terrain/terrain.tese.spv");
    VkPipelineTessellationStateCreateInfo tci = vkinit::pipelineTessellationStateCreateInfo(4);
    m_presentGPipeline->setTessellationState(tci);

    m_presentGPipeline->create();

    
}

void Terrain::buildCmd()
{
    for(size_t currentFrame = 0;currentFrame<MAX_IN_FLIGHT;currentFrame++)
    {
        VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkClearValue clearValues[2];
        VkViewport viewport{};
        VkRect2D scissor{};
        //float flash = std::abs(std::sin(_frameNumber / 120.f));
        clearValues[0].color = { { 0.75f, 0.85f, 1.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f , 0};
        
        VK_CHECK(vkBeginCommandBuffer(m_mainCmdBuffers[currentFrame], &cmdBeginInfo));

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass->m_renderPass;
        renderPassInfo.framebuffer = m_framebuffer->m_framebuffers[currentFrame];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {VulkanContext::get()->getWidth(),VulkanContext::get()->getHeight()};
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;
        vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        VkDeviceSize offset = 0;

        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_presentGPipeline->getPipeline());
        //vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_GPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_presentGPipeline->getLayout(),0,1,&m_presentDescriptor->getSet(),0,nullptr);

       
        vkCmdBindVertexBuffers(m_mainCmdBuffers[currentFrame], 0, 1, &m_terrain->m_vertexBuffer->m_buffer, &offset);
    
        vkCmdBindIndexBuffer(m_mainCmdBuffers[currentFrame], m_terrain->m_indexBuffer->m_buffer, 0, VK_INDEX_TYPE_UINT32);

        //vkCmdDraw(m_mainCmdBuffers[currentFrame], mesh->_vertices.size(), 1, 0, 0);

        vkCmdDrawIndexed(m_mainCmdBuffers[currentFrame],uint32_t(m_terrain->m_indices.size()),1,0,0,0);

        //draw grass
        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_grassGPipeline->getPipeline());
        
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_grassGPipeline->getLayout(),0,1,&m_grassDescriptor->getSet(),0,nullptr);

       
        vkCmdBindVertexBuffers(m_mainCmdBuffers[currentFrame], 0, 1, &m_grass->m_vertexBuffer->m_buffer, &offset);
        vkCmdBindVertexBuffers(m_mainCmdBuffers[currentFrame], 1, 1, &m_grassposBuffer->m_buffer, &offset);

        vkCmdBindIndexBuffer(m_mainCmdBuffers[currentFrame], m_grass->m_indexBuffer->m_buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(m_mainCmdBuffers[currentFrame],uint32_t(m_grass->m_indices.size()),GRASS_COUNT,0,0,0);

        //-----------------------------------------------------------------------------------------------------

        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));
    }
}

void Terrain::tick()
{
    uint32_t currentFrame = 0;
    VK_CHECK(vkAcquireNextImageKHR(VulkanContext::get()->getDevice(),VulkanContext::get()->getSwapchain().m_swapchain,UINT64_MAX,m_presentSemaphore,VK_NULL_HANDLE,&currentFrame))

    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    VkSubmitInfo windsubmit{};
    windsubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    windsubmit.commandBufferCount = 1;
    windsubmit.pCommandBuffers = &m_windCmd;
    windsubmit.waitSemaphoreCount = 1;
    windsubmit.pWaitSemaphores = &m_windGraphicsSemaphore;
    windsubmit.signalSemaphoreCount = 1;
    windsubmit.pSignalSemaphores = &m_windComputeSemaphore;
    windsubmit.pWaitDstStageMask = &waitStageMask;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&windsubmit,VK_NULL_HANDLE))

    VkSemaphore graphicsWaitSemaphores[] = { m_windComputeSemaphore, m_presentSemaphore };
    VkSemaphore graphicsSignalSemaphores[] = { m_renderSemaphore, m_windGraphicsSemaphore };
    VkPipelineStageFlags graphicsWaitStageMasks[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submit.waitSemaphoreCount = 2;
    submit.pWaitSemaphores = graphicsWaitSemaphores;
    submit.signalSemaphoreCount = 2;
    submit.pSignalSemaphores = graphicsSignalSemaphores;
    submit.pWaitDstStageMask = graphicsWaitStageMasks;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pImageIndices = &currentFrame;
    present.swapchainCount = 1;
    present.pSwapchains = &VulkanContext::get()->getSwapchain().m_swapchain;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &m_renderSemaphore;
    VK_CHECK(vkQueuePresentKHR(VulkanContext::get()->getQueue(),&present))
}

void Terrain::initResources()
{
    m_cam = VulkanContext::get()->getCamera();

    m_camUBO = std::make_shared<Buffer>(sizeof(vkinit::CameraUBO));
    m_camUBO->writeToSet(m_presentDescriptor->getSet(),0);

    m_camParam = std::make_shared<vkinit::CameraUBO>();

    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1280.f / 720.f, 0.1f, 2000.0f);
	projection[1][1] *= -1;

    m_camParam->view = m_cam->GetViewMatrix();
    m_camParam->project = projection;
    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);

    m_camUBO->map(m_camParam.get());

    m_heightMap->writeToSet(m_presentDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);
    m_terrainAlbedo->writeToSet(m_presentDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2);
    m_terrainNormalMap->writeToSet(m_presentDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3);

}

void Terrain::update(float delta) 
{
    m_camParam->view = m_cam->GetViewMatrix();

    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);

    m_camUBO->map(m_camParam.get());

    m_time += delta;
    m_windTimeBuffer->map(&m_time);

}

Terrain::~Terrain()
{
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
    vkDestroySemaphore(VulkanContext::get()->getDevice(),m_windComputeSemaphore,nullptr);
    vkDestroySemaphore(VulkanContext::get()->getDevice(),m_windGraphicsSemaphore,nullptr);
}

void Terrain::precomputeGrassPos()
{
    size_t currentFrame = 0;
    vkResetCommandBuffer(m_mainCmdBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

    //compute 
    VkCommandBufferBeginInfo compCmdBeginInfo = vkinit::commandBufferBeginInfo();
    VK_CHECK(vkBeginCommandBuffer(m_mainCmdBuffers[currentFrame], &compCmdBeginInfo));
    vkCmdBindPipeline(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_COMPUTE,m_grassposCPipeline->getPipeline());
    vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_COMPUTE,
    m_grassposCPipeline->getLayout(),0,1,&m_grassposDescriptor->getSet(),0,nullptr);

    vkCmdDispatch(m_mainCmdBuffers[currentFrame], GRASS_COUNT / 256, 1, 1);

    VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));

    VkSubmitInfo submitCP{};
    submitCP.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitCP.commandBufferCount = 1;
    submitCP.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submitCP.waitSemaphoreCount = 0;
    //submitCP.pWaitSemaphores = &m_presentSemaphores[currentFrame];
    submitCP.signalSemaphoreCount = 0;
    //submitCP.pSignalSemaphores = &m_renderSemaphores[currentFrame];
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submitCP,VK_NULL_HANDLE))
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
}

void Terrain::initGrass()
{
    initGrassDescriptors();

    initGrassPipelines();

    initGrassResources();

    precomputeGrassPos();

    buildWindCmd();

}
void Terrain::initGrassDescriptors()
{
    m_grassDescriptor = std::make_shared<Descriptor>();

    m_grassDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
    m_grassDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_VERTEX_BIT,1);
    m_grassDescriptor->update();

    m_grassposDescriptor = std::make_shared<Descriptor>();
    m_grassposDescriptor->bind(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT,0);
    m_grassposDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_COMPUTE_BIT,1);
    m_grassposDescriptor->update();

    //wind
    m_windDescriptor = std::make_shared<Descriptor>();
    m_windDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT,0);
    m_windDescriptor->bind(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,VK_SHADER_STAGE_COMPUTE_BIT,1);
    m_windDescriptor->update();
}

void Terrain::initGrassPipelines()
{
    m_grassGPipeline = std::make_unique<GraphicsPipeline>(
    "../../shaders/terrain/grass/present.vert.spv",
    "../../shaders/terrain/grass/present.frag.spv",
    m_grassDescriptor->getLayout(),
    m_renderPass->m_renderPass
    );
    VertexInputDescription des = Vertex::get_vertex_description();
    des.bindings.push_back(vkinit::vertexInputBindingDescription(1 /*Instance*/, sizeof(vkinit::GrassData), VK_VERTEX_INPUT_RATE_INSTANCE));
    des.attributes.push_back(vkinit::vertexInputAttributeDescription(1, 4, VK_FORMAT_R32G32B32A32_SFLOAT, 0));
    des.attributes.push_back(vkinit::vertexInputAttributeDescription(1, 5, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4));
    VkPipelineVertexInputStateCreateInfo ci = vkinit::vertexInputStateCreateInfo();

    ci.vertexAttributeDescriptionCount = des.attributes.size();
	ci.pVertexAttributeDescriptions = des.attributes.data();
	ci.vertexBindingDescriptionCount = des.bindings.size();
	ci.pVertexBindingDescriptions = des.bindings.data();

    m_grassGPipeline->setVertexInputState(ci);
    m_grassGPipeline->create();

    m_grassposCPipeline = std::make_unique<ComputePipeline>(
    "../../shaders/terrain/grass/grasspos.comp.spv",
    m_grassposDescriptor->getLayout()
    );

    m_windCPipeline = std::make_unique<ComputePipeline>(
    "../../shaders/terrain/grass/windtex.comp.spv",
    m_windDescriptor->getLayout()
    );

}

void Terrain::initGrassResources()
{
    vkinit::allocateCmdBuffer(m_windCmd);

    auto info = vkinit::semaphoreCreateInfo();
    VK_CHECK(vkCreateSemaphore(VulkanContext::get()->getDevice(),&info,nullptr,&m_windComputeSemaphore));
    VK_CHECK(vkCreateSemaphore(VulkanContext::get()->getDevice(),&info,nullptr,&m_windGraphicsSemaphore));

    m_grass = std::make_shared<Mesh>();
    m_grass->load_from_obj("../../assets/grass_blade.obj");

    //grass pos buffer
    uint32_t grassposBufferSize = sizeof(vkinit::GrassData)*GRASS_COUNT;
    m_grassposBuffer = std::make_shared<Buffer>(grassposBufferSize,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_grassposBuffer->writeToSet(m_grassposDescriptor->getSet(),0,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    m_heightMap->writeToSet(m_grassposDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);

    m_time = 0.0f;
    uint32_t windTimeBufferSize = sizeof(float);
    m_windTimeBuffer = std::make_shared<Buffer>(windTimeBufferSize);
    m_windTimeBuffer->writeToSet(m_windDescriptor->getSet(),0);
    m_windTimeBuffer->map(&m_time);

    //grass cam
    m_camUBO->writeToSet(m_grassDescriptor->getSet(),0);

    VkImageCreateInfo windtexci = vkinit::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT,
    VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,{WIND_TEX_DIM,WIND_TEX_DIM,1});
    m_windTex = std::make_shared<Image>(windtexci,VK_IMAGE_ASPECT_COLOR_BIT);
    m_windTex->transitionImaglayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_GENERAL);
    m_windTex->writeToSet(m_windDescriptor->getSet(),VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,1);
    m_windTex->writeToSet(m_grassDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);

}

void Terrain::buildWindCmd()
{
    vkResetCommandBuffer(m_windCmd,0);
    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    
    VK_CHECK(vkBeginCommandBuffer(m_windCmd, &cmdBeginInfo));
    vkCmdBindPipeline(m_windCmd,VK_PIPELINE_BIND_POINT_COMPUTE,m_windCPipeline->getPipeline());
    vkCmdBindDescriptorSets(m_windCmd,VK_PIPELINE_BIND_POINT_COMPUTE,
    m_windCPipeline->getLayout(),0,1,&m_windDescriptor->getSet(),0,nullptr);

    vkCmdDispatch(m_windCmd, WIND_TEX_DIM / 8, WIND_TEX_DIM / 8, 1);

    VK_CHECK(vkEndCommandBuffer(m_windCmd));


    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_windGraphicsSemaphore;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    vkQueueWaitIdle(VulkanContext::get()->getQueue());
}

void Deferred::initDeferredRenderPass()
{
    std::vector<VkFormat> formats = {
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_FORMAT_R8G8B8A8_SRGB
        };

    m_deferredRenderPass = std::make_shared<RenderPass>(formats);

    m_deferredFramebuffer = std::make_shared<Framebuffer>(m_deferredRenderPass);

    std::vector<VkFormat> filterFormats = {VK_FORMAT_R32G32B32A32_SFLOAT};
    m_filterRenderPass = std::make_shared<RenderPass>(filterFormats,false);
    m_filterFramebuffer = std::make_shared<Framebuffer>(m_filterRenderPass,512,512);
}

void Deferred::windowResize()
{
    
}
void Deferred::initDeferredDescriptor()
{
    m_deferredDescriptor = std::make_shared<Descriptor>();
    m_deferredDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
    m_deferredDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1);
    m_deferredDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2);//metal
    m_deferredDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,3);//roughness
    m_deferredDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,4);//normal
    m_deferredDescriptor->update();

    m_skyboxDescriptor = std::make_shared<Descriptor>();
    m_skyboxDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
    m_skyboxDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1);
    m_skyboxDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2);
    m_skyboxDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,3);
    m_skyboxDescriptor->update();

    m_diffuseFilterDescriptor = std::make_shared<Descriptor>();
    //m_hdrToCubeMapDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);;
    m_diffuseFilterDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,0);
    m_diffuseFilterDescriptor->update();

    m_head_material = m_deferredDescriptor->makeSet();
    m_body_material = m_deferredDescriptor->makeSet();
    m_base_material = m_deferredDescriptor->makeSet();
    m_plane_material = m_deferredDescriptor->makeSet();
    m_diffuseFilterSet = m_diffuseFilterDescriptor->makeSet();
    m_specularFilterSet = m_diffuseFilterDescriptor->makeSet();
}

void Deferred::initDeferredPipelines()
{
    m_deferredGPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/deferred/mrt.vert.spv",
        "../../shaders/deferred/mrt.frag.spv",
        m_deferredDescriptor->getLayout(),
        m_deferredRenderPass->m_renderPass,
        sizeof(PushConsts)
    );

    VertexInputDescription des = Vertex::get_vertex_description();
    VkPipelineVertexInputStateCreateInfo ci = vkinit::vertexInputStateCreateInfo();

    ci.vertexAttributeDescriptionCount = des.attributes.size();
	ci.pVertexAttributeDescriptions = des.attributes.data();
	ci.vertexBindingDescriptionCount = des.bindings.size();
	ci.pVertexBindingDescriptions = des.bindings.data();

    m_deferredGPipeline->setColorBlendState(3);
    m_deferredGPipeline->setVertexInputState(ci);
    m_deferredGPipeline->create();


    m_diffuseFilterGPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/deferred/env/diffusefilter.vert.spv",
        "../../shaders/deferred/env/diffusefilter.frag.spv",
        m_diffuseFilterDescriptor->getLayout(),
        m_filterRenderPass->m_renderPass,
        sizeof(glm::mat4)
    );
    m_dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    m_diffuseFilterGPipeline->setVertexInputState(ci);
    m_diffuseFilterGPipeline->setViewport({CUBEMAP_DIM,CUBEMAP_DIM});
    m_diffuseFilterGPipeline->setDepthStencilState(false,false);
    m_diffuseFilterGPipeline->setDynamicState(m_dynamicStates);
    m_diffuseFilterGPipeline->create();

    m_specularFilterGPipeline = std::make_unique<GraphicsPipeline>(
         "../../shaders/deferred/env/specularfilter.vert.spv",
        "../../shaders/deferred/env/specularfilter.frag.spv",
        m_diffuseFilterDescriptor->getLayout(),
        m_filterRenderPass->m_renderPass,
        sizeof(PushConsts)
    );

    m_specularFilterGPipeline->setVertexInputState(ci);
    m_specularFilterGPipeline->setViewport({SPECULAR_DIM,SPECULAR_DIM});
    m_specularFilterGPipeline->setDepthStencilState(false,false);
    m_specularFilterGPipeline->setDynamicState(m_dynamicStates);
    m_specularFilterGPipeline->create();

    m_skyboxGPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/deferred/skybox.vert.spv",
        "../../shaders/deferred/skybox.frag.spv",
        m_skyboxDescriptor->getLayout(),
        m_renderPass->m_renderPass
    );

    m_skyboxGPipeline->setDepthStencilState(false,false);
    m_skyboxGPipeline->setVertexInputState(ci);
    m_skyboxGPipeline->create();

    m_brdfLUTCPipeline = std::make_unique<ComputePipeline>(
        "../../shaders/deferred/env/brdflut.comp.spv",
        m_brdfLUTDescriptor->getLayout()
    );

}

void Deferred::initDescriptors()
{
    m_presentDescriptor = std::make_shared<Descriptor>();
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,0);//pos
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1);//normal
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2);//albedo
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT,3);
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,4);//skybox
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,5);//deferred depth
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,6);//deferred depth
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,7);//spec
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,8);//diffuse
    m_presentDescriptor->update();

    initDeferredDescriptor();

    m_brdfLUTDescriptor = std::make_shared<Descriptor>();
    m_brdfLUTDescriptor->bind(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,VK_SHADER_STAGE_COMPUTE_BIT,0);
    m_brdfLUTDescriptor->update();

}

void Deferred::initPipelines()
{
    m_presentGPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/deferred/present.vert.spv",
        "../../shaders/deferred/present.frag.spv",
        m_presentDescriptor->getLayout(),
        m_renderPass->m_renderPass
    );

    m_presentGPipeline->create();

    initDeferredPipelines();

}

void Deferred::initResources()
{
    vkinit::allocateCmdBuffer(m_diffuseFilterCmd);
    vkinit::allocateCmdBuffer(m_brdfLUTCmd);

    m_cam = VulkanContext::get()->getCamera();

    m_camUBO = std::make_shared<Buffer>(sizeof(vkinit::CameraUBO));
    m_camUBO->writeToSet(m_deferredDescriptor->getSet(),0);

    m_camUBO->writeToSet(m_head_material,0);
    m_camUBO->writeToSet(m_body_material,0);
    m_camUBO->writeToSet(m_base_material,0);
    m_camUBO->writeToSet(m_plane_material,0);
    m_camUBO->writeToSet(m_skyboxDescriptor->getSet(),0);
    m_camUBO->writeToSet(m_presentDescriptor->getSet(),3);

    m_camParam = std::make_shared<vkinit::CameraUBO>();

    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1280.f / 720.f, 0.1f, 200.0f);
	projection[1][1] *= -1;

    m_camParam->view = m_cam->GetViewMatrix();
    m_camParam->project = projection;
    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);

    m_camUBO->map(m_camParam.get());

    for(int i = 0 ;i<m_deferredFramebuffer->m_colorAttachments.size();i++)
    {
        m_deferredFramebuffer->m_colorAttachments[i]->writeToSet(m_presentDescriptor->getSet(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i);
    }
    m_deferredFramebuffer->m_depth->writeToSet(m_presentDescriptor->getSet(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5);

    m_mesh = std::make_shared<Mesh>();
    m_mesh->load_from_obj("../../assets/toyman/head.obj");

    m_toyman_head = std::make_shared<Mesh>("../../assets/toyman/head.obj");
    m_toyman_body = std::make_shared<Mesh>("../../assets/toyman/body.obj");
    m_toyman_base = std::make_shared<Mesh>("../../assets/toyman/base.obj");
    m_plane = std::make_shared<Mesh>("../../assets/toyman/plane.obj");
    m_skybox = std::make_shared<Mesh>("../../assets/cube.obj");

    m_head_albedo = std::make_shared<Image>("../../assets/toyman/01_Head_Base_Color.png",VK_FORMAT_R8G8B8A8_SRGB);
    m_body_albedo = std::make_shared<Image>("../../assets/toyman/02_Body_Base_Color.png",VK_FORMAT_R8G8B8A8_SRGB);
    m_base_albedo = std::make_shared<Image>("../../assets/toyman/03_Base_Base_Color.png",VK_FORMAT_R8G8B8A8_SRGB);

    m_head_roughness = std::make_shared<Image>("../../assets/toyman/01_Head_Roughness.png",VK_FORMAT_R8G8B8A8_UNORM);
    m_body_roughness = std::make_shared<Image>("../../assets/toyman/02_Body_Roughness.png",VK_FORMAT_R8G8B8A8_UNORM);
    m_base_roughness = std::make_shared<Image>("../../assets/toyman/03_Base_Roughness.png",VK_FORMAT_R8G8B8A8_UNORM);

    m_head_metallic = std::make_shared<Image>("../../assets/toyman/01_Head_Metallic.png",VK_FORMAT_R8G8B8A8_UNORM);
    m_body_metallic = std::make_shared<Image>("../../assets/toyman/02_Body_Metallic.png",VK_FORMAT_R8G8B8A8_UNORM);
    m_base_metallic = std::make_shared<Image>("../../assets/toyman/03_Base_Metallic.png",VK_FORMAT_R8G8B8A8_UNORM);

    m_head_normal = std::make_shared<Image>("../../assets/toyman/01_Head_Normal_DirectX.png",VK_FORMAT_R8G8B8A8_UNORM);
    m_body_normal = std::make_shared<Image>("../../assets/toyman/02_Body_Normal_DirectX.png",VK_FORMAT_R8G8B8A8_UNORM);
    m_base_normal = std::make_shared<Image>("../../assets/toyman/03_Base_Normal_DirectX.png",VK_FORMAT_R8G8B8A8_UNORM);

    m_empty = std::make_shared<Image>("../../assets/empty.png",VK_FORMAT_R8G8B8A8_SRGB);
    m_skybox_tex = std::make_shared<Image>("../../assets/hdr_cloudy.hdr",VK_FORMAT_R32G32B32A32_SFLOAT,true);
    m_skybox_tex->generateMipmaps();

    m_head_albedo->writeToSet(m_head_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);
    m_body_albedo->writeToSet(m_body_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);
    m_base_albedo->writeToSet(m_base_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);

    m_head_roughness->writeToSet(m_head_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2);
    m_body_roughness->writeToSet(m_body_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2);
    m_base_roughness->writeToSet(m_base_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2);

    m_head_metallic->writeToSet(m_head_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3);
    m_body_metallic->writeToSet(m_body_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3);
    m_base_metallic->writeToSet(m_base_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3);

    m_head_normal->writeToSet(m_head_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4);
    m_body_normal->writeToSet(m_body_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4);
    m_base_normal->writeToSet(m_base_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4);

    m_empty->writeToSet(m_plane_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);
    m_empty->writeToSet(m_plane_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2);
    m_empty->writeToSet(m_plane_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3);
    m_empty->writeToSet(m_plane_material,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4);

    m_skybox_tex->writeToSet(m_skyboxDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);
    m_skybox_tex->writeToSet(m_presentDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4);
    m_skybox_tex->writeToSet(m_diffuseFilterSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,0);
    m_skybox_tex->writeToSet(m_specularFilterSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,0);

    glm::mat4 model = glm::mat4(1.0);
    model = glm::scale(model,glm::vec3(20.0f,20.0f,20.0f));
    //model = glm::rotate(model,glm::radians(180.0f),glm::vec3(1,0,0));
    model = glm::rotate(model,glm::radians(-90.0f),glm::vec3(0,1,0));

    glm::mat4 planeModel = glm::mat4(1.0);
    planeModel = glm::scale(planeModel,glm::vec3(50.0f,50.0f,50.0f));

    std::shared_ptr<StaticObject> head = std::make_shared<StaticObject>();
    head->mesh = m_toyman_head;
    head->material = m_head_material;
    head->pc.model = model;
    head->pc.roughness = -1.0f;
    std::shared_ptr<StaticObject> body = std::make_shared<StaticObject>();
    body->mesh = m_toyman_body;
    body->material = m_body_material;
    body->pc.roughness = -1.0f;
    body->pc.model = model;
    std::shared_ptr<StaticObject> base = std::make_shared<StaticObject>();
    base->mesh = m_toyman_base;
    base->material = m_base_material;
    base->pc.roughness = -1.0f;
    base->pc.model = model;
    std::shared_ptr<StaticObject> plane = std::make_shared<StaticObject>();
    plane->mesh = m_plane;
    plane->material = m_plane_material;
    plane->pc.roughness = 0.5f;
    plane->pc.model = planeModel;

    m_scene.push_back(head);
    m_scene.push_back(body);
    m_scene.push_back(base);
    m_scene.push_back(plane);

    //env
    m_diffuseMipNum = static_cast<uint32_t>(floor(log2(CUBEMAP_DIM)))+1;
    m_specularMipNum = static_cast<uint32_t>(floor(log2(SPECULAR_DIM)))+1;

    VkImageCreateInfo cmci = vkinit::imageCreateInfo(VK_FORMAT_R32G32B32A32_SFLOAT,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,{CUBEMAP_DIM,CUBEMAP_DIM,1},6,
    VK_IMAGE_TYPE_2D,m_diffuseMipNum
    );
    cmci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    //cmci.extent = {FILTER_DIM,FILTER_DIM,1};
    m_diffuseCubeMap = std::make_shared<Image>(cmci,VK_IMAGE_ASPECT_COLOR_BIT);
    m_diffuseCubeMap->transitionImaglayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    cmci.extent.width = SPECULAR_DIM;
    cmci.extent.height = SPECULAR_DIM;
    cmci.mipLevels = m_specularMipNum;

    m_specularFilterMap = std::make_shared<Image>(cmci,VK_IMAGE_ASPECT_COLOR_BIT);
    m_specularFilterMap->transitionImaglayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    VkImageCreateInfo brdfci = vkinit::imageCreateInfo(VK_FORMAT_R16G16_SFLOAT,
    VK_IMAGE_USAGE_STORAGE_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,{BRDF_DIM,BRDF_DIM,1});

    m_brdfLUT = std::make_shared<Image>(brdfci,VK_IMAGE_ASPECT_COLOR_BIT);
    m_brdfLUT->transitionImaglayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_GENERAL);
    m_brdfLUT->writeToSet(m_brdfLUTDescriptor->getSet(),VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0);
    m_brdfLUT->writeToSet(m_presentDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,6);
}

void Deferred::init()
{
    initDeferredRenderPass();
    initDescriptors();
    initPipelines();
    initResources();

    generatePrecomputeMap();

    buildCmd();
}

void Deferred::buildCmd()
{
    for(size_t currentFrame = 0;currentFrame<MAX_IN_FLIGHT;currentFrame++)
    {
        //vkResetCommandBuffer(m_mainCmdBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

        VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkClearValue clearValues[4];
        VkViewport viewport{};
        VkRect2D scissor{};
        //float flash = std::abs(std::sin(_frameNumber / 120.f));
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	    clearValues[3].depthStencil = { 1.0f , 0};
        
        VK_CHECK(vkBeginCommandBuffer(m_mainCmdBuffers[currentFrame], &cmdBeginInfo));

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_deferredRenderPass->m_renderPass;
        renderPassInfo.framebuffer = m_deferredFramebuffer->m_framebuffers[currentFrame];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {VulkanContext::get()->getWidth(),VulkanContext::get()->getHeight()};
        renderPassInfo.clearValueCount = 4;
        renderPassInfo.pClearValues = clearValues;
        vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        VkDeviceSize offset = 0;

        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_deferredGPipeline->getPipeline());
        
       
        for(int i = 0;i<m_scene.size();i++)
        {
            vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_deferredGPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConsts),&m_scene[i]->pc);
            vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_deferredGPipeline->getLayout(),0,1,&m_scene[i]->material,0,nullptr);
            vkCmdBindVertexBuffers(m_mainCmdBuffers[currentFrame],0,1,&m_scene[i]->mesh->m_vertexBuffer->m_buffer,&offset);
            vkCmdBindIndexBuffer(m_mainCmdBuffers[currentFrame],m_scene[i]->mesh->m_indexBuffer->m_buffer,offset,VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(m_mainCmdBuffers[currentFrame],uint32_t(m_scene[i]->mesh->m_indices.size()),1,0,0,0);
        }
        

        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        {
            
            VkClearValue clearValues[2];
            VkViewport viewport{};
            VkRect2D scissor{};
            //float flash = std::abs(std::sin(_frameNumber / 120.f));
            clearValues[0].color = { { 0.75f, 0.85f, 1.0f, 1.0f } };
            clearValues[1].depthStencil = { 1.0f , 0};

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_renderPass->m_renderPass;
            renderPassInfo.framebuffer = m_framebuffer->m_framebuffers[currentFrame];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = {VulkanContext::get()->getWidth(),VulkanContext::get()->getHeight()};
            renderPassInfo.clearValueCount = 2;
            renderPassInfo.pClearValues = clearValues;
            vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            VkDeviceSize offset = 0;

            drawSkybox(m_mainCmdBuffers[currentFrame]);

            vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_presentGPipeline->getPipeline());
            //vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_graphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
            vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_presentGPipeline->getLayout(),0,1,&m_presentDescriptor->getSet(),0,nullptr);

            vkCmdDraw(m_mainCmdBuffers[currentFrame],3,1,0,0);

            vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);
        }

        VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));
    }
}

void Deferred::tick()
{
    uint32_t currentFrame = 0;
    VK_CHECK(vkAcquireNextImageKHR(VulkanContext::get()->getDevice(),VulkanContext::get()->getSwapchain().m_swapchain,UINT64_MAX,m_presentSemaphore,VK_NULL_HANDLE,&currentFrame))

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &m_presentSemaphore;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_renderSemaphore;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pImageIndices = &currentFrame;
    present.swapchainCount = 1;
    present.pSwapchains = &VulkanContext::get()->getSwapchain().m_swapchain;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &m_renderSemaphore;
    VK_CHECK(vkQueuePresentKHR(VulkanContext::get()->getQueue(),&present))
}

void Deferred::update(float delta)
{
    m_camParam->view = m_cam->GetViewMatrix();

    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);

    m_camUBO->map(m_camParam.get());
}

void Deferred::drawSkybox(VkCommandBuffer cmd)
{
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_skyboxGPipeline->getPipeline());
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,m_skyboxGPipeline->getLayout(),0,1,&m_skyboxDescriptor->getSet(),0,nullptr);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd,0,1,&m_skybox->m_vertexBuffer->m_buffer,&offset);
    vkCmdBindIndexBuffer(cmd,m_skybox->m_indexBuffer->m_buffer,offset,VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd,m_skybox->m_indices.size(),1,0,0,0);
}

Deferred::~Deferred()
{
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
}

void Deferred::generatePrecomputeMap()
{

    std::vector<glm::mat4> matrices = {
        // POSITIVE_X
        glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // NEGATIVE_X
        glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // POSITIVE_Y
        glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // NEGATIVE_Y
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // POSITIVE_Z
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // NEGATIVE_Z
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    };

    uint32_t currentFrame = 0;
    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo();
    VkClearValue clearValues[2];
    VkViewport viewport{};
    VkRect2D scissor{};

    viewport = vkinit::viewport((float)CUBEMAP_DIM, (float)CUBEMAP_DIM, 0.0f, 1.0f);
    scissor = vkinit::rect2D(CUBEMAP_DIM, CUBEMAP_DIM, 0, 0);
    //float flash = std::abs(std::sin(_frameNumber / 120.f));
    clearValues[0].color = { { 0.75f, 0.85f, 1.0f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f , 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_filterRenderPass->m_renderPass;
    renderPassInfo.framebuffer = m_filterFramebuffer->m_framebuffers[currentFrame];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {CUBEMAP_DIM,CUBEMAP_DIM};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkResetCommandBuffer(m_diffuseFilterCmd,0);
    renderPassInfo.renderArea = {CUBEMAP_DIM,CUBEMAP_DIM};

    vkBeginCommandBuffer(m_diffuseFilterCmd,&cmdBeginInfo);

    vkCmdSetViewport(m_diffuseFilterCmd,0,1,&viewport);
    vkCmdSetScissor(m_diffuseFilterCmd,0,1,&scissor);
   
    for(int m = 0;m<m_diffuseMipNum;m++)
    {
        for(int i = 0;i<6;i++)
        {
            viewport.width = static_cast<float>(CUBEMAP_DIM * std::pow(0.5f, m));
			viewport.height = static_cast<float>(CUBEMAP_DIM * std::pow(0.5f, m));
            vkCmdSetViewport(m_diffuseFilterCmd,0,1,&viewport);

            vkCmdBeginRenderPass(m_diffuseFilterCmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            VkDeviceSize offset = 0;

            vkCmdBindPipeline(m_diffuseFilterCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_diffuseFilterGPipeline->getPipeline());
            //vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_graphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
            vkCmdBindDescriptorSets(m_diffuseFilterCmd,VK_PIPELINE_BIND_POINT_GRAPHICS,m_diffuseFilterGPipeline->getLayout(),0,1,&m_diffuseFilterSet,0,nullptr);

            glm::mat4 proj = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f);
            //proj[1][1] *= -1;
            m_hdrToCubeMapMat =  proj* matrices[i];
            vkCmdPushConstants(m_diffuseFilterCmd,m_diffuseFilterGPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(glm::mat4),&m_hdrToCubeMapMat);
            vkCmdBindVertexBuffers(m_diffuseFilterCmd,0,1,&m_skybox->m_vertexBuffer->m_buffer,&offset);
            vkCmdBindIndexBuffer(m_diffuseFilterCmd,m_skybox->m_indexBuffer->m_buffer,offset,VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(m_diffuseFilterCmd,m_skybox->m_indices.size(),1,0,0,0);

            vkCmdEndRenderPass(m_diffuseFilterCmd);

            m_filterFramebuffer->m_colorAttachments[0]->transitionImaglayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,m_diffuseFilterCmd);

            VkImageCopy copyRegion = {};

            copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.srcSubresource.baseArrayLayer = 0;
            copyRegion.srcSubresource.mipLevel = 0;
            copyRegion.srcSubresource.layerCount = 1;
            copyRegion.srcOffset = { 0, 0, 0 };

            copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.dstSubresource.baseArrayLayer = i;
            copyRegion.dstSubresource.mipLevel = m;
            copyRegion.dstSubresource.layerCount = 1;
            copyRegion.dstOffset = { 0, 0, 0 };

            copyRegion.extent.width = viewport.width;
            copyRegion.extent.height = viewport.height;
            copyRegion.extent.depth = 1;

            vkCmdCopyImage(
                m_diffuseFilterCmd,
                m_filterFramebuffer->m_colorAttachments[0]->m_image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                m_diffuseCubeMap->m_image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &copyRegion);

            m_filterFramebuffer->m_colorAttachments[0]->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,m_diffuseFilterCmd);

        }
    }

    m_diffuseCubeMap->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,m_diffuseFilterCmd);
    vkEndCommandBuffer(m_diffuseFilterCmd);
    

    VkSubmitInfo submitCP{};
    submitCP.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitCP.commandBufferCount = 1;
    submitCP.pCommandBuffers = &m_diffuseFilterCmd;
    submitCP.waitSemaphoreCount = 0;
    //submitCP.pWaitSemaphores = &m_presentSemaphores[currentFrame];
    submitCP.signalSemaphoreCount = 0;
    //submitCP.pSignalSemaphores = &m_renderSemaphores[currentFrame];
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submitCP,VK_NULL_HANDLE))
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());

    m_diffuseCubeMap->writeToSet(m_skyboxDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2);

    vkResetCommandBuffer(m_diffuseFilterCmd,0);

    renderPassInfo.renderArea = {SPECULAR_DIM,SPECULAR_DIM};
    scissor.extent = {SPECULAR_DIM,SPECULAR_DIM};

    vkBeginCommandBuffer(m_diffuseFilterCmd,&cmdBeginInfo);

    vkCmdSetViewport(m_diffuseFilterCmd,0,1,&viewport);
    vkCmdSetScissor(m_diffuseFilterCmd,0,1,&scissor);
   
    for(int m = 0;m<m_specularMipNum;m++)
    {
        for(int i = 0;i<6;i++)
        {
            viewport.width = static_cast<float>(SPECULAR_DIM * std::pow(0.5f, m));
			viewport.height = static_cast<float>(SPECULAR_DIM * std::pow(0.5f, m));
            vkCmdSetViewport(m_diffuseFilterCmd,0,1,&viewport);

            vkCmdBeginRenderPass(m_diffuseFilterCmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            VkDeviceSize offset = 0;

            vkCmdBindPipeline(m_diffuseFilterCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_specularFilterGPipeline->getPipeline());
            //vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_graphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
            vkCmdBindDescriptorSets(m_diffuseFilterCmd,VK_PIPELINE_BIND_POINT_GRAPHICS,m_specularFilterGPipeline->getLayout(),0,1,&m_specularFilterSet,0,nullptr);

            glm::mat4 proj = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f);
            //proj[1][1] *= -1;
            m_specularPC.model =  proj* matrices[i];
            m_specularPC.roughness = (float)m / (float)(m_specularMipNum - 1);
            vkCmdPushConstants(m_diffuseFilterCmd,m_specularFilterGPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConsts),&m_specularPC);
            vkCmdBindVertexBuffers(m_diffuseFilterCmd,0,1,&m_skybox->m_vertexBuffer->m_buffer,&offset);
            vkCmdBindIndexBuffer(m_diffuseFilterCmd,m_skybox->m_indexBuffer->m_buffer,offset,VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(m_diffuseFilterCmd,m_skybox->m_indices.size(),1,0,0,0);

            vkCmdEndRenderPass(m_diffuseFilterCmd);

            m_filterFramebuffer->m_colorAttachments[0]->transitionImaglayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,m_diffuseFilterCmd);

            VkImageCopy copyRegion = {};

            copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.srcSubresource.baseArrayLayer = 0;
            copyRegion.srcSubresource.mipLevel = 0;
            copyRegion.srcSubresource.layerCount = 1;
            copyRegion.srcOffset = { 0, 0, 0 };

            copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.dstSubresource.baseArrayLayer = i;
            copyRegion.dstSubresource.mipLevel = m;
            copyRegion.dstSubresource.layerCount = 1;
            copyRegion.dstOffset = { 0, 0, 0 };

            copyRegion.extent.width = viewport.width;
            copyRegion.extent.height = viewport.height;
            copyRegion.extent.depth = 1;

            vkCmdCopyImage(
                m_diffuseFilterCmd,
                m_filterFramebuffer->m_colorAttachments[0]->m_image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                m_specularFilterMap->m_image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &copyRegion);

            m_filterFramebuffer->m_colorAttachments[0]->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,m_diffuseFilterCmd);

        }
    }

    m_specularFilterMap->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,m_diffuseFilterCmd);
    vkEndCommandBuffer(m_diffuseFilterCmd);
    

    //VkSubmitInfo submitCP{};
    submitCP.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitCP.commandBufferCount = 1;
    submitCP.pCommandBuffers = &m_diffuseFilterCmd;
    submitCP.waitSemaphoreCount = 0;
    //submitCP.pWaitSemaphores = &m_presentSemaphores[currentFrame];
    submitCP.signalSemaphoreCount = 0;
    //submitCP.pSignalSemaphores = &m_renderSemaphores[currentFrame];
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submitCP,VK_NULL_HANDLE))
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());

    m_specularFilterMap->writeToSet(m_skyboxDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3);
    m_specularFilterMap->writeToSet(m_presentDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,7);
    m_diffuseCubeMap->writeToSet(m_presentDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,8);

    vkResetCommandBuffer(m_brdfLUTCmd,0);
    cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    
    VK_CHECK(vkBeginCommandBuffer(m_brdfLUTCmd, &cmdBeginInfo));
    vkCmdBindPipeline(m_brdfLUTCmd,VK_PIPELINE_BIND_POINT_COMPUTE,m_brdfLUTCPipeline->getPipeline());
    vkCmdBindDescriptorSets(m_brdfLUTCmd,VK_PIPELINE_BIND_POINT_COMPUTE,
    m_brdfLUTCPipeline->getLayout(),0,1,&m_brdfLUTDescriptor->getSet(),0,nullptr);

    vkCmdDispatch(m_brdfLUTCmd, BRDF_DIM / 8, BRDF_DIM / 8, 1);

    VK_CHECK(vkEndCommandBuffer(m_brdfLUTCmd));

    submitCP.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitCP.commandBufferCount = 1;
    submitCP.pCommandBuffers = &m_brdfLUTCmd;
    submitCP.waitSemaphoreCount = 0;
    //submitCP.pWaitSemaphores = &m_presentSemaphores[currentFrame];
    submitCP.signalSemaphoreCount = 0;
    //submitCP.pSignalSemaphores = &m_renderSemaphores[currentFrame];
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submitCP,VK_NULL_HANDLE))
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());

}

void ImGuiTest::init()
{
    ImGuiLayer::get()->init(m_renderPass->m_renderPass);

    VkFenceCreateInfo fci = vkinit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

    vkCreateFence(VulkanContext::get()->getDevice(),&fci,nullptr,&m_fence);

    testTex = std::make_shared<Image>("../../assets/testTex.jpg",VK_FORMAT_R8G8B8A8_SRGB);

  
    //ImGui::LoadIniSettingsFromDisk("../../assets/ui/imgui.ini");

    m_descriptor = std::make_shared<Descriptor>();
    m_descriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT,0);
    m_descriptor->update();

    offscreenWidth = 1280;offscreenHeight = 720;

    float w = (float)offscreenWidth;
    float h = (float)offscreenHeight;

    std::vector<VkFormat> filterFormats = {VK_FORMAT_R8G8B8A8_SRGB};
    m_offscreenRenderPass = std::make_shared<RenderPass>(filterFormats,false);
    m_offscreenFramebuffer = std::make_shared<Framebuffer>(m_offscreenRenderPass,w,h);
    m_des = ImGui_ImplVulkan_AddTexture(m_offscreenFramebuffer->m_colorAttachments[0]->m_sampler,m_offscreenFramebuffer->m_colorAttachments[0]->m_view,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_graphicsPipeline = std::make_unique<GraphicsPipeline>("../../shaders/shaderToy/shadertoy.vert.spv","../../shaders/shaderToy/shadertoy.frag.spv",m_descriptor->getLayout(),m_offscreenRenderPass->m_renderPass,sizeof(PushConstantData));
    m_dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    m_graphicsPipeline->setDynamicState(m_dynamicStates);
    //m_graphicsPipeline->bindPushConstant(sizeof(PushConstantData));

    m_graphicsPipeline->create();

    m_inputUniform = std::make_shared<ShaderToyInputData>();

    m_inputUniform->iDate = glm::vec4(2024.0f, 1.0f, 9.0f, 0.0f);
    m_inputUniform->iFrame = m_frameNumber;
    m_inputUniform->iResolution = glm::vec3(w,h,1.0f);
    m_inputUniform->iFrameRate = 0;
    m_inputUniform->iMouse = glm::vec4(0.0f);
    m_inputUniform->iTime = 0;
    m_inputUniform->iTimeDelta = 0;

    m_inputUniformBuffer = std::make_shared<Buffer>(sizeof(ShaderToyInputData));
    m_inputUniformBuffer->writeToSet(m_descriptor->getSet(),0);

    m_pcd.width = w;
    m_pcd.height = h;

}

void ImGuiTest::update(float delta)
{
    m_inputUniform->iTime += delta;
    m_inputUniform->iTimeDelta = delta;
    
    double xpos, ypos;
    static double last_click_x = 0, last_click_y = 0;
    glfwGetCursorPos(VulkanContext::get()->getWindow(), &xpos, &ypos);
    static bool btn1_down;
    auto btn1 = glfwGetMouseButton(VulkanContext::get()->getWindow(), GLFW_MOUSE_BUTTON_1);
    m_inputUniform->iMouse.z = 0;
    m_inputUniform->iMouse.w = 0;
    if (!btn1_down && btn1 == GLFW_PRESS) {
        btn1_down = true;
    } else if (btn1_down && btn1 == GLFW_RELEASE) {
        btn1_down = false;
        last_click_x = xpos;
        last_click_y = ypos;
        m_inputUniform->iMouse.w = 1;
    }
    if (btn1_down) {
        m_inputUniform->iMouse.x = xpos;
        m_inputUniform->iMouse.y = ypos;
        m_inputUniform->iMouse.z = 1;
    }

    m_inputUniformBuffer->map(m_inputUniform.get());

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    bool flag = true;
   // ImGui::ShowDemoWindow();
    
    //m_des = ImGui_ImplVulkan_AddTexture(testTex->m_sampler,m_offscreenFramebuffer->m_colorAttachments[0]->m_view,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    auto *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    ImGui::DockSpace(ImGui::GetID("DockSpace"), { 0.0f, 0.0f }, 0, nullptr);
    
    ImGui::Begin("Scene");
    // ImGui::Checkbox("Show Skybox",&showSkybox);
    // ImGui::SliderFloat("Test Slider",&sharedUbo.exposure,0.1f,10.0f);
    ImGui::Image(m_des,ImVec2(1280,720));    
    ImGui::End();

    ImGui::Begin("Settings");
    // ImGui::Checkbox("Show Skybox",&showSkybox);
    // ImGui::SliderFloat("Test Slider",&sharedUbo.exposure,0.1f,10.0f);
    ImGui::Text("hello ImGui");
    ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);    
    ImGui::End();

    ImGui::Begin("Test");
    // ImGui::Checkbox("Show Skybox",&showSkybox);
    // ImGui::SliderFloat("Test Slider",&sharedUbo.exposure,0.1f,10.0f);
    ImGui::Text("hello ImGui");    
    ImGui::End();

    ImGui::End();


    ImGui::Render();

   // buildCmd();
}

void ImGuiTest::tick()
{
    uint32_t currentFrame = m_frameNumber % 2;
    uint32_t nextImage = 0;
    VK_CHECK(vkWaitForFences(VulkanContext::get()->getDevice(),1,&m_fence,VK_TRUE,UINT64_MAX));
    VK_CHECK(vkResetFences(VulkanContext::get()->getDevice(),1,&m_fence));

    VK_CHECK(vkAcquireNextImageKHR(VulkanContext::get()->getDevice(),VulkanContext::get()->getSwapchain().m_swapchain,UINT64_MAX,m_presentSemaphore,VK_NULL_HANDLE,&nextImage))

    ImDrawData* draw_data = ImGui::GetDrawData();
    
    vkResetCommandBuffer(m_mainCmdBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo();

    uint32_t w = VulkanContext::get()->getWidth();
    uint32_t h = VulkanContext::get()->getHeight();

    VkClearValue clearValues[2];
    VkViewport viewport{};
    VkRect2D scissor{};
    
    //float flash = std::abs(std::sin(_frameNumber / 120.f));
    clearValues[0].color = { { 0.75f, 0.85f, 1.0f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f , 0};
    
    VK_CHECK(vkBeginCommandBuffer(m_mainCmdBuffers[currentFrame], &cmdBeginInfo));

    viewport = vkinit::viewport((float)offscreenWidth, (float)offscreenHeight, 0.0f, 1.0f);
    scissor = vkinit::rect2D(offscreenWidth, offscreenHeight, 0, 0);

    vkCmdSetViewport(m_mainCmdBuffers[currentFrame],0,1,&viewport);
    vkCmdSetScissor(m_mainCmdBuffers[currentFrame],0,1,&scissor);

    VkRenderPassBeginInfo offscreenRenderPassInfo{};
    offscreenRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    offscreenRenderPassInfo.renderPass = m_offscreenRenderPass->m_renderPass;
    offscreenRenderPassInfo.framebuffer = m_offscreenFramebuffer->m_framebuffers[currentFrame];
    offscreenRenderPassInfo.renderArea.offset = {0, 0};
    offscreenRenderPassInfo.renderArea.extent = {offscreenWidth,offscreenHeight};
    offscreenRenderPassInfo.clearValueCount = 2;
    offscreenRenderPassInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &offscreenRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkDeviceSize offset = 0;

    vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->getPipeline());
    vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_graphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
    vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_graphicsPipeline->getLayout(),0,1,&m_descriptor->getSet(),0,nullptr);

    vkCmdDraw(m_mainCmdBuffers[currentFrame],3,1,0,0);

    vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

    viewport = vkinit::viewport((float)w, (float)h, 0.0f, 1.0f);
    scissor = vkinit::rect2D(w, h, 0, 0);

    vkCmdSetViewport(m_mainCmdBuffers[currentFrame],0,1,&viewport);
    vkCmdSetScissor(m_mainCmdBuffers[currentFrame],0,1,&scissor);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass->m_renderPass;
    renderPassInfo.framebuffer = m_framebuffer->m_framebuffers[nextImage];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {w,h};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    //VkDeviceSize offset = 0;

    // vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->getPipeline());
    // vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_graphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
    // vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_graphicsPipeline->getLayout(),0,1,&m_descriptor->getSet(),0,nullptr);

    // vkCmdDraw(m_mainCmdBuffers[currentFrame],3,1,0,0);
    ImGui_ImplVulkan_RenderDrawData(draw_data, m_mainCmdBuffers[currentFrame]);

    vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

    VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &m_presentSemaphore;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_renderSemaphore;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,m_fence))

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pImageIndices = &nextImage;
    present.swapchainCount = 1;
    present.pSwapchains = &VulkanContext::get()->getSwapchain().m_swapchain;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &m_renderSemaphore;
    auto result = vkQueuePresentKHR(VulkanContext::get()->getQueue(),&present);

    m_frameNumber++;
}

void ImGuiTest::buildCmd()
{
    uint32_t currentFrame = 0;
	
    auto result2 = vkAcquireNextImageKHR(VulkanContext::get()->getDevice(),VulkanContext::get()->getSwapchain().m_swapchain,UINT64_MAX,m_presentSemaphore,VK_NULL_HANDLE,&currentFrame);

    // if ((result2 == VK_ERROR_OUT_OF_DATE_KHR) || (result2 == VK_SUBOPTIMAL_KHR)) {
		
	// 	if(result2 == VK_ERROR_OUT_OF_DATE_KHR)
    //     {
    //         windowResize();
    //         LOG_TRACE("SwapChain Reset");
    //     }
	// }
	// else {
	// 	VK_CHECK(result2);
	// }

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &m_presentSemaphore;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_renderSemaphore;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pImageIndices = &currentFrame;
    present.swapchainCount = 1;
    present.pSwapchains = &VulkanContext::get()->getSwapchain().m_swapchain;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &m_renderSemaphore;
    auto result = vkQueuePresentKHR(VulkanContext::get()->getQueue(),&present);
	//m_frameNumber ++;
    // if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
	// 	windowResize();
    //     LOG_TRACE("SwapChain Reset");
		
	// }
	// else {
	// 	VK_CHECK(result);
	// }
}

void ImGuiTest::windowResize()
{

}

ImGuiTest::~ImGuiTest()
{
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
    vkDestroyFence(VulkanContext::get()->getDevice(),m_fence,nullptr);
    ImGuiLayer::get()->release();
}

void FC::loadAssets()
{   
    glm::mat4 model = glm::mat4(1.0);
    m_teapot = std::make_shared<Mesh>("../../assets/sphere.obj");
    m_teapot->m_pc.objectColor = glm::vec4(0.55,0.93,0.70,1.0);
    glm::mat4 sc = glm::scale(model,glm::vec3(1.0,1.0,1.0));
    m_teapot->m_pc.model = glm::translate(model,glm::vec3(m_lightubo.pos))*sc;
    m_empty = std::make_shared<Image>("../../assets/empty.png",VK_FORMAT_R8G8B8A8_SRGB);

    m_planeLight = std::make_shared<Mesh>("../../assets/toyman/plane.obj");
    m_planeLight->m_pc.objectColor = glm::vec4(0.0,0.0,1.0,1.0);
    glm::mat4 rotx = glm::rotate(model,glm::radians(90.0f),glm::vec3(1.0,0.0,0.0));
    glm::mat4 roty = glm::rotate(model,glm::radians(90.0f),glm::vec3(0.0,1.0,0.0));
    glm::mat4 pls = glm::scale(model,glm::vec3(20.0,20.0,20.0));
    glm::mat4 plt = glm::translate(model,glm::vec3(-20,5,0));
    m_planeLight->m_pc.model = plt*roty*rotx*pls;

    vkinit::Plane rectLight = initPlane(m_planeLight->bounding_box.MaxPoint,m_planeLight->bounding_box.MinPoint);
    rectLight.v1 = m_planeLight->m_pc.model*rectLight.v1;
    rectLight.v2 = m_planeLight->m_pc.model*rectLight.v2;
    rectLight.v3 = m_planeLight->m_pc.model*rectLight.v3;
    rectLight.v4 = m_planeLight->m_pc.model*rectLight.v4;
    m_lightubo.rectLight = rectLight;
   
    m_plane = std::make_shared<Mesh>("../../assets/toyman/plane.obj");
    m_plane->m_pc.model = glm::scale(model,glm::vec3(130.0f,130.0f,130.0f));
    m_plane->m_pc.objectColor = glm::vec4(0.8,0.83,0.84,0.0);
    //m_plane_albedo = std::make_shared<Image>("../../assets/rusty/rusty-metal_albedo.png",VK_FORMAT_R8G8B8A8_SRGB);
    m_plane_albedo = std::make_shared<Image>("../../assets/rusted_iron/albedo.png",VK_FORMAT_R8G8B8A8_UNORM);  
    m_plane_normal = std::make_shared<Image>("../../assets/rusted_iron/normal.png",VK_FORMAT_R8G8B8A8_UNORM);
    m_plane_ao = std::make_shared<Image>("../../assets/rusted_iron/ao.png",VK_FORMAT_R8G8B8A8_UNORM);
    m_plane_metal = std::make_shared<Image>("../../assets/rusted_iron/metallic.png",VK_FORMAT_R8G8B8A8_UNORM);
    m_plane_roughness = std::make_shared<Image>("../../assets/rusted_iron/roughness.png",VK_FORMAT_R8G8B8A8_UNORM);

    RenderObject teapot;
    teapot.mesh = m_teapot;
    teapot.descriptorSet = m_presentDescriptor->makeSet();
    m_empty->writeToSet(teapot.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);
    m_empty->writeToSet(teapot.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2);
    m_empty->writeToSet(teapot.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3);
    m_empty->writeToSet(teapot.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4);
    m_empty->writeToSet(teapot.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,5);

    RenderObject plane;
    plane.mesh = m_plane;
    plane.descriptorSet = m_presentDescriptor->makeSet();
    m_plane_albedo->writeToSet(plane.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);
    m_plane_normal->writeToSet(plane.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2);
    m_plane_metal->writeToSet(plane.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3);
    m_plane_ao->writeToSet(plane.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4);
    m_plane_roughness->writeToSet(plane.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,5);

    RenderObject planeLight;
    planeLight.mesh = m_planeLight;
    planeLight.descriptorSet = m_presentDescriptor->makeSet();
    m_empty->writeToSet(planeLight.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);
    m_empty->writeToSet(planeLight.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2);
    m_empty->writeToSet(planeLight.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3);
    m_empty->writeToSet(planeLight.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4);
    m_empty->writeToSet(planeLight.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,5);

    m_scenes.push_back(teapot);
    m_scenes.push_back(plane);
    m_scenes.push_back(planeLight);

}

void FC::init()
{
    m_presentDescriptor = std::make_shared<Descriptor>();
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1);//albedo
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2);//normal
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,3);//metal
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,4);//ao
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,5);//roughness
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT,6);//light
    m_presentDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,7);//shadowmap
    m_presentDescriptor->update();

    m_boxDescriptor = std::make_shared<Descriptor>();
    m_boxDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
    m_boxDescriptor->update();

    m_lightubo.pos = glm::vec4(8.0,4.0,0.0,1.0);
    loadAssets();

    std::vector<VkFormat> formats = {
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_FORMAT_R16G16B16A16_SFLOAT
        };
    m_lightRenderPass = std::make_shared<RenderPass>(formats);
    m_lightFramebuffer = std::make_shared<Framebuffer>(m_lightRenderPass);

    m_presentGPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/obj/boundingbox/v.vert.spv",
        "../../shaders/obj/boundingbox/f.frag.spv",
        m_presentDescriptor->getLayout(),
        m_lightRenderPass->m_renderPass,
        sizeof(Mesh::PushConst)
        );
    std::cout<<std::endl<<sizeof(Mesh::PushConst);

    VertexInputDescription des = Vertex::get_vertex_description();

    VkPipelineVertexInputStateCreateInfo ci = vkinit::vertexInputStateCreateInfo();

    ci.vertexAttributeDescriptionCount = des.attributes.size();
	ci.pVertexAttributeDescriptions = des.attributes.data();
	ci.vertexBindingDescriptionCount = des.bindings.size();
	ci.pVertexBindingDescriptions = des.bindings.data();

    m_presentGPipeline->setVertexInputState(ci);

    m_presentGPipeline->setColorBlendState(2);

    m_presentGPipeline->create();

    // m_boundingBoxGPipeline = std::make_unique<GraphicsPipeline>(
    //     "../../shaders/obj/boundingbox/box.vert.spv",
    //     "../../shaders/obj/boundingbox/box.frag.spv",
    //     m_boxDescriptor->getLayout(),
    //     m_lightRenderPass->m_renderPass,
    //     sizeof(glm::mat4)
    //     );

    // VkVertexInputBindingDescription mainBinding = {};
	// mainBinding.binding = 0;
	// mainBinding.stride = sizeof(glm::vec3);
	// mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // VkVertexInputAttributeDescription positionAttribute = {};
	// positionAttribute.binding = 0;
	// positionAttribute.location = 0;
	// positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	// positionAttribute.offset = 0;

    // ci.vertexAttributeDescriptionCount = 1;
	// ci.pVertexAttributeDescriptions = &positionAttribute;
	// ci.vertexBindingDescriptionCount = 1;
	// ci.pVertexBindingDescriptions = &mainBinding;

    // m_boundingBoxGPipeline->setVertexInputState(ci);

    // VkPipelineInputAssemblyStateCreateInfo iaci = vkinit::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    // m_boundingBoxGPipeline->setInputAssemblyState(iaci);

    // m_boundingBoxGPipeline->create();

  

    m_cam = VulkanContext::get()->getCamera();

    m_camUBO = std::make_shared<Buffer>(sizeof(vkinit::CameraUBO));
    m_camUBO->writeToSet(m_boxDescriptor->getSet(),0);

    m_camParam = std::make_shared<vkinit::CameraUBO>();

    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1920.f / 1080.f, 0.1f, 200.0f);
	projection[1][1] *= -1;

    m_camParam->view = m_cam->GetViewMatrix();
    m_camParam->project = projection;
    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);

    m_camUBO->map(m_camParam.get());

   
    m_lightBuffer = std::make_shared<Buffer>(sizeof(vkinit::LightUBO));
    
    std::cout<<std::endl<<sizeof(vkinit::LightUBO)<<std::endl;
   // Factory::CreateShadowPassResource(m_lightubo.pos,m_shadowPass,m_lightubo.lightSpace);

    m_lightBuffer->map(&m_lightubo);

    for(auto& obj:m_scenes)
    {
        m_camUBO->writeToSet(obj.descriptorSet,0);
        m_lightBuffer->writeToSet(obj.descriptorSet,6);
        //m_shadowPass.framebuffer->m_depth->writeToSet(obj.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,7);
    }

    //bloom
    m_offscreenDescriptor = std::make_shared<Descriptor>();
    m_offscreenDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,0);
    m_offscreenDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1);
    m_offscreenDescriptor->update();

    m_offscreenGPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/obj/boundingbox/offscreen.vert.spv",
        "../../shaders/obj/boundingbox/offscreen.frag.spv",
        m_offscreenDescriptor->getLayout(),
        m_renderPass->m_renderPass
    );

    VkPipelineRasterizationStateCreateInfo rsci = vkinit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
    rsci.cullMode = VK_CULL_MODE_NONE;
    m_offscreenGPipeline->setRasterizationState(rsci);

    m_offscreenGPipeline->create();

    m_lightFramebuffer->m_colorAttachments[0]->writeToSet(
        m_offscreenDescriptor->getSet(),
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        0
    );

    initBlurPass();

    buildCmd();
}

void FC::initBlurPass()
{
    uint32_t w = VulkanContext::get()->getWidth();
    uint32_t h = VulkanContext::get()->getHeight();


    VkImageCreateInfo cmci = vkinit::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,{w,h,1},1,
    VK_IMAGE_TYPE_2D,BLUR_MIP
    );

    m_downSamplingDescriptor = std::make_shared<Descriptor>();
    m_downSamplingDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,0);
    m_downSamplingDescriptor->update();

    m_downSamplingImage = std::make_shared<Image>(cmci,VK_IMAGE_ASPECT_COLOR_BIT);
    //m_downSamplingImage->generateMipmaps();
    m_downSamplingImage->writeToSet(m_downSamplingDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,0);
    m_downSamplingImage->writeToSet(m_offscreenDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);

    m_downSamplingImage->transitionImaglayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkFormat> formats{VK_FORMAT_R16G16B16A16_SFLOAT};

    m_downSamplingRenderPass = std::make_shared<RenderPass>(formats);
    m_downSamplingFramebuffer = std::make_shared<Framebuffer>(m_downSamplingRenderPass);

    m_dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    m_downSamplingPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/obj/boundingbox/blur/downsampling.vert.spv",
        "../../shaders/obj/boundingbox/blur/downsampling.frag.spv",
        m_downSamplingDescriptor->getLayout(),
        m_downSamplingRenderPass->m_renderPass,
        sizeof(float)
    );

    m_downSamplingPipeline->setDynamicState(m_dynamicStates);
    VkPipelineRasterizationStateCreateInfo rsci = vkinit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
    rsci.cullMode = VK_CULL_MODE_NONE;
    m_downSamplingPipeline->setRasterizationState(rsci);
    m_downSamplingPipeline->create();

    m_upSamplingPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/obj/boundingbox/blur/upsampling.vert.spv",
        "../../shaders/obj/boundingbox/blur/upsampling.frag.spv",
        m_downSamplingDescriptor->getLayout(),
        m_downSamplingRenderPass->m_renderPass,
        sizeof(float)
    );

    m_upSamplingPipeline->setDynamicState(m_dynamicStates);
    
    m_upSamplingPipeline->setRasterizationState(rsci);
    m_upSamplingPipeline->create();

}

void FC::downSamplingPass(VkCommandBuffer cmd)
{
    uint32_t w = VulkanContext::get()->getWidth();
    uint32_t h = VulkanContext::get()->getHeight();

    VkClearValue clearValues[2];
    VkViewport viewport{};
    VkRect2D scissor{};
    VkDeviceSize offset = 0;
    //float flash = std::abs(std::sin(_frameNumber / 120.f));
    clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f , 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_downSamplingRenderPass->m_renderPass;
    renderPassInfo.framebuffer = m_downSamplingFramebuffer->m_framebuffers[0];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {VulkanContext::get()->getWidth(),VulkanContext::get()->getHeight()};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    viewport = vkinit::viewport((float)w, (float)h, 0.0f, 1.0f);
    scissor = vkinit::rect2D(w, h, 0, 0);
   
    vkCmdSetViewport(cmd,0,1,&viewport);
    vkCmdSetScissor(cmd,0,1,&scissor);
   
    for(int i = 1;i<BLUR_MIP;i++)
    {
        float sampleIndex = static_cast<float>(i-1);
        viewport.width = static_cast<float>(w * std::pow(0.5f, i));
		viewport.height = static_cast<float>(h * std::pow(0.5f, i));

        //m_downSamplingImage->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,cmd);
        vkCmdSetViewport(cmd,0,1,&viewport);

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_downSamplingPipeline->getPipeline());
        vkCmdPushConstants(cmd,m_downSamplingPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(float),&sampleIndex);
        vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,m_downSamplingPipeline->getLayout(),0,1,&m_downSamplingDescriptor->getSet(),0,nullptr);
        vkCmdDraw(cmd,3,1,0,0);

        vkCmdEndRenderPass(cmd);

        m_downSamplingFramebuffer->m_colorAttachments[0]->transitionImaglayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,cmd);
        m_downSamplingImage->transitionImaglayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,cmd);
        VkImageCopy copyRegion = {};

        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.srcOffset = { 0, 0, 0 };

        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.baseArrayLayer = 0;
        copyRegion.dstSubresource.mipLevel = i;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.dstOffset = { 0, 0, 0 };

        copyRegion.extent.width = viewport.width;
        copyRegion.extent.height = viewport.height;
        copyRegion.extent.depth = 1;

        vkCmdCopyImage(
            cmd,
            m_downSamplingFramebuffer->m_colorAttachments[0]->m_image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            m_downSamplingImage->m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &copyRegion
        );

        m_downSamplingFramebuffer->m_colorAttachments[0]->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,cmd);
        m_downSamplingImage->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,cmd);
    }

    //m_downSamplingImage->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,cmd);
}

void FC::upSamplingPass(VkCommandBuffer cmd)
{
    uint32_t w = VulkanContext::get()->getWidth();
    uint32_t h = VulkanContext::get()->getHeight();

    VkClearValue clearValues[2];
    VkViewport viewport{};
    VkRect2D scissor{};
    VkDeviceSize offset = 0;
    //float flash = std::abs(std::sin(_frameNumber / 120.f));
    clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f , 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_downSamplingRenderPass->m_renderPass;
    renderPassInfo.framebuffer = m_downSamplingFramebuffer->m_framebuffers[0];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {VulkanContext::get()->getWidth(),VulkanContext::get()->getHeight()};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    viewport = vkinit::viewport((float)w, (float)h, 0.0f, 1.0f);
    scissor = vkinit::rect2D(w, h, 0, 0);
   
    vkCmdSetViewport(cmd,0,1,&viewport);
    vkCmdSetScissor(cmd,0,1,&scissor);
   
    for(int i = BLUR_MIP - 2;i>=0;i--)
    {
        float sampleIndex = static_cast<float>(i+1);
        viewport.width = static_cast<float>(w * std::pow(0.5f, i));
		viewport.height = static_cast<float>(h * std::pow(0.5f, i));

        //m_downSamplingImage->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,cmd);
        vkCmdSetViewport(cmd,0,1,&viewport);

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_upSamplingPipeline->getPipeline());
        vkCmdPushConstants(cmd,m_upSamplingPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(float),&sampleIndex);
        vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,m_upSamplingPipeline->getLayout(),0,1,&m_downSamplingDescriptor->getSet(),0,nullptr);
        vkCmdDraw(cmd,3,1,0,0);

        vkCmdEndRenderPass(cmd);

        m_downSamplingFramebuffer->m_colorAttachments[0]->transitionImaglayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,cmd);
        m_downSamplingImage->transitionImaglayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,cmd);
        VkImageCopy copyRegion = {};

        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.srcOffset = { 0, 0, 0 };

        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.baseArrayLayer = 0;
        copyRegion.dstSubresource.mipLevel = i;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.dstOffset = { 0, 0, 0 };

        copyRegion.extent.width = viewport.width;
        copyRegion.extent.height = viewport.height;
        copyRegion.extent.depth = 1;

        vkCmdCopyImage(
            cmd,
            m_downSamplingFramebuffer->m_colorAttachments[0]->m_image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            m_downSamplingImage->m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &copyRegion
        );

        m_downSamplingFramebuffer->m_colorAttachments[0]->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,cmd);
        m_downSamplingImage->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,cmd);
    }
}

void FC::buildCmd() 
{
    for(size_t currentFrame = 0;currentFrame<MAX_IN_FLIGHT;currentFrame++)
    {
        VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkClearValue clearValues[3];
        VkViewport viewport{};
        VkRect2D scissor{};
        //float flash = std::abs(std::sin(_frameNumber / 120.f));
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[2].depthStencil = { 1.0f , 0};
        
        VK_CHECK(vkBeginCommandBuffer(m_mainCmdBuffers[currentFrame], &cmdBeginInfo));

        //shadowPass(m_mainCmdBuffers[currentFrame]);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_lightRenderPass->m_renderPass;
        renderPassInfo.framebuffer = m_lightFramebuffer->m_framebuffers[currentFrame];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {VulkanContext::get()->getWidth(),VulkanContext::get()->getHeight()};
        renderPassInfo.clearValueCount = 3;
        renderPassInfo.pClearValues = clearValues;
        vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        VkDeviceSize offset = 0;

        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_presentGPipeline->getPipeline());
       
        for(auto obj:m_scenes)
        {
            vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_presentGPipeline->getLayout(),0,1,&obj.descriptorSet,0,nullptr);
            vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_presentGPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(Mesh::PushConst),&obj.mesh->m_pc);
            vkCmdBindVertexBuffers(m_mainCmdBuffers[currentFrame], 0, 1, &obj.mesh->m_vertexBuffer->m_buffer, &offset);
            vkCmdBindIndexBuffer(m_mainCmdBuffers[currentFrame], obj.mesh->m_indexBuffer->m_buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(m_mainCmdBuffers[currentFrame],uint32_t(obj.mesh->m_indices.size()),1,0,0,0);
        }


        // {
        //     vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_boundingBoxGPipeline->getPipeline());
           
        //     vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_boundingBoxGPipeline->getLayout(),0,1,&m_boxDescriptor->getSet(),0,nullptr);

        //     for(auto obj:m_scenes)
        //     {
        //         vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_presentGPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(glm::mat4),&obj.mesh->m_pc.model);
        //         vkCmdBindVertexBuffers(m_mainCmdBuffers[currentFrame], 0, 1, &obj.mesh->m_bounding_box_vertexBuffer->m_buffer, &offset);
        //         vkCmdBindIndexBuffer(m_mainCmdBuffers[currentFrame], obj.mesh->m_bounding_box_indexBuffer->m_buffer, 0, VK_INDEX_TYPE_UINT32);
        //         vkCmdDrawIndexed(m_mainCmdBuffers[currentFrame],uint32_t(obj.mesh->m_bounding_box_indices.size()),1,0,0,0);
        //     }
           
        // }

        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        m_lightFramebuffer->m_colorAttachments[1]->transitionImaglayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,m_mainCmdBuffers[currentFrame]);
        m_downSamplingImage->transitionImaglayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,m_mainCmdBuffers[currentFrame]);
        VkImageCopy copyRegion = {};

        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.srcOffset = { 0, 0, 0 };

        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.baseArrayLayer = 0;
        copyRegion.dstSubresource.mipLevel = 0;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.dstOffset = { 0, 0, 0 };

        copyRegion.extent.width = VulkanContext::get()->getWidth();
        copyRegion.extent.height = VulkanContext::get()->getHeight();
        copyRegion.extent.depth = 1;

        vkCmdCopyImage(
            m_mainCmdBuffers[currentFrame],
            m_lightFramebuffer->m_colorAttachments[1]->m_image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            m_downSamplingImage->m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &copyRegion);

        m_lightFramebuffer->m_colorAttachments[1]->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,m_mainCmdBuffers[currentFrame]);
        m_downSamplingImage->transitionImaglayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,m_mainCmdBuffers[currentFrame]);
        
        downSamplingPass(m_mainCmdBuffers[currentFrame]);
        upSamplingPass(m_mainCmdBuffers[currentFrame]);

        VkClearValue offscreenClearValues[2];
        offscreenClearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        offscreenClearValues[1].depthStencil  = {1.0,0};
        renderPassInfo.renderPass = m_renderPass->m_renderPass;
        renderPassInfo.framebuffer = m_framebuffer->m_framebuffers[currentFrame];
        renderPassInfo.pClearValues = offscreenClearValues;
        renderPassInfo.clearValueCount = 2;
        vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_offscreenGPipeline->getPipeline());
        //vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_graphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_offscreenGPipeline->getLayout(),0,1,&m_offscreenDescriptor->getSet(),0,nullptr);

        vkCmdDraw(m_mainCmdBuffers[currentFrame],3,1,0,0);

        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));
    }
}

void FC::tick()
{
     uint32_t currentFrame = 0;

    auto result2 = vkAcquireNextImageKHR(VulkanContext::get()->getDevice(),VulkanContext::get()->getSwapchain().m_swapchain,UINT64_MAX,m_presentSemaphore,VK_NULL_HANDLE,&currentFrame);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &m_presentSemaphore;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_renderSemaphore;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pImageIndices = &currentFrame;
    present.swapchainCount = 1;
    present.pSwapchains = &VulkanContext::get()->getSwapchain().m_swapchain;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &m_renderSemaphore;
    auto result = vkQueuePresentKHR(VulkanContext::get()->getQueue(),&present);
}

void FC::update(float delta) 
{
    m_time += delta;

    m_camParam->view = m_cam->GetViewMatrix();

    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);

    m_camUBO->map(m_camParam.get());

    m_lightubo.pos.x = 5*cos(m_time/(M_PI));
    m_lightubo.pos.z = 5*sin(m_time/(M_PI));
    //m_lightBuffer->map(&m_lightubo);
}

void FC::windowResize() 
{

}
FC::~FC()
{
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
}

vkinit::Plane FC::initPlane(const glm::vec3& vmax, const glm::vec3& vmin)
{
    glm::vec3 z = (vmax.z - vmin.z)*glm::vec3(0.0,0.0,1.0);
    glm::vec3 x = (vmax.x - vmin.x)*glm::vec3(1.0,0.0,0.0);

    vkinit::Plane plane;
    plane.v1 = glm::vec4(vmax,1.0);
    plane.v2 = glm::vec4(vmax - z,1.0);
    plane.v3 = glm::vec4(vmin,1.0);
    plane.v4 = glm::vec4(vmax - x,1.0);

    return plane;
}

void ShadowMapping::initRenderPass()
{
    std::vector<VkFormat> formats = {
            VK_FORMAT_R8G8B8A8_UNORM
        };

    m_staticObjectRenderPass = std::make_shared<RenderPass>(formats);

    m_staticObjectFramebuffer = std::make_shared<Framebuffer>(m_staticObjectRenderPass);
}

void ShadowMapping::initPipeline()
{
    m_staticObjectDescriptor = std::make_shared<Descriptor>();
    m_staticObjectDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
    m_staticObjectDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,1);
    m_staticObjectDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2);
    m_staticObjectDescriptor->update();

    m_offscreenDescriptor = std::make_shared<Descriptor>();
    m_offscreenDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,0);
    m_offscreenDescriptor->update();

    m_staticObjectGraphicsPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/Assobj/staticobj.vert.spv",
        "../../shaders/Assobj/staticobj.frag.spv",
        m_staticObjectDescriptor->getLayout(),
        m_staticObjectRenderPass->m_renderPass,
        sizeof(glm::mat4)
    );

    AssVertexInputDescription des = AssVertex::get_vertex_description();
    VkPipelineVertexInputStateCreateInfo ci = vkinit::vertexInputStateCreateInfo();
    ci.vertexAttributeDescriptionCount = des.attributes.size();
	ci.pVertexAttributeDescriptions = des.attributes.data();
	ci.vertexBindingDescriptionCount = des.bindings.size();
	ci.pVertexBindingDescriptions = des.bindings.data();

    m_staticObjectGraphicsPipeline->setVertexInputState(ci);

    std::vector<VkDynamicState> m_dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    m_staticObjectGraphicsPipeline->setDynamicState(m_dynamicStates);
    
    m_staticObjectGraphicsPipeline->create();

    m_offscreenGraphicsPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/Assobj/offscreen.vert.spv",
        "../../shaders/Assobj/offscreen.frag.spv",
        m_offscreenDescriptor->getLayout(),
        m_renderPass->m_renderPass
    );

    VkPipelineRasterizationStateCreateInfo rsci = vkinit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
    rsci.cullMode = VK_CULL_MODE_NONE;
    m_offscreenGraphicsPipeline->setRasterizationState(rsci);

    m_offscreenGraphicsPipeline->setDynamicState(m_dynamicStates);
    m_offscreenGraphicsPipeline->create();
}

void ShadowMapping::initResource()
{
    m_model = glm::mat4(1.0);
    m_cam = VulkanContext::get()->getCamera();
    m_camUBO = std::make_shared<Buffer>(sizeof(vkinit::CameraUBO));
    m_camUBO->writeToSet(m_staticObjectDescriptor->getSet(),0);
    m_camParam = std::make_shared<vkinit::CameraUBO>();
    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1920.f / 1080.f, 0.1f, 200.0f);
	projection[1][1] *= -1;
    m_camParam->view = m_cam->GetViewMatrix();
    m_camParam->project = projection;
    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);
    m_camUBO->map(m_camParam.get());

    m_staticObjectFramebuffer->m_colorAttachments[0]->writeToSet
    (
        m_offscreenDescriptor->getSet(),
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        0
    );

    m_lightPos = glm::vec3(20,25,10);
    Factory::CreateShadowPassResource(m_lightPos,m_shadowPass,m_lightSpace);

    m_shadowPass.framebuffer->m_depth->writeToSet
    (
        m_staticObjectDescriptor->getSet(),
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        2
    );

    m_lightBuffer = std::make_shared<Buffer>(sizeof(LightParam));
    std::cout<<sizeof(LightParam);
    m_lightBuffer->writeToSet(m_staticObjectDescriptor->getSet(),1);
    m_lightParam.lightSpace = m_lightSpace;
    m_lightParam.lightPos = m_lightPos;
    m_lightBuffer->map(&m_lightParam);

}

void ShadowMapping::init() 
{
    m_scene = std::make_shared<AssScene>();
    m_scene->loadScene("../../assets/vulkanscene_shadow/vulkanscene_shadow.obj");
    //std::cout<<m_scene->m_objects.size();

    initRenderPass();
    initPipeline();
    initResource();
    buildCmd();
}

void ShadowMapping::shadowPass(VkCommandBuffer cmd)
{
    VkDeviceSize offset = 0;

    VkClearValue clearValues[2];
    VkViewport viewport;
    VkRect2D scissor;

    clearValues[0].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = m_shadowPass.renderpass->m_renderPass;
    renderPassBeginInfo.framebuffer = m_shadowPass.framebuffer->m_framebuffers[0];
    renderPassBeginInfo.renderArea.extent.width = SHADOW_DIM;
    renderPassBeginInfo.renderArea.extent.height = SHADOW_DIM;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(cmd,&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);

    viewport = vkinit::viewport((float)SHADOW_DIM, (float)SHADOW_DIM, 0.0f, 1.0f);
	vkCmdSetViewport(cmd, 0, 1, &viewport);

    scissor = vkinit::rect2D(SHADOW_DIM, SHADOW_DIM, 0, 0);
	vkCmdSetScissor(cmd, 0, 1, &scissor);
            
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadowPass.pipeline->getPipeline());
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,m_shadowPass.pipeline->getLayout(),0,1,&m_shadowPass.descriptor->getSet(),0,nullptr);
    vkCmdPushConstants(cmd,m_shadowPass.pipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(glm::mat4),&m_model);
    for(int i = 0 ;i<m_scene->m_objects.size();i++)
    {
        vkCmdBindVertexBuffers(cmd, 0, 1, &m_scene->m_objects[i]->m_mesh->m_vertexBuffer->m_buffer, &offset);
        vkCmdBindIndexBuffer(cmd, m_scene->m_objects[i]->m_mesh->m_indexBuffer->m_buffer, offset,VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd,m_scene->m_objects[i]->m_mesh->m_indices.size(),1,0,0,0);
    }
    vkCmdEndRenderPass(cmd);
}

void ShadowMapping::buildCmd() {
    for(size_t currentFrame = 0;currentFrame<MAX_IN_FLIGHT;currentFrame++)
    {
        VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkClearValue clearValues[2];
        VkViewport viewport{};
        VkRect2D scissor{};
        //float flash = std::abs(std::sin(_frameNumber / 120.f));
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f , 0};
        
        VK_CHECK(vkBeginCommandBuffer(m_mainCmdBuffers[currentFrame], &cmdBeginInfo));

        shadowPass(m_mainCmdBuffers[currentFrame]);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_staticObjectRenderPass->m_renderPass;
        renderPassInfo.framebuffer = m_staticObjectFramebuffer->m_framebuffers[currentFrame];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {VulkanContext::get()->getWidth(),VulkanContext::get()->getHeight()};
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;

        VkDeviceSize offset = 0;

        vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        viewport = vkinit::viewport((float)VulkanContext::get()->getWidth(), (float)VulkanContext::get()->getHeight(), 0.0f, 1.0f);
	    vkCmdSetViewport(m_mainCmdBuffers[currentFrame], 0, 1, &viewport);

        scissor = vkinit::rect2D(VulkanContext::get()->getWidth(), VulkanContext::get()->getHeight(), 0, 0);
        vkCmdSetScissor(m_mainCmdBuffers[currentFrame], 0, 1, &scissor);

        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_staticObjectGraphicsPipeline->getPipeline());
        vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_staticObjectGraphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(glm::mat4),&m_model);
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_staticObjectGraphicsPipeline->getLayout(),0,1,&m_staticObjectDescriptor->getSet(),0,nullptr);
        for(int i = 0 ;i<m_scene->m_objects.size();i++)
        {
            vkCmdBindVertexBuffers(m_mainCmdBuffers[currentFrame], 0, 1, &m_scene->m_objects[i]->m_mesh->m_vertexBuffer->m_buffer, &offset);
            vkCmdBindIndexBuffer(m_mainCmdBuffers[currentFrame], m_scene->m_objects[i]->m_mesh->m_indexBuffer->m_buffer, offset,VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(m_mainCmdBuffers[currentFrame],m_scene->m_objects[i]->m_mesh->m_indices.size(),1,0,0,0);
        }
        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        renderPassInfo.renderPass = m_renderPass->m_renderPass;
        renderPassInfo.framebuffer = m_framebuffer->m_framebuffers[currentFrame];
        vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_offscreenGraphicsPipeline->getPipeline());
        //vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_graphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_offscreenGraphicsPipeline->getLayout(),0,1,&m_offscreenDescriptor->getSet(),0,nullptr);

        vkCmdDraw(m_mainCmdBuffers[currentFrame],3,1,0,0);

        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));
    }
}

void ShadowMapping::tick() 
{
    uint32_t currentFrame = 0;

    auto result2 = vkAcquireNextImageKHR(VulkanContext::get()->getDevice(),VulkanContext::get()->getSwapchain().m_swapchain,UINT64_MAX,m_presentSemaphore,VK_NULL_HANDLE,&currentFrame);
    
    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &m_presentSemaphore;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_renderSemaphore;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pImageIndices = &currentFrame;
    present.swapchainCount = 1;
    present.pSwapchains = &VulkanContext::get()->getSwapchain().m_swapchain;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &m_renderSemaphore;
    auto result = vkQueuePresentKHR(VulkanContext::get()->getQueue(),&present);

}

void ShadowMapping::update(float delta)
{
    m_camParam->view = m_cam->GetViewMatrix();

    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);

    m_camUBO->map(m_camParam.get());
}
void ShadowMapping::windowResize() 
{

}

ShadowMapping::~ShadowMapping()
{
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
}

Particle::Particle(uint32_t count, uint32_t size):m_particleCount(count),m_particleSize(size){}

Particle::~Particle()
{
    vkDeviceWaitIdle(VulkanContext::get()->getDevice());
    vkDestroySemaphore(VulkanContext::get()->getDevice(),m_updateParticleComputeSemaphore,nullptr);
    vkDestroySemaphore(VulkanContext::get()->getDevice(),m_updateParticleGraphicsSemaphore,nullptr);
}

void Particle::init() 
{
    m_offscreenDescriptor = std::make_shared<Descriptor>();
    m_offscreenDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,0);
    m_offscreenDescriptor->update();

    m_staticObjectDescriptor = std::make_shared<Descriptor>();
    m_staticObjectDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
    m_staticObjectDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1);
    m_staticObjectDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2);
    m_staticObjectDescriptor->update();

    m_offscreenPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/particle/offscreen.vert.spv",
        "../../shaders/particle/offscreen.frag.spv",
        m_offscreenDescriptor->getLayout(),
        m_renderPass->m_renderPass   
    );
    m_offscreenPipeline->setCullMode(VK_CULL_MODE_NONE);
    m_offscreenPipeline->create();

    std::vector<VkFormat>formats{VK_FORMAT_R8G8B8A8_UNORM};
    m_staticObjectRenderPass = std::make_shared<RenderPass>(formats);
    m_staticObjectFramebuffer = std::make_shared<Framebuffer>(m_staticObjectRenderPass);
    m_staticObjectPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/particle/env.vert.spv",
        "../../shaders/particle/env.frag.spv",
        m_staticObjectDescriptor->getLayout(),
        m_staticObjectRenderPass->m_renderPass,
        sizeof(glm::mat4)  
    );

    AssVertexInputDescription des = AssVertex::get_vertex_description();
    VkPipelineVertexInputStateCreateInfo ci = vkinit::vertexInputStateCreateInfo();
    ci.vertexAttributeDescriptionCount = des.attributes.size();
	ci.pVertexAttributeDescriptions = des.attributes.data();
	ci.vertexBindingDescriptionCount = des.bindings.size();
	ci.pVertexBindingDescriptions = des.bindings.data();

    m_staticObjectPipeline->setVertexInputState(ci);
    // m_staticObjectPipeline->setCullMode(VK_CULL_MODE_NONE);
    m_staticObjectPipeline->create();

    initParticlePipeline();

    initResource();

    m_firePlaceTexture->writeToSet(m_staticObjectDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);
    //m_firePlaceTexture->writeToSet(m_offscreenDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,0);
    m_firePlaceNormalTexture->writeToSet(m_staticObjectDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2);

    m_fireTexture->writeToSet(m_particleDrawDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2);
    m_smokeTexture->writeToSet(m_particleDrawDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3);
    
    m_staticObjectFramebuffer->m_colorAttachments[0]->writeToSet(m_offscreenDescriptor->getSet(),VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,0);

    initParticles();
    updateParticleCmd();
    buildCmd();
}

float Particle::rnd(float range)
{
    std::uniform_real_distribution<float> rndDist(0.0f, range);
	return rndDist(rndEngine);
}

void Particle::initParticles()
{
    m_particleComputeDescriptor = std::make_shared<Descriptor>();
    m_particleComputeDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT,0);
    m_particleComputeDescriptor->bind(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT,1);
    m_particleComputeDescriptor->update();

    m_particleCount = 256;
    m_particles.resize(256);

    for(uint32_t i = 0;i<m_particleCount;i++)
    {
        m_particles[i].vel = glm::vec4(0.0f, m_envParam.minVel.y + rnd(m_envParam.maxVel.y - m_envParam.minVel.y), 0.0f, 0.0f);
		m_particles[i].alpha = rnd(0.75f);
		m_particles[i].size = 1.0f + rnd(0.5f);
		m_particles[i].color = glm::vec4(1.0f);
		m_particles[i].type = 1;
		m_particles[i].rotation = rnd(2.0f * float(M_PI));
		m_particles[i].rotationSpeed = rnd(2.0f) - rnd(2.0f);

		// Get random sphere point
		float theta = rnd(2.0f * float(M_PI));
		float phi = rnd(float(M_PI)) - float(M_PI) / 2.0f;
		float r = rnd(8.0f);

		m_particles[i].pos.x = r * cos(theta) * cos(phi);
		m_particles[i].pos.y = r * sin(phi);
		m_particles[i].pos.z = r * sin(theta) * cos(phi);
		m_particles[i].pos += glm::vec4(m_envParam.emitterPos);
    }

    std::cout<<"ParticleParam size:"<<sizeof(ParticleParam)<<std::endl;
    
    m_particleVertexBuffer = std::make_shared<Buffer>(sizeof(ParticleParam)*m_particleCount,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    m_particleVertexBuffer->map(m_particles.data());
    m_particleVertexBuffer->unmap();

    m_envParamBuffer = std::make_shared<Buffer>(sizeof(EnvParam));
    m_envParamBuffer->map(&m_envParam);

    m_particleVertexBuffer->writeToSet(m_particleComputeDescriptor->getSet(),1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    m_envParamBuffer->writeToSet(m_particleComputeDescriptor->getSet(),0);

    m_particleComputePipeline = std::make_unique<ComputePipeline>(
        "../../shaders/particle/particlecomp.comp.spv",
        m_particleComputeDescriptor->getLayout()
    );
   // m_particleComputePipeline->create();

   vkinit::allocateCmdBuffer(m_updateParticleCmd);

}

void Particle::updateParticleCmd()
{
    vkResetCommandBuffer(m_updateParticleCmd,0);
    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    
    VK_CHECK(vkBeginCommandBuffer(m_updateParticleCmd, &cmdBeginInfo));
    vkCmdBindPipeline(m_updateParticleCmd,VK_PIPELINE_BIND_POINT_COMPUTE,m_particleComputePipeline->getPipeline());
    vkCmdBindDescriptorSets(m_updateParticleCmd,VK_PIPELINE_BIND_POINT_COMPUTE,
    m_particleComputePipeline->getLayout(),0,1,&m_particleComputeDescriptor->getSet(),0,nullptr);

    vkCmdDispatch(m_updateParticleCmd, 16, 1, 1);

    VK_CHECK(vkEndCommandBuffer(m_updateParticleCmd));

    auto info = vkinit::semaphoreCreateInfo();
    VK_CHECK(vkCreateSemaphore(VulkanContext::get()->getDevice(),&info,nullptr,&m_updateParticleGraphicsSemaphore));
    VK_CHECK(vkCreateSemaphore(VulkanContext::get()->getDevice(),&info,nullptr,&m_updateParticleComputeSemaphore));
    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_updateParticleGraphicsSemaphore;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    vkQueueWaitIdle(VulkanContext::get()->getQueue());
}

void Particle::initParticlePipeline()
{
    m_particleDrawDescriptor = std::make_shared<Descriptor>();
    m_particleDrawDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);          //cam
    m_particleDrawDescriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,1);          //ubo
    m_particleDrawDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2);//fire tex
    m_particleDrawDescriptor->bind(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,3);//smoke tex
    m_particleDrawDescriptor->update();

    std::vector<VkVertexInputBindingDescription> bindings{
        vkinit::vertexInputBindingDescription(0, sizeof(ParticleParam), VK_VERTEX_INPUT_RATE_VERTEX)
    };

	std::vector<VkVertexInputAttributeDescription> attributes{
        vkinit::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT,	offsetof(ParticleParam, pos)),	    // Location 0: Position
        vkinit::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32A32_SFLOAT,	offsetof(ParticleParam, color)),	// Location 1: Color
        vkinit::vertexInputAttributeDescription(0, 2, VK_FORMAT_R32_SFLOAT,             offsetof(ParticleParam, alpha)),	// Location 2: Alpha
        vkinit::vertexInputAttributeDescription(0, 3, VK_FORMAT_R32_SFLOAT,             offsetof(ParticleParam, size)),		// Location 3: Size
        vkinit::vertexInputAttributeDescription(0, 4, VK_FORMAT_R32_SFLOAT,             offsetof(ParticleParam, rotation)),	// Location 4: Rotation
        vkinit::vertexInputAttributeDescription(0, 5, VK_FORMAT_R32_SINT,               offsetof(ParticleParam, type)),		
    };

    m_particleDrawPipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/particle/particle.vert.spv",
        "../../shaders/particle/particle.frag.spv",
        m_particleDrawDescriptor->getLayout(),
        m_staticObjectRenderPass->m_renderPass
    );

    VkPipelineVertexInputStateCreateInfo ci = vkinit::vertexInputStateCreateInfo();
    ci.vertexAttributeDescriptionCount = attributes.size();
	ci.pVertexAttributeDescriptions = attributes.data();
	ci.vertexBindingDescriptionCount = bindings.size();
	ci.pVertexBindingDescriptions = bindings.data();

    VkPipelineInputAssemblyStateCreateInfo asci = vkinit::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    m_particleDrawPipeline->setInputAssemblyState(asci);
    m_particleDrawPipeline->setVertexInputState(ci);
    m_particleDrawPipeline->setDepthStencilState(true,false);

    std::vector<VkPipelineColorBlendAttachmentState> colorblendattachments{
        vkinit::colorBlendAttachmentState(true)
    };
    m_particleDrawPipeline->setColorBlendState(colorblendattachments);
    m_particleDrawPipeline->create();
	
}

void Particle::initResource()
{
    m_cam = VulkanContext::get()->getCamera();
    m_camUBO = std::make_shared<Buffer>(sizeof(vkinit::CameraUBO));
    m_camUBO->writeToSet(m_staticObjectDescriptor->getSet(),0);
    m_camUBO->writeToSet(m_particleDrawDescriptor->getSet(),0);
    m_camParam = std::make_shared<vkinit::CameraUBO>();
    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1920.f / 1080.f, 0.1f, 200.0f);
	projection[1][1] *= -1;
    m_camParam->view = m_cam->GetViewMatrix();
    m_camParam->project = projection;
    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);
    m_camUBO->map(m_camParam.get());

    m_ubo.viewPortDim = glm::vec2(VulkanContext::get()->getWidth(),VulkanContext::get()->getHeight());
    m_uboBuffer = std::make_shared<Buffer>(sizeof(ParamUBO));
    m_uboBuffer->map(&m_ubo);
    m_uboBuffer->unmap();
    m_uboBuffer->writeToSet(m_particleDrawDescriptor->getSet(),1);

    m_firePlaceTexture = std::make_shared<Image>();
    m_firePlaceTexture->loadKTXFromFile("../../assets/particle/fireplace_colormap_rgba.ktx",VK_FORMAT_R8G8B8A8_UNORM);

    m_firePlaceNormalTexture = std::make_shared<Image>();
    m_firePlaceNormalTexture->loadKTXFromFile("../../assets/particle/fireplace_normalmap_rgba.ktx",VK_FORMAT_R8G8B8A8_UNORM);

    m_fireTexture = std::make_shared<Image>();
    m_fireTexture->loadKTXFromFile("../../assets/particle/particle_fire.ktx",VK_FORMAT_R8G8B8A8_UNORM);

    m_smokeTexture = std::make_shared<Image>();
    m_smokeTexture->loadKTXFromFile("../../assets/particle/particle_smoke.ktx",VK_FORMAT_R8G8B8A8_UNORM);

    m_scene = std::make_shared<AssScene>();
    m_scene->loadScene("../../assets/fireplace/fireplace.obj");
    m_scene->m_model = glm::scale(glm::mat4(1.0),glm::vec3(1.0,1.0,1.0));
}

void Particle::buildCmd()
{
    for(size_t currentFrame = 0;currentFrame<MAX_IN_FLIGHT;currentFrame++)
    {
        VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkClearValue clearValues[2];
        VkViewport viewport{};
        VkRect2D scissor{};
        //float flash = std::abs(std::sin(_frameNumber / 120.f));
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f , 0};
        
        VK_CHECK(vkBeginCommandBuffer(m_mainCmdBuffers[currentFrame], &cmdBeginInfo));

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {VulkanContext::get()->getWidth(),VulkanContext::get()->getHeight()};
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;

        VkDeviceSize offset = 0;

        renderPassInfo.renderPass = m_staticObjectRenderPass->m_renderPass;
        renderPassInfo.framebuffer = m_staticObjectFramebuffer->m_framebuffers[0];

        vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_staticObjectPipeline->getPipeline());
        vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_staticObjectPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(glm::mat4),&m_scene->m_model);
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_staticObjectPipeline->getLayout(),0,1,&m_staticObjectDescriptor->getSet(),0,nullptr);
        for(int i = 0 ;i<m_scene->m_objects.size();i++)
        {
            vkCmdBindVertexBuffers(m_mainCmdBuffers[currentFrame], 0, 1, &m_scene->m_objects[i]->m_mesh->m_vertexBuffer->m_buffer, &offset);
            vkCmdBindIndexBuffer(m_mainCmdBuffers[currentFrame], m_scene->m_objects[i]->m_mesh->m_indexBuffer->m_buffer, offset,VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(m_mainCmdBuffers[currentFrame],m_scene->m_objects[i]->m_mesh->m_indices.size(),1,0,0,0);
        }

        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_particleDrawPipeline->getPipeline());
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_particleDrawPipeline->getLayout(),0,1,&m_particleDrawDescriptor->getSet(),0,nullptr);
        vkCmdBindVertexBuffers(m_mainCmdBuffers[currentFrame], 0, 1, &m_particleVertexBuffer->m_buffer, &offset);
        vkCmdDraw(m_mainCmdBuffers[currentFrame],m_particleCount,1,0,0);

        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        renderPassInfo.renderPass = m_renderPass->m_renderPass;
        renderPassInfo.framebuffer = m_framebuffer->m_framebuffers[currentFrame];
        vkCmdBeginRenderPass(m_mainCmdBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_offscreenPipeline->getPipeline());
        //vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_graphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_offscreenPipeline->getLayout(),0,1,&m_offscreenDescriptor->getSet(),0,nullptr);

        vkCmdDraw(m_mainCmdBuffers[currentFrame],3,1,0,0);

        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));
    }
}

void Particle::tick() 
{
    uint32_t currentFrame = 0;

    auto result2 = vkAcquireNextImageKHR(VulkanContext::get()->getDevice(),VulkanContext::get()->getSwapchain().m_swapchain,UINT64_MAX,m_presentSemaphore,VK_NULL_HANDLE,&currentFrame);
    
    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    VkSubmitInfo windsubmit{};
    windsubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    windsubmit.commandBufferCount = 1;
    windsubmit.pCommandBuffers = &m_updateParticleCmd;
    windsubmit.waitSemaphoreCount = 1;
    windsubmit.pWaitSemaphores = &m_updateParticleGraphicsSemaphore;
    windsubmit.signalSemaphoreCount = 1;
    windsubmit.pSignalSemaphores = &m_updateParticleComputeSemaphore;
    windsubmit.pWaitDstStageMask = &waitStageMask;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&windsubmit,VK_NULL_HANDLE))

    VkSemaphore graphicsWaitSemaphores[] = { m_updateParticleComputeSemaphore, m_presentSemaphore };
    VkSemaphore graphicsSignalSemaphores[] = { m_renderSemaphore, m_updateParticleGraphicsSemaphore };
    VkPipelineStageFlags graphicsWaitStageMasks[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_mainCmdBuffers[currentFrame];
    submit.waitSemaphoreCount = 2;
    submit.pWaitSemaphores = graphicsWaitSemaphores;
    submit.signalSemaphoreCount = 2;
    submit.pSignalSemaphores = graphicsSignalSemaphores;
    submit.pWaitDstStageMask = graphicsWaitStageMasks;
    VK_CHECK(vkQueueSubmit(VulkanContext::get()->getQueue(),1,&submit,VK_NULL_HANDLE))

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pImageIndices = &currentFrame;
    present.swapchainCount = 1;
    present.pSwapchains = &VulkanContext::get()->getSwapchain().m_swapchain;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &m_renderSemaphore;
    auto result = vkQueuePresentKHR(VulkanContext::get()->getQueue(),&present);
}

void Particle::update(float delta)
{
    //m_time += delta;
    m_envParam.timeRadius.x+=delta;
    m_envParamBuffer->map(&m_camParam);

    m_camParam->view = m_cam->GetViewMatrix();

    m_camParam->camPos = glm::vec4(m_cam->Position,0.0f);

    m_camUBO->map(m_camParam.get());

    // m_lightubo.pos.x = 5*cos(m_time/(M_PI));
    // m_lightubo.pos.z = 5*sin(m_time/(M_PI));
    //m_lightBuffer->map(&m_lightubo);
}

void Particle::windowResize()
{

}