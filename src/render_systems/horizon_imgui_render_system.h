#ifndef RENDER_SYSTEMS_HORIZON_IMGUI_H
#define RENDER_SYSTEMS_HORIZON_IMGUI_H

#include "gfx/horizon_device.h"
#include "gfx/horizon_descriptor.h"
#include "gfx/horizon_renderer.h"
#include "core/horizon_window.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace horizon {

class ImGuiRenderSystem {
public:
    ImGuiRenderSystem(Window& window, gfx::Device& device, gfx::Renderer& renderer);
    ~ImGuiRenderSystem();

    void newFrame();
    void render(VkCommandBuffer commandBuffer);

private:
    void frameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
    void framePresent(ImGui_ImplVulkanH_Window* wd);

private:
    gfx::Device& mDevice;
    Window& mWindow;
    std::unique_ptr<gfx::DescriptorPool> mDescriptorPool;
};

} // namespace horizon


#endif