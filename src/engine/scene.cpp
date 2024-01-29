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

        vkCmdBindPipeline(m_mainCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->getPipeline());
        vkCmdPushConstants(m_mainCmdBuffers[currentFrame],m_graphicsPipeline->getLayout(),VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstantData),&m_pcd);
        vkCmdBindDescriptorSets(m_mainCmdBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,m_graphicsPipeline->getLayout(),0,1,&m_descriptor->getSet(),0,nullptr);

        vkCmdDraw(m_mainCmdBuffers[currentFrame],3,1,0,0);

        vkCmdEndRenderPass(m_mainCmdBuffers[currentFrame]);

        VK_CHECK(vkEndCommandBuffer(m_mainCmdBuffers[currentFrame]));
    }
}
void ShaderToy::tick()
{
    // uint32_t currentFrame = m_frameNumber % 2;
    uint32_t currentFrame = 0;
	// VK_CHECK(vkWaitForFences(VulkanContext::get()->getDevice(),1,&m_fences[currentFrame],VK_TRUE,UINT64_MAX));
	// VK_CHECK(vkResetFences(VulkanContext::get()->getDevice(),1,&m_fences[currentFrame]));
    VK_CHECK(vkAcquireNextImageKHR(VulkanContext::get()->getDevice(),VulkanContext::get()->getSwapchain().m_swapchain,UINT64_MAX,m_presentSemaphore,VK_NULL_HANDLE,&currentFrame))

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
    VK_CHECK(vkQueuePresentKHR(VulkanContext::get()->getQueue(),&present))
	//m_frameNumber ++;
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