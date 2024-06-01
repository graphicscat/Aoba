#include<engine/ui.h>
#include<util/vkinit.h>
//std::unique_ptr<ImGuiLayer>ImGuiLayer::m_ImGuiLayer = nullptr;

ImGuiLayer* ImGuiLayer::get()
{
   static ImGuiLayer layer;
    return &layer;
}

void ImGuiLayer::init(VkRenderPass renderPass)
{
     VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        //VkDescriptorPool imguiPool;
        VK_CHECK(vkCreateDescriptorPool(VulkanContext::get()->getDevice(), &pool_info, nullptr, &imguiPool));


        // 2: initialize imgui library

        //this initializes the core structures of imgui
        IMGUI_CHECKVERSION();
        std::cout<<IMGUI_VERSION;
        ImGui::CreateContext();

        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard ;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImFont* font = io.Fonts->AddFontFromFileTTF("../../assets/font/Roboto-Medium.ttf", 26.0f);
        IM_ASSERT(font != nullptr);

        //this initializes imgui for SDL
        ImGui_ImplGlfw_InitForVulkan(VulkanContext::get()->getWindow(),true);

        //this initializes imgui for Vulkan
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = VulkanContext::get()->getInstance();
        init_info.PhysicalDevice = VulkanContext::get()->getGPU();;
        init_info.Device = VulkanContext::get()->getDevice();
        init_info.Queue = VulkanContext::get()->getQueue();
        init_info.DescriptorPool = imguiPool;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 2;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.RenderPass = renderPass;

        ImGui_ImplVulkan_Init(&init_info);

        //execute a gpu command to upload imgui font textures
        //  VkCommandBuffer cmd = {};
        //  vkinit::allocateCmdBuffer(cmd,true);
        //  ImGui_ImplVulkan_CreateFontsTexture(cmd);
        //  vkinit::flushCmdBuffer(cmd);
        
        // io.Fonts->Build();

        //clear font textures from cpu data
        //ImGui_ImplVulkan_DestroyFontUploadObjects();
    
}

void ImGuiLayer::release()
{
    ImGui::SaveIniSettingsToDisk("../../assets/ui/imgui.ini");
	ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(VulkanContext::get()->getDevice(), imguiPool, nullptr);
}