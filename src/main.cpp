#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "VkBootstrap.h"
#include "volk.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <random>
#include <stdio.h>

const int WIDTH = 800;
const int HEIGHT = 600;
const int NUM_RECTANGLES = 500;

inline constexpr std::uint32_t FRAME_OVERLAP = 2;


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

struct InstanceData {
    glm::vec2 position;
    glm::vec2 scale;
};

class Swapchain {

private:
    struct FrameData {
        VkSemaphore swapchainSemaphore;
        VkSemaphore renderSemaphore;
        VkFence renderFence;
    };

    //飞行帧
    std::array<FrameData, FRAME_OVERLAP> frames;
    vkb::Swapchain swapchain;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    bool dirty{false};
}

class VulkanApp {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    vkb::Instance vkbInstance;
    vkb::PhysicalDevice physicalDevice;
    vkb::Device device;
    VmaAllocator allocator;

    std::uint32_t graphicsQueueFamily;
    VkQueue graphicsQueue;

    VkSurfaceKHR surface;
    VkFormat swapchainFormat;
    Swapchain swapchain;

    std::array<FrameData, graphics::FRAME_OVERLAP> frames{};

    VkDevice device;
    VkQueue graphicsQueue;
    VkPhysicalDevice physicalDevice;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkBuffer vertexBuffer, instanceBuffer;
    VmaAllocator allocator;
    VmaAllocation vertexBufferAlloc, instanceBufferAlloc;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    VkSwapchainKHR swapchain;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    VkSemaphore imageAvailableSemaphore, renderFinishedSemaphore;
    VkFence inFlightFence;

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
        volkInitialize();
        
        vkb::InstanceBuilder builder;
        vkbInstance = builder.set_app_name("Vulkan Sprites")
                    .request_validation_layers(true)
                    .build().value();

        VkSurfaceKHR surface;
        VkResult err;

        glfwCreateWindowSurface(vkbInstance.instance, window, nullptr, &surface);

        vkb::PhysicalDeviceSelector selector{vkbInstance};
        auto phys_ret = selector.set_surface(surface).select();
        physicalDevice = phys_ret.value();

        vkb::DeviceBuilder deviceBuilder{phys_ret.value()};
        auto vkbDevice = deviceBuilder.build().value();
        device = vkbDevice.device;
        graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();

        volkLoadInstance(vkbInstance);

        const auto vulkanFunctions = VmaVulkanFunctions{
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        };

        const auto allocatorInfo = VmaAllocatorCreateInfo{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = physicalDevice,
            .device = vkbDevice,
            .pVulkanFunctions = &vulkanFunctions,
            .instance = vkbInstance,
        };

        vmaCreateAllocator(&allocatorInfo, &allocator);

        // ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
        // // Select Surface Format
        // const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
        // const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        // wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

        createInstanceBuffer();
    }

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

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        vmaDestroyBuffer(allocator, instanceBuffer, instanceBufferAlloc);
        vmaDestroyAllocator(allocator);
        vkDestroyDevice(device, nullptr);
        vkb::destroy_instance(vkbInstance);
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
