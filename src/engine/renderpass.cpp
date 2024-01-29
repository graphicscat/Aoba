#include<engine/renderpass.h>

#include<pch.h>

void RenderPass::init(const rpBuilder& builder)
{

}

void RenderPass::getPresentRenderPass()
{
    VkAttachmentDescription color_attachment{};
    color_attachment.format = VulkanContext::get()->m_swapchain.m_swapchainImageFormat;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment{};
    //TODO
    depth_attachment.format = VK_FORMAT_D32_SFLOAT;
    depth_attachment.flags = 0;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //msaa
    // VkAttachmentDescription color_attachmentResolve{};
    // color_attachmentResolve.format = VulkanContext::get()->m_swapchain.m_swapchainImageFormat;
    // color_attachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // color_attachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    // color_attachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // color_attachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // color_attachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // color_attachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // color_attachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachmentResolve_ref{};
    color_attachmentResolve_ref.attachment = 2;
    color_attachmentResolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;
    //subpass.pResolveAttachments = &color_attachmentResolve_ref;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency depth_dependency = {};
    depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    depth_dependency.dstSubpass = 0;
    depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depth_dependency.srcAccessMask = 0;
    depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments = {color_attachment,depth_attachment/*,color_attachmentResolve*/};
    std::vector<VkSubpassDependency> dependencies = { dependency,depth_dependency};
    VkRenderPassCreateInfo renderpassInfo{};
    renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassInfo.attachmentCount = uint32_t(attachments.size());
    renderpassInfo.pAttachments = attachments.data();
    renderpassInfo.dependencyCount = uint32_t(dependencies.size());
    renderpassInfo.pDependencies = dependencies.data();
    renderpassInfo.subpassCount = 1;
    renderpassInfo.pSubpasses = &subpass;

    VK_CHECK(vkCreateRenderPass(VulkanContext::get()->getDevice(),&renderpassInfo,nullptr,&m_renderPass))
    LOG_TRACE("Init RenderPass Success");
}

void RenderPass::release()
{
    vkDestroyRenderPass(VulkanContext::get()->getDevice(),m_renderPass,nullptr);
    LOG_WARN("RenderPass delete");
}

RenderPass::~RenderPass()
{
    release();
}