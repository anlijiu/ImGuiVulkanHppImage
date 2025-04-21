
#include "GfxDevice.h"

#include "Util.h"
#include "Init.h"

#include <vma/vk_mem_alloc.h>
#include <iostream>
#include <GLFW/glfw3.h>


GfxDevice::GfxDevice() {}

GfxDevice::~GfxDevice() {}

void GfxDevice::init(GLFWwindow* window, const char* appName, const Version& version, bool vSync) {

    initVulkan(window, appName, version);

    // executor = createImmediateExecutor();

    swapchain.initSyncStructures(device);

    this->vSync = vSync;

    // [Mac OS 和 glfw 上的高 DPI 缩放](https://github.com/ocornut/imgui/issues/5081)
    int winWidth, winHeight;
    glfwGetWindowSize(window, &winWidth, &winHeight); // 逻辑尺寸 例如可能是 800x600

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight); // 物理像素 例如可能是 1600x1200
                                                                        //
    swapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;
    swapchain.create(device, swapchainFormat, (std::uint32_t)fbWidth, (std::uint32_t)fbHeight, vSync);

    createCommandBuffers();
}

VkCommandBuffer GfxDevice::beginFrame()
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

void GfxDevice::waitIdle() const
{
    VK_CHECK(vkDeviceWaitIdle(device));
}

void GfxDevice::recreateSwapchain(std::uint32_t swapchainWidth, std::uint32_t swapchainHeight)
{
    assert(swapchainWidth != 0 && swapchainHeight != 0);
    waitIdle();
    swapchain.recreate(
        device,
        swapchainFormat,
        (std::uint32_t)swapchainWidth,
        (std::uint32_t)swapchainHeight,
        vSync);
}

void GfxDevice::initVulkan(GLFWwindow* window, const char* appName, const Version& appVersion) {

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

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        std::cout << "Failed to create Vulkan surface: check GLFW_NO_API " << std::endl;
        std::exit(0);
    }

    const auto deviceFeatures = VkPhysicalDeviceFeatures{
#if VK_VALIDATION
		.robustBufferAccess = VK_TRUE,
#endif //VK_VALIDATION
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


    physicalDevice = vkb::PhysicalDeviceSelector{instance}
                         .set_minimum_version(1, 3)
                         .set_required_features(deviceFeatures)
                         .set_required_features_12(features12)
                         .set_required_features_13(features13)
                         .set_surface(surface)
                         .select()
                         .value();

    checkDeviceCapabilities();

    device = vkb::DeviceBuilder{physicalDevice}.build().value();

    graphicsQueueFamily = device.get_queue_index(vkb::QueueType::graphics).value();
    graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();

    { // Init VMA
        const auto vulkanFunctions = VmaVulkanFunctions{
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        };

        const auto allocatorInfo = VmaAllocatorCreateInfo{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = physicalDevice,
            .device = device,
            .pVulkanFunctions = &vulkanFunctions,
            .instance = instance,
        };
        vmaCreateAllocator(&allocatorInfo, &allocator);
    }
}

void GfxDevice::checkDeviceCapabilities()
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

void GfxDevice::createCommandBuffers()
{
    const auto poolCreateInfo = vkinit::
        commandPoolCreateInfo(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueueFamily);

    for (std::uint32_t i = 0; i < graphics::FRAME_OVERLAP; ++i) {
        auto& commandPool = frames[i].commandPool;
        VK_CHECK(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &commandPool));

        const auto cmdAllocInfo = vkinit::commandBufferAllocateInfo(commandPool, 1);
        auto& mainCommandBuffer = frames[i].mainCommandBuffer;
        VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &mainCommandBuffer));
    }
}

GfxDevice::FrameData& GfxDevice::getCurrentFrame()
{
    return frames[getCurrentFrameIndex()];
}


std::uint32_t GfxDevice::getCurrentFrameIndex() const
{
    return frameNumber % graphics::FRAME_OVERLAP;
}


void GfxDevice::cleanup()
{

}
