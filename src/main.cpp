
#include <vulkan/vulkan.h>

// #define VK_NO_PROTOTYPES
#define VOLK_IMPLEMENTATION
#include "volk.h"

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_DEBUG_LOG
#include "vk_mem_alloc.h"

#include "VkBootstrap.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <random>
#include <stdio.h>
#include <print>
#include <fmt/format.h>

#include <vulkan/vk_enum_string_helper.h>

#include <tracy/TracyVulkan.hpp>

#include "Init.h"
#include "Util.h"
#include "Color.h"
#include "GPUImage.h"

#define VK_CHECK(call)                 \
    do {                               \
        VkResult result_ = call;       \
        assert(result_ == VK_SUCCESS); \
    } while (0)

const int WIDTH = 800;
const int HEIGHT = 600;
const int NUM_RECTANGLES = 500;

inline constexpr std::uint32_t FRAME_OVERLAP = 2;
static constexpr auto NO_TIMEOUT = std::numeric_limits<std::uint64_t>::max();

const char* appName = "test";

struct Version {
    uint32_t major{0};
    uint32_t minor{0};
    uint32_t patch{0};

    std::string toString(bool addV = true) const
    {
        return fmt::format("{}{}.{}.{}", addV ? "v" : "", major, minor, patch);
    }
};
Version appVersion = {0, 1, 0};

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

struct InstanceData {
    glm::vec2 position;
    glm::vec2 scale;
};

struct GPUBuffer {
    VkBuffer buffer{VK_NULL_HANDLE};
    VmaAllocation allocation;
    VmaAllocationInfo info;

    // Only for buffers created with VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
    // TODO: add check that this is not 0 if requesting address?
    VkDeviceAddress address{0};
};

class Swapchain {
public:
    struct FrameData {
        VkSemaphore swapchainSemaphore;
        VkSemaphore renderSemaphore;
        VkFence renderFence;
    };

    void initSyncStructures(VkDevice device)
    {
        const auto fenceCreateInfo = VkFenceCreateInfo {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        const auto semaphoreCreateInfo = VkSemaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        for (std::uint32_t i = 0; i < FRAME_OVERLAP; ++i) {
            VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].renderFence));
            VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].swapchainSemaphore));
            VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore));
        }
    }
    void create(
        const vkb::Device& device,
        VkFormat swapchainFormat,
        std::uint32_t width,
        std::uint32_t height,
        bool vSync)
    {
        assert(swapchainFormat == VK_FORMAT_B8G8R8A8_SRGB && "TODO: test other formats");
        // vSync = false;

        auto res = vkb::SwapchainBuilder{device}
        .set_desired_format(VkSurfaceFormatKHR{
                .format = swapchainFormat,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                })
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .set_desired_present_mode(
                    vSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR)
            .set_desired_extent(width, height)
            .build();
        if (!res.has_value()) {
            throw std::runtime_error(fmt::format(
                        "failed to create swapchain: error = {}, vk result = {}",
                        res.full_error().type.message(),
                        string_VkResult(res.full_error().vk_result)));
        }
        swapchain = res.value();

        images = swapchain.get_images().value();
        imageViews = swapchain.get_image_views().value();

        // TODO: if re-creation of swapchain is supported, don't forget to call
        // vkutil::initSwapchainViews here.
    }
    void recreate(
        const vkb::Device& device,
        VkFormat swapchainFormat,
        std::uint32_t width,
        std::uint32_t height,
        bool vSync)
    {
        assert(swapchain);

        assert(swapchainFormat == VK_FORMAT_B8G8R8A8_SRGB && "TODO: test other formats");
        auto res = vkb::SwapchainBuilder{device}
        .set_old_swapchain(swapchain)
            .set_desired_format(VkSurfaceFormatKHR{
                    .format = swapchainFormat,
                    .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                    })
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .set_desired_present_mode(
                    vSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR)
            .set_desired_extent(width, height)
            .build();
        if (!res.has_value()) {
            throw std::runtime_error(fmt::format(
                        "failed to create swapchain: error = {}, vk result = {}",
                        res.full_error().type.message(),
                        string_VkResult(res.full_error().vk_result)));
        }
        vkb::destroy_swapchain(swapchain);

        for (auto imageView : imageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        swapchain = res.value();

        images = swapchain.get_images().value();
        imageViews = swapchain.get_image_views().value();

        dirty = false;
    }

    void cleanup(VkDevice device)
    {
        for (auto& frame : frames) {
            vkDestroyFence(device, frame.renderFence, nullptr);
            vkDestroySemaphore(device, frame.swapchainSemaphore, nullptr);
            vkDestroySemaphore(device, frame.renderSemaphore, nullptr);
        }

        { // destroy swapchain and its views
            for (auto imageView : imageViews) {
                vkDestroyImageView(device, imageView, nullptr);
            }
            imageViews.clear();

            vkb::destroy_swapchain(swapchain);
        }
    }

    VkExtent2D getExtent() const { return swapchain.extent; }

    const std::vector<VkImage>& getImages() { return images; };

    void beginFrame(VkDevice device, std::size_t frameIndex) const
    {
        auto& frame = frames[frameIndex];
        VK_CHECK(vkWaitForFences(device, 1, &frame.renderFence, true, NO_TIMEOUT));
    }

    void resetFences(VkDevice device, std::size_t frameIndex) const
    {
        auto& frame = frames[frameIndex];
        VK_CHECK(vkResetFences(device, 1, &frame.renderFence));
    }

    // returns the image and its index
    std::pair<VkImage, std::uint32_t> acquireImage(VkDevice device, std::size_t frameIndex)
    {
        std::uint32_t swapchainImageIndex{};
        const auto result = vkAcquireNextImageKHR(
                device,
                swapchain,
                NO_TIMEOUT,
                frames[frameIndex].swapchainSemaphore,
                VK_NULL_HANDLE,
                &swapchainImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            dirty = true;
            return {images[swapchainImageIndex], swapchainImageIndex};
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        return {images[swapchainImageIndex], swapchainImageIndex};
    }

    void submitAndPresent(
        VkCommandBuffer cmd,
        VkQueue graphicsQueue,
        std::size_t frameIndex,
        std::uint32_t swapchainImageIndex)
    {
        const auto& frame = frames[frameIndex];

        { // submit
            const auto submitInfo = VkCommandBufferSubmitInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                    .commandBuffer = cmd,
            };
            const auto waitInfo = vkinit::semaphoreSubmitInfo(
                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame.swapchainSemaphore);
            const auto signalInfo = vkinit::
                semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame.renderSemaphore);

            const auto submit = vkinit::submitInfo(&submitInfo, &waitInfo, &signalInfo);
            VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submit, frame.renderFence));
        }

        { // present
            const auto presentInfo = VkPresentInfoKHR{
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &frame.renderSemaphore,
                    .swapchainCount = 1,
                    .pSwapchains = &swapchain.swapchain,
                    .pImageIndices = &swapchainImageIndex,
            };

            auto res = vkQueuePresentKHR(graphicsQueue, &presentInfo);
            if (res != VK_SUCCESS) {
                if (res != VK_SUBOPTIMAL_KHR) {
                    fmt::println("failed to present: {}", string_VkResult(res));
                }
                dirty = true;
            }
        }
    }

    VkImageView getImageView(std::size_t swapchainImageIndex)
    {
        return imageViews[swapchainImageIndex];
    }

    bool needsRecreation() const { return dirty; }
private:

    //飞行帧
    std::array<FrameData, FRAME_OVERLAP> frames;
    vkb::Swapchain swapchain;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    bool dirty{false};
};

using ImageId = std::uint32_t;
static const auto NULL_IMAGE_ID = std::numeric_limits<std::uint32_t>::max();

class VulkanApp {
public:
    struct FrameData {
        VkCommandPool commandPool;
        VkCommandBuffer mainCommandBuffer;
        TracyVkCtx tracyVkCtx;
    };

    struct EndFrameProps {
        const LinearColor clearColor{0.f, 0.f, 0.f, 1.f};
        bool copyImageIntoSwapchain{true};
        glm::ivec4 drawImageBlitRect{}; // where to blit draw image to
        bool drawImageLinearBlit{true}; // if false - nearest filter will be used
        bool drawImGui{true};
    };

    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    vkb::Instance instance;
    vkb::PhysicalDevice physicalDevice;
    vkb::Device device;
    VmaAllocator allocator;

    std::uint32_t graphicsQueueFamily;
    VkQueue graphicsQueue;

    VkSurfaceKHR surface;
    VkFormat swapchainFormat;
    Swapchain swapchain;

    std::array<FrameData, FRAME_OVERLAP> frames{};
    std::uint32_t frameNumber{0};

    VkSampleCountFlagBits supportedSampleCounts;
    VkSampleCountFlagBits highestSupportedSamples{VK_SAMPLE_COUNT_1_BIT};
    float maxSamplerAnisotropy{1.f};

    bool vSync{true};


    ImageId gameScreenDrawImageId{NULL_IMAGE_ID}; // image to which game pixels are drawn to
    ImageId finalDrawImageId{NULL_IMAGE_ID}; // id of image which is drawn to the window

    VkFormat drawImageFormat{VK_FORMAT_R16G16B16A16_SFLOAT};

    // VkCommandPool commandPool;
    // VkCommandBuffer commandBuffer;
    // VkPipeline pipeline;
    // VkPipelineLayout pipelineLayout;
    // VkRenderPass renderPass;
    // VkBuffer vertexBuffer, instanceBuffer;
    // VmaAllocator allocator;
    // VmaAllocation vertexBufferAlloc, instanceBufferAlloc;
    // VkDescriptorSetLayout descriptorSetLayout;
    // VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // VkFormat swapchainImageFormat;
    // VkExtent2D swapchainExtent;
    // VkSwapchainKHR swapchain;
    // std::vector<VkImageView> swapchainImageViews;
    // std::vector<VkFramebuffer> swapchainFramebuffers;
    // std::vector<VkCommandBuffer> commandBuffers;
    // VkSemaphore imageAvailableSemaphore, renderFinishedSemaphore;
    // VkFence inFlightFence;

    // bool g_SwapChainRebuild;
    // ImGui_ImplVulkanH_Window g_MainWindowData;
     
     struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;
    };

    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, 
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},  
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},  
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}}  
    };

    std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

    std::vector<InstanceData> instances;


    VkExtent2D getSwapchainExtent() const { return swapchain.getExtent(); }


    void checkDeviceCapabilities()
    {
        // check limits
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(physicalDevice, &props);

        maxSamplerAnisotropy = props.limits.maxSamplerAnisotropy;

        { // store which sampling counts HW supports
            const auto counts = std::array{
                VK_SAMPLE_COUNT_1_BIT,
                    VK_SAMPLE_COUNT_2_BIT,
                    VK_SAMPLE_COUNT_4_BIT,
                    VK_SAMPLE_COUNT_8_BIT,
                    VK_SAMPLE_COUNT_16_BIT,
                    VK_SAMPLE_COUNT_32_BIT,
                    VK_SAMPLE_COUNT_64_BIT,
            };

            const auto supportedByDepthAndColor =
                props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts;
            supportedSampleCounts = {};
            for (const auto& count : counts) {
                if (supportedByDepthAndColor & count) {
                    supportedSampleCounts = (VkSampleCountFlagBits)(supportedSampleCounts | count);
                    highestSupportedSamples = count;
                }
            }
        }
    }

    int initWindow() {
        glfwInit();

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Sprites", nullptr, nullptr);
        return 0;
    }

    int initVulkan() {

        VK_CHECK(volkInitialize());

        instance = vkb::InstanceBuilder{}
        .set_app_name(appName)
            .set_app_version(appVersion.major, appVersion.major, appVersion.patch)
            .request_validation_layers()
            .use_default_debug_messenger()
            .require_api_version(1, 3, 0)
            .build()
            .value();

        volkLoadInstance(instance);

        VkResult err = glfwCreateWindowSurface(instance, window, nullptr, &surface);

        if (err) {
            const char* error_msg;
            int ret = glfwGetError(&error_msg);
            if (ret != 0) {
                std::cout << ret << " ";
                if (error_msg != nullptr) std::cout << error_msg;
                std::cout << "\n";
            }
            surface = VK_NULL_HANDLE;
            std::exit(1);
        }
        
        const auto deviceFeatures = VkPhysicalDeviceFeatures{
            .imageCubeArray = VK_TRUE,
            .geometryShader = VK_TRUE, // for im3d
            .depthClamp = VK_TRUE,
            .samplerAnisotropy = VK_TRUE,
        };
        
        const auto features12 = VkPhysicalDeviceVulkan12Features{
            .descriptorIndexing = true,
            .descriptorBindingSampledImageUpdateAfterBind = true,
            .descriptorBindingStorageImageUpdateAfterBind = true,
            .descriptorBindingPartiallyBound = true,
            .descriptorBindingVariableDescriptorCount = true,
            .runtimeDescriptorArray = true,
            .scalarBlockLayout = true,
            .bufferDeviceAddress = true,
        };
        const auto features13 = VkPhysicalDeviceVulkan13Features{
            .synchronization2 = true,
            .dynamicRendering = true,
        };
        auto features14 = VkPhysicalDeviceVulkan14Features{
            .maintenance5 = VK_TRUE,
        };
        
        auto selector = vkb::PhysicalDeviceSelector{instance}
                             .set_minimum_version(1, 4)
                             .set_required_features(deviceFeatures)
                             .set_required_features_12(features12)
                             .set_required_features_13(features13)
                             .set_required_features_14(features14)
                             .set_surface(surface)
                             .allow_any_gpu_device_type(false)
                             .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete);
        physicalDevice = selector.select().value();

        checkDeviceCapabilities();

        device = vkb::DeviceBuilder{physicalDevice}.build().value();

        graphicsQueueFamily = device.get_queue_index(vkb::QueueType::graphics).value();
        graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();

        auto names = selector.select_device_names().value();
        for(auto name : names) {
            std::println("selector  device name: {}", name);
        }
        std::println("physicalDevice name: {}", physicalDevice.name);

        // { // Init VMA
            const auto vulkanFunctions = VmaVulkanFunctions {
                .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
                .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
            };

            const auto allocatorInfo = VmaAllocatorCreateInfo {
                .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
                .physicalDevice = physicalDevice,
                .device = device,
                .pVulkanFunctions = &vulkanFunctions,
                .instance = instance,
            };
            vmaCreateAllocator(&allocatorInfo, &allocator);
        // }




        swapchain.initSyncStructures(device);


        int w, h;
        glfwGetFramebufferSize(window, &w, &h); 
        swapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;
        swapchain.create(device, swapchainFormat, (std::uint32_t)w, (std::uint32_t)h, vSync);

        createCommandBuffers();

        // createInstanceBuffer();
        return 0;
    }


    void createCommandBuffers()
    {
        const auto poolCreateInfo = vkinit::
            commandPoolCreateInfo(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueueFamily);

        for (std::uint32_t i = 0; i < FRAME_OVERLAP; ++i) {
            auto& commandPool = frames[i].commandPool;
            VK_CHECK(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &commandPool));

            const auto cmdAllocInfo = vkinit::commandBufferAllocateInfo(commandPool, 1);
            auto& mainCommandBuffer = frames[i].mainCommandBuffer;
            VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &mainCommandBuffer));
        }
    }

    VkCommandBuffer beginFrame()
    {
        swapchain.beginFrame(device, getCurrentFrameIndex());

        const auto& frame = getCurrentFrame();
        const auto& cmd = frame.mainCommandBuffer;
        const auto cmdBeginInfo = VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        return cmd;
    }

    void endFrame(VkCommandBuffer cmd, const GPUImage& drawImage, const EndFrameProps& props)
    {
        // get swapchain image
        const auto [swapchainImage, swapchainImageIndex] =
            swapchain.acquireImage(device, getCurrentFrameIndex());
        if (swapchainImage == VK_NULL_HANDLE) {
            return;
        }

        // Fences are reset here to prevent the deadlock in case swapchain becomes dirty
        swapchain.resetFences(device, getCurrentFrameIndex());

        auto swapchainLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        {
            // clear swapchain image
            VkImageSubresourceRange clearRange =
                vkinit::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
            vkutil::transitionImage(cmd, swapchainImage, swapchainLayout, VK_IMAGE_LAYOUT_GENERAL);
            swapchainLayout = VK_IMAGE_LAYOUT_GENERAL;

            const auto clearValue = VkClearColorValue{
                {props.clearColor.r, props.clearColor.g, props.clearColor.b, props.clearColor.a}};
            vkCmdClearColorImage(
                    cmd, swapchainImage, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
        }

        if (props.copyImageIntoSwapchain) {
            // copy from draw image into swapchain
            vkutil::transitionImage(
                    cmd,
                    drawImage.image,
                    VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            vkutil::transitionImage(
                    cmd, swapchainImage, swapchainLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            swapchainLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            const auto filter = props.drawImageLinearBlit ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
            if (props.drawImageBlitRect != glm::ivec4{}) {
                vkutil::copyImageToImage(
                        cmd,
                        drawImage.image,
                        swapchainImage,
                        drawImage.getExtent2D(),
                        props.drawImageBlitRect.x,
                        props.drawImageBlitRect.y,
                        props.drawImageBlitRect.z,
                        props.drawImageBlitRect.w,
                        filter);
            } else {
                // will stretch image to swapchain
                vkutil::copyImageToImage(
                        cmd,
                        drawImage.image,
                        swapchainImage,
                        drawImage.getExtent2D(),
                        getSwapchainExtent(),
                        filter);
            }
        }

        // if (props.drawImGui) {
        //     vkutil::transitionImage(
        //             cmd, swapchainImage, swapchainLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        //     swapchainLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // 
        //     { // draw Dear ImGui
        //         ZoneScopedN("ImGui draw");
        //         TracyVkZoneC(getTracyVkCtx(), cmd, "ImGui", tracy::Color::VioletRed);
        //         vkutil::cmdBeginLabel(cmd, "Draw Dear ImGui");
        //         imGuiBackend.draw(
        //                 cmd, *this, swapchain.getImageView(swapchainImageIndex), swapchain.getExtent());
        //         vkutil::cmdEndLabel(cmd);
        //     }
        // }

        // prepare for present
        vkutil::transitionImage(cmd, swapchainImage, swapchainLayout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        swapchainLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        {
            // TODO: don't collect every frame?
            auto& frame = getCurrentFrame();
            TracyVkCollect(frame.tracyVkCtx, frame.mainCommandBuffer);
        }

        VK_CHECK(vkEndCommandBuffer(cmd));

        swapchain.submitAndPresent(cmd, graphicsQueue, getCurrentFrameIndex(), swapchainImageIndex);

        frameNumber++;
    }

    FrameData& getCurrentFrame()
    {
        return frames[getCurrentFrameIndex()];
    }

    const TracyVkCtx& getTracyVkCtx() const
    {
        return frames[getCurrentFrameIndex()].tracyVkCtx;
    }

    std::uint32_t getCurrentFrameIndex() const
    {
        return frameNumber % FRAME_OVERLAP;
    }

    [[nodiscard]] GPUBuffer createBuffer(
        std::size_t allocSize,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) const {

        const auto bufferInfo = VkBufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = allocSize,
            .usage = usage,
        };
       
        const auto allocInfo = VmaAllocationCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                     // TODO: allow to set VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT when needed
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = memoryUsage,
        };
       
        GPUBuffer buffer{};
        VK_CHECK(vmaCreateBuffer(
            allocator, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.allocation, &buffer.info));
        if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0) {
            const auto deviceAdressInfo = VkBufferDeviceAddressInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .buffer = buffer.buffer,
            };
            buffer.address = vkGetBufferDeviceAddress(device, &deviceAdressInfo);
        }
       
        return buffer;
    }

    [[nodiscard]] VkDeviceAddress getBufferAddress(const GPUBuffer& buffer) const {
        const auto deviceAdressInfo = VkBufferDeviceAddressInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer.buffer,
        };
        return vkGetBufferDeviceAddress(device, &deviceAdressInfo);
    }

    void destroyBuffer(const GPUBuffer& buffer) const {
        vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
    }

    /*
    void createInstanceBuffer() {
        instances.resize(NUM_RECTANGLES);
        std::random_device rd;
        // std::mt19937是C++标准库中的一个伪随机数生成器类
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDist(-1.0f, 1.0f);// [-1.0f, 1.0f)
        std::uniform_real_distribution<float> scaleDist(0.02f, 0.1f);

        for (auto& instance : instances) {
            instance.position = {posDist(gen), posDist(gen)};
            instance.scale = {scaleDist(gen), scaleDist(gen)};
        }

        VkDeviceSize bufferSize = sizeof(InstanceData) * instances.size();
        
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        
        vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &instanceBuffer, &instanceBufferAlloc, nullptr);

        void* data;
        vmaMapMemory(allocator, instanceBufferAlloc, &data);
        memcpy(data, instances.data(), bufferSize);
        vmaUnmapMemory(allocator, instanceBufferAlloc);
    }
    */

    ImageId createDrawImage(
            VkFormat format,
            glm::ivec2 size,
            const char* debugName,
            ImageId imageId = NULL_IMAGE_ID)
    {
        assert(size.x > 0 && size.y > 0);
        const auto extent = VkExtent3D{
            .width = (std::uint32_t)size.x,
                .height = (std::uint32_t)size.y,
                .depth = 1,
        };

        VkImageUsageFlags usages{};
        usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        usages |= VK_IMAGE_USAGE_SAMPLED_BIT;

        const auto createImageInfo = vkutil::CreateImageInfo{
            .format = format,
                .usage = usages,
                .extent = extent,
        };
        return createImage(createImageInfo, debugName, nullptr, imageId);
    }


    ImageId createImage(
            const vkutil::CreateImageInfo& createInfo,
            const char* debugName,
            void* pixelData,
            ImageId imageId)
    {
        auto image = createImageRaw(createInfo);
        if (debugName) {
            vkutil::addDebugLabel(device, image.image, debugName);
            image.debugName = debugName;
        }
        if (pixelData) {
            uploadImageData(image, pixelData);
        }
        if (imageId != NULL_IMAGE_ID) {
            return imageCache.addImage(imageId, std::move(image));
        }
        return addImageToCache(std::move(image));
    }


    GPUImage createImageRaw(
            const vkutil::CreateImageInfo& createInfo,
            std::optional<VmaAllocationCreateInfo> customAllocationCreateInfo) const
    {
        std::uint32_t mipLevels = 1;
        if (createInfo.mipMap) {
            const auto maxExtent = std::max(createInfo.extent.width, createInfo.extent.height);
            mipLevels = (std::uint32_t)std::floor(std::log2(maxExtent)) + 1;
        }

        if (createInfo.isCubemap) {
            assert(createInfo.numLayers % 6 == 0);
            assert(!createInfo.mipMap);
            assert((createInfo.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) != 0);
        }

        auto imgInfo = VkImageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .flags = createInfo.flags,
                .imageType = VK_IMAGE_TYPE_2D,
                .format = createInfo.format,
                .extent = createInfo.extent,
                .mipLevels = mipLevels,
                .arrayLayers = createInfo.numLayers,
                .samples = createInfo.samples,
                .tiling = createInfo.tiling,
                .usage = createInfo.usage,
        };

        static const auto defaultAllocInfo = VmaAllocationCreateInfo{
            .usage = VMA_MEMORY_USAGE_AUTO,
                .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        };
        const auto allocInfo = customAllocationCreateInfo.has_value() ?
            customAllocationCreateInfo.value() :
            defaultAllocInfo;

        GPUImage image{};
        image.format = createInfo.format;
        image.usage = createInfo.usage;
        image.extent = createInfo.extent;
        image.mipLevels = mipLevels;
        image.numLayers = createInfo.numLayers;
        image.isCubemap = createInfo.isCubemap;

        VK_CHECK(
                vmaCreateImage(allocator, &imgInfo, &allocInfo, &image.image, &image.allocation, nullptr));

        // create view only when usage flags allow it
        bool shouldCreateView = ((createInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) != 0) ||
            ((createInfo.usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) ||
            ((createInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0) ||
            ((createInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0);

        if (shouldCreateView) {
            VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
            if (createInfo.format == VK_FORMAT_D32_SFLOAT) { // TODO: support other depth formats
                aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
            }

            auto viewType =
                createInfo.numLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            if (createInfo.isCubemap && createInfo.numLayers == 6) {
                viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            }

            const auto viewCreateInfo = VkImageViewCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = image.image,
                    .viewType = viewType,
                    .format = createInfo.format,
                    .subresourceRange =
                        VkImageSubresourceRange{
                            .aspectMask = aspectFlag,
                            .baseMipLevel = 0,
                            .levelCount = mipLevels,
                            .baseArrayLayer = 0,
                            .layerCount = createInfo.numLayers,
                        },
            };

            VK_CHECK(vkCreateImageView(device, &viewCreateInfo, nullptr, &image.imageView));
        }

        return image;
    }

    void drawFrame() {
        auto cmd = beginFrame();

        gameScreenDrawImageId = createDrawImage(drawImageFormat, glm::ivec2{100, 100}, "game screen draw image");

        finalDrawImageId = createDrawImage(
        drawImageFormat, glm::ivec2{100 , 100}, "final draw image", finalDrawImageId);
        endFrame();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(device);
    }

    void cleanup() {
        // vmaDestroyBuffer(allocator, instanceBuffer, instanceBufferAlloc);
        vmaDestroyAllocator(allocator);
        vkDestroyDevice(device, nullptr);
        // vkb::destroy_instance(vkbInstance);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    VulkanApp app;
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
