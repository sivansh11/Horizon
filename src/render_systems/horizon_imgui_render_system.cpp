#include "render_systems/horizon_imgui_render_system.h"


namespace horizon {

static void check_vk_result(VkResult err) {
    if (err == 0) return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0) abort();
}

ImGuiRenderSystem::ImGuiRenderSystem(Window& window, gfx::Device& device, gfx::Renderer& renderer) : mDevice(device), mWindow(window) {
    mDescriptorPool = gfx::DescriptorPool::Builder(device)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
                        .addFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                        .setMaxSets(1000 * 11)
                        .build();    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();   
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = device.getInstance();
    initInfo.PhysicalDevice = device.getPhysicalDevice();
    initInfo.Device = device.getDevice();
    initInfo.QueueFamily = device.getPhysicalDeviceQueueFamilies().graphicsFamily;
    initInfo.Queue = device.getGraphicsQueue();

    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = mDescriptorPool->getDescriptorPool();

    initInfo.Allocator = VK_NULL_HANDLE;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = static_cast<uint32_t>(renderer.getSwapChainImageCount());
    initInfo.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&initInfo, renderer.getSwapChainRenderPass());

    auto commandBuffer = device.getSingleUseCommandBuffer();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    device.endSingleUseCommandBuffer(commandBuffer);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

ImGuiRenderSystem::~ImGuiRenderSystem() {
    mDescriptorPool = nullptr;
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();    
}

void ImGuiRenderSystem::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiRenderSystem::render(VkCommandBuffer commandBuffer) {
    ImGui::Render();
    ImDrawData *drawdata = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(drawdata, commandBuffer);
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}

} // namespace horizon

