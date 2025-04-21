#ifndef _GFLDEVICE_H_
#define _GFLDEVICE_H_

// don't sort these includes
// clang-format off
#include <vulkan/vulkan.h>
#include <volk.h> // include needed for TracyVulkan.hpp
#include <VkBootstrap.h>
#include <vma/vk_mem_alloc.h>
#include <tracy/TracyVulkan.hpp>

// clang-format on

#include <glm/vec4.hpp>

#include "Version.h"

struct GLFWwindow;

class GfxDevice
{
public:
    struct FrameData {
        VkCommandPool commandPool;
        VkCommandBuffer mainCommandBuffer;
        TracyVkCtx tracyVkCtx;
    };

public:

    GfxDevice();
    GfxDevice(const GfxDevice&) = delete;
    GfxDevice& operator=(const GfxDevice&) = delete;

    virtual ~GfxDevice ();

    void init(GLFWwindow* window, const char* appName, const Version& appVersion, bool vSync);
    void recreateSwapchain(std::uint32_t swapchainWidth, std::uint32_t swapchainHeight);

    VkCommandBuffer beginFrame();

private:
    void initVulkan(GLFWwindow* window, const char* appName, const Version& appVersion);
    void checkDeviceCapabilities();
private:
    /* data */
    vkb::Instance instance;
    vkb::PhysicalDevice physicalDevice;
    vkb::Device device;
    VmaAllocator allocator;

    std::uint32_t graphicsQueueFamily;
    VkQueue graphicsQueue;

    VkSurfaceKHR surface;
    VkFormat swapchainFormat;
    // Swapchain swapchain;




    VkSampleCountFlagBits supportedSampleCounts;
    VkSampleCountFlagBits highestSupportedSamples{VK_SAMPLE_COUNT_1_BIT};
    float maxSamplerAnisotropy{1.f};
};

#endif /* ifndef _GFLDEVICE_H_ */
