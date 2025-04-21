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
#include <glm/vec2.hpp>

#include "Color.h"
#include "Version.h"
#include "Swapchain.h"
#include "GPUImage.h"

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

    struct EndFrameProps {
        const LinearColor clearColor{0.f, 0.f, 0.f, 1.f};
        bool copyImageIntoSwapchain{true};
        glm::ivec4 drawImageBlitRect{}; // where to blit draw image to
        bool drawImageLinearBlit{true}; // if false - nearest filter will be used
        bool drawImGui{true};
    };
    void endFrame(VkCommandBuffer cmd, const GPUImage& drawImage, const EndFrameProps& props);
    void cleanup();

    void waitIdle() const;

public:
    VkDevice getDevice() const { return device; }

    std::uint32_t getCurrentFrameIndex() const;
    VkExtent2D getSwapchainExtent() const { return swapchain.getExtent(); }
    glm::ivec2 getSwapchainSize() const
    {
        return {getSwapchainExtent().width, getSwapchainExtent().height};
    }

    VkFormat getSwapchainFormat() const { return swapchainFormat; }
    bool needsSwapchainRecreate() const { return swapchain.needsRecreation(); }
private:
    void initVulkan(GLFWwindow* window, const char* appName, const Version& appVersion);
    void checkDeviceCapabilities();
    void createCommandBuffers();

    FrameData& getCurrentFrame();
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
    Swapchain swapchain;

    std::array<FrameData, graphics::FRAME_OVERLAP> frames{};
    std::uint32_t frameNumber{0};



    VkSampleCountFlagBits supportedSampleCounts;
    VkSampleCountFlagBits highestSupportedSamples{VK_SAMPLE_COUNT_1_BIT};
    float maxSamplerAnisotropy{1.f};

    bool vSync{true};
};

#endif /* ifndef _GFLDEVICE_H_ */
