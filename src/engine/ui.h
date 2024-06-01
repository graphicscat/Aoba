#pragma once
#include <imgui/imgui.h>
#include<imgui/backends/imgui_impl_glfw.h>
#include<imgui/backends/imgui_impl_vulkan.h>
#include<macro.h>
#include<pch.h>


class ImGuiLayer
{
    public:
    VkDescriptorPool imguiPool;
    void init(VkRenderPass renderPass);
    void release();
    static ImGuiLayer* get();
    private:
    ImGuiLayer() = default;

   
};