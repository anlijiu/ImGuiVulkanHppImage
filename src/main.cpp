#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <format>
#include <stdexcept>
#include <vulkan/vulkan_beta.h>
#include <vulkan/vulkan.hpp>

#include "Scene.h"
#include "VulkanContext.h"
#include "Window.h"

// #define IMGUI_UNLIMITED_FRAME_RATE
#define  VK_ENABLE_BETA_EXTENSIONS 1

static ImGui_ImplVulkanH_Window MainWindowData;
static uint MinImageCount = 2;
static bool SwapChainRebuild = false;

static WindowsState Windows;
static std::unique_ptr<VulkanContext> VC;
static std::unique_ptr<Scene> MainScene;
static vk::DescriptorSet MainSceneDescriptorSet;

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void SetupVulkanWindow(ImGui_ImplVulkanH_Window *wd, vk::SurfaceKHR surface, int width, int height) {
    wd->Surface = surface;

    /**
     * 由于vulkan无法和窗口直接交互， 需要使用 Windows System Integration 扩展， 简称WSI
     * 拿到 VkSurfaceKHR 就可将 image 呈现在 surface 上了
     */
    // Check for WSI support
    auto res = VC->PhysicalDevice.getSurfaceSupportKHR(
        VC->QueueFamily,
        wd->Surface
    );
    if (res != VK_TRUE) throw std::runtime_error("Error no WSI support on physical device 0\n");

    // Select surface format.
    const VkFormat requestSurfaceImageFormat[] = {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8_UNORM,
        VK_FORMAT_R8G8B8_UNORM
    };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    // VkSurfaceFormatKHR // 表面格式（像素格式、颜色空间）  06_swap_chain_creation.cpp :: chooseSwapSurfaceFormat
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
        VC->PhysicalDevice,
        wd->Surface,
        requestSurfaceImageFormat,
        (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
        requestSurfaceColorSpace
    );

    // Select present mode.呈现模式. 06_swap_chain_creation.cpp :: chooseSwapPresentMode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = {
        VK_PRESENT_MODE_MAILBOX_KHR,  // 与FIFO的区别就是 当队列已满时，
                                      // 应用程序不会阻塞，而是直接用较新的图像替换已排队的图像。
        VK_PRESENT_MODE_IMMEDIATE_KHR,// 提交的图像会立即传输到屏幕上，这可能会导致撕裂
        VK_PRESENT_MODE_FIFO_KHR,     // 就把这个当做垂直同步
    };
#else
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
        VC->PhysicalDevice,
        wd->Surface,
        &present_modes[0],
        IM_ARRAYSIZE(present_modes)
    );
    // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(MinImageCount >= 2);
    // 建立交换链，渲染过程，图像视图，帧缓冲
    ImGui_ImplVulkanH_CreateOrResizeWindow(
        VC->Instance.get(),
        VC->PhysicalDevice,
        VC->Device.get(),
        wd,
        VC->QueueFamily,
        nullptr,
        width,
        height,
        MinImageCount
    );
}

static void CleanupVulkanWindow() {
    ImGui_ImplVulkanH_DestroyWindow(VC->Instance.get(), VC->Device.get(), &MainWindowData, nullptr);
}

static void FrameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data) {
    // Semaphores 信号量  ----------  15_hello_triangle.cpp :: drawFrame
    // 信号量用于同步队列queue。
    // 信号量用于GPU操作。  block GPU
    VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;

    // ImGui_ImplVulkanH_Frame 包含渲染一帧的各种必需的vk 资源
    // VkCommandPool VkCommandBuffer VkFence VkImage VkImageView VkFramebuffer  
    // FrameIndex 当前帧 index
    ImGui_ImplVulkanH_Frame *fd = &wd->Frames[wd->FrameIndex];
    {
        // 15_hello_triangle.cpp :: drawFrame
        // 等到前一帧完成
        CheckVk(vkWaitForFences(VC->Device.get(), 1, &fd->Fence, VK_TRUE, UINT64_MAX)); // wait indefinitely instead of periodically checking
        // 重置为无信号状态
        CheckVk(vkResetFences(VC->Device.get(), 1, &fd->Fence));
    }

    // 从交换链获取图像。
    const VkResult err = vkAcquireNextImageKHR(
        VC->Device.get(),
        wd->Swapchain,
        UINT64_MAX,
        image_acquired_semaphore,
        VK_NULL_HANDLE,
        &wd->FrameIndex
    );

    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        SwapChainRebuild = true;
        return;
    }
    CheckVk(err);

    {
        // reset -> 所有已从命令池分配的命令缓冲区都将处于初始状态。
        // 任何从另一个 VkCommandPool 分配的主命令缓冲区 处于记录或可执行状态的，
        // 并且记录到主命令缓冲区的,  从 commandPool 分配的辅助命令缓冲区，都将变为无效。
        CheckVk(vkResetCommandPool(VC->Device.get(), fd->CommandPool, 0));
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        // 开始记录命令缓冲区
        CheckVk(vkBeginCommandBuffer(fd->CommandBuffer, &info));
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;// 别废话， 必须是这个
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        // 开始渲染过程
        vkCmdBeginRenderPass(
            fd->CommandBuffer, 
            &info, 
            VK_SUBPASS_CONTENTS_INLINE
        );
    }

    // Record dear imgui primitives into command buffer
    // 将imgui ImDrawData 转化为 vkCommandBuffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    // 结束渲染过程
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        // 完成命令缓冲区的记录  
        CheckVk(vkEndCommandBuffer(fd->CommandBuffer));

        // vkQueueSubmit 调用会立即返回 - 等待仅发生在 GPU 上
        CheckVk(vkQueueSubmit(VC->Queue, 1, &info, fd->Fence));
    }
}

static void FramePresent(ImGui_ImplVulkanH_Window *wd) {
    if (SwapChainRebuild) return;

    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    // vkQueuePresentKHR 函数向交换链提交显示图像的请求。
    VkResult err = vkQueuePresentKHR(VC->Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        SwapChainRebuild = true;
        return;
    }
    CheckVk(err);
    // ImageCount 基本上就是 MinImageCount (2)
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores.
}

using namespace ImGui;

static vk::ClearColorValue ImVec4ToClearColor(const ImVec4 &v) {
    return {
        v.x,
        v.y,
        v.z,
        v.w
    };
}

int main(int, char **) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        throw std::runtime_error(std::format("SDL_Init error: {}", SDL_GetError()));
    }

    // SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    // Create window with Vulkan graphics context.
    const auto window_flags =
        SDL_WINDOW_VULKAN
        | SDL_WINDOW_RESIZABLE
        | SDL_WINDOW_MAXIMIZED
        | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    auto *Window = SDL_CreateWindow(
        "Mesh2Audio-Vulkan",
        1280, 
        720, 
        window_flags
    );

    uint extensions_count = 0;
    const auto extensionsPtr = SDL_Vulkan_GetInstanceExtensions(
        &extensions_count
    );
    std::vector<const char *> extensions(extensions_count);
    std::copy(extensionsPtr, extensionsPtr + extensions_count, extensions.begin());

    // 创建上下文 VulkanContext
    VC = std::make_unique<VulkanContext>(extensions);

    // Create window surface.
    VkSurfaceKHR surface;
    if (SDL_Vulkan_CreateSurface(Window, VC->Instance.get(), nullptr, &surface) == 0) throw std::runtime_error("Failed to create Vulkan surface.\n");

    // Create framebuffers.
    int w, h;
    SDL_GetWindowSize(Window, &w, &h);
    ImGui_ImplVulkanH_Window *wd = &MainWindowData;
    SetupVulkanWindow(wd, surface, w, h);

    // Setup ImGui context. ImGui上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

    io.IniFilename = nullptr; // Disable ImGui's .ini file saving

    // 设置样式中各种颜色
    StyleColorsDark();
    // StyleColorsLight();

    // Setup Platform/Renderer backends  
    ImGui_ImplSDL3_InitForVulkan(Window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = VC->Instance.get();
    init_info.PhysicalDevice = VC->PhysicalDevice;
    init_info.Device = VC->Device.get();
    init_info.QueueFamily = VC->QueueFamily;
    init_info.Queue = VC->Queue;
    init_info.PipelineCache = VC->PipelineCache.get();
    init_info.DescriptorPool = VC->DescriptorPool.get();
    init_info.Subpass = 0;
    init_info.MinImageCount = MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = CheckVk;
    init_info.RenderPass = wd->RenderPass;
    ImGui_ImplVulkan_Init(&init_info);

    // Load fonts.
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != nullptr);

    // Upload fonts.
    {
        // Use any command queue
        vk::CommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
        vk::CommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;
        VC->Device->resetCommandPool(command_pool, vk::CommandPoolResetFlags());
        command_buffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        ImGui_ImplVulkan_CreateFontsTexture();
        command_buffer.end();

        vk::SubmitInfo submit;
        submit.setCommandBuffers(command_buffer);
        VC->Queue.submit(submit);//vkQueueSubmit
        VC->Device->waitIdle();
    }

    MainScene = std::make_unique<Scene>(*VC);

    // Main loop
    bool done = false;
    while (!done) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(Window))
                done = true;
        }

        int width, height;
        SDL_GetWindowSize(Window, &width, &height);
        // Resize swap chain?
        if (width > 0 && height > 0 && (SwapChainRebuild || MainWindowData.Width != width || MainWindowData.Height != height)) {
            if (width > 0 && height > 0) {
                ImGui_ImplVulkan_SetMinImageCount(MinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(
                    VC->Instance.get(),
                    VC->PhysicalDevice,
                    VC->Device.get(),
                    &MainWindowData,
                    VC->QueueFamily,
                    nullptr,
                    width,
                    height,
                    MinImageCount
                );
                MainWindowData.FrameIndex = 0;
                SwapChainRebuild = false;
            }
        }

        // Start the ImGui frame.
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        NewFrame();

        auto dockspace_id = DockSpaceOverViewport(
            0,
            ImGui::GetMainViewport(),
            ImGuiDockNodeFlags_PassthruCentralNode
        );
        if (GetFrameCount() == 1) {
            auto demo_node_id = DockBuilderSplitNode(
                dockspace_id,
                ImGuiDir_Right,
                0.3f,
                nullptr,
                &dockspace_id
            );
            DockBuilderDockWindow(Windows.ImGuiDemo.Name, demo_node_id);
            auto mesh_node_id = dockspace_id;
            auto controls_node_id = DockBuilderSplitNode(
                mesh_node_id,
                ImGuiDir_Left,
                0.4f,
                nullptr,
                &mesh_node_id
            );
            DockBuilderDockWindow(Windows.SceneControls.Name, controls_node_id);
            DockBuilderDockWindow(Windows.Scene.Name, mesh_node_id);
        }

        if (Windows.ImGuiDemo.Visible) ShowDemoWindow(&Windows.ImGuiDemo.Visible);

        if (Windows.Scene.Visible) {
            PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
            Begin(Windows.Scene.Name, &Windows.Scene.Visible);
            const auto content_region = GetContentRegionAvail();
            bool ret_render = MainScene->Render(
                content_region.x,
                content_region.y,
                ImVec4ToClearColor(GetStyleColorVec4(ImGuiCol_WindowBg))
            );
            if (ret_render) {
                ImGui_ImplVulkan_RemoveTexture(MainSceneDescriptorSet);
                MainSceneDescriptorSet = ImGui_ImplVulkan_AddTexture(
                    MainScene->TC.TextureSampler.get(),
                    MainScene->TC.ResolveImageView.get(),
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                );
            }

            ImGui::Image((ImTextureID)(void*)MainSceneDescriptorSet, ImGui::GetContentRegionAvail());
            End();
            PopStyleVar();
        }

        // Rendering
        ImGui::Render();
        ImDrawData *draw_data = GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized) {
            static const ImVec4 clear_color{0.45f, 0.55f, 0.60f, 1.f};
            wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            wd->ClearValue.color.float32[3] = clear_color.w;
            FrameRender(wd, draw_data);
            FramePresent(wd);
        }
    }

    // Cleanup
    VC->Device->waitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    DestroyContext();

    CleanupVulkanWindow();
    MainScene.reset();
    VC.reset();

    SDL_DestroyWindow(Window);
    SDL_Quit();

    return 0;
}
