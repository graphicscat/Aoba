#include<engine/renderpass.h>

#include<pch.h>

RenderPass::RenderPass(const std::vector<VkFormat>& formats,bool hasdepth)
{
    m_hasDepth = hasdepth;
    m_isPresent = false;
    size_t n = formats.size();
    m_colAttachDescriptions.resize(n);

    for(int i = 0 ;i<n;i++)
    {
        m_colAttachDescriptions[i] = vkinit::createAttachmentDescription(formats[i],false);
    }

    m_formats = formats;

    init();
}

std::vector<VkFormat> RenderPass::getFormats()
{
    return m_formats;
}

void RenderPass::init()
{

    std::vector<VkAttachmentReference> attachRefs;
    attachRefs.resize(m_colAttachDescriptions.size());
    for(int i = 0;i<attachRefs.size();i++)
    {   
        attachRefs[i].attachment = i;
        attachRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentDescription depth{};
    depth = vkinit::createAttachmentDescription(VK_FORMAT_D16_UNORM,true,true);

    VkAttachmentReference depthAttachRef{};
    depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachRef.attachment = static_cast<uint32_t>(attachRefs.size());

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(attachRefs.size());
    subpass.pColorAttachments = attachRefs.data();
    if(m_hasDepth)
    subpass.pDepthStencilAttachment = &depthAttachRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency depth_dependency = {};
    depth_dependency.srcSubpass = 0;
    depth_dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    depth_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    depth_dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    depth_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    depth_dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    depth_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::vector<VkAttachmentDescription> attachments = m_colAttachDescriptions;
    if(m_hasDepth)
    attachments.push_back(depth);
    std::vector<VkSubpassDependency> dependencies = { dependency,depth_dependency};
    VkRenderPassCreateInfo renderpassInfo{};
    renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassInfo.attachmentCount = uint32_t(attachments.size());
    renderpassInfo.pAttachments = attachments.data();
    renderpassInfo.dependencyCount = uint32_t(dependencies.size());
    renderpassInfo.pDependencies = dependencies.data();
    renderpassInfo.subpassCount = 1;
    renderpassInfo.pSubpasses = &subpass;

    VK_CHECK(vkCreateRenderPass(VulkanContext::get()->getDevice(),&renderpassInfo,nullptr,&m_renderPass));
    LOG_TRACE("Init RenderPass Success");

}

bool RenderPass::hasDepth()
{
    return m_hasDepth;
}

bool RenderPass::isPresent()
{
    return m_isPresent;
}
void RenderPass::getPresentRenderPass()
{
    m_isPresent = true;

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
    depth_attachment.format = VK_FORMAT_D16_UNORM;
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