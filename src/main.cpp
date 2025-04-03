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
#include <fmt/format.h>

#define VK_CHECK(call)                 \
    do {                               \
        VkResult result_ = call;       \
        assert(result_ == VK_SUCCESS); \
    } while (0)

const int WIDTH = 800;
const int HEIGHT = 600;
const int NUM_RECTANGLES = 500;

inline constexpr std::uint32_t FRAME_OVERLAP = 2;
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
Version appVersion = {0, 0, 0};

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
};

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
            .require_api_version(1, 4, 0)
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
        
        physicalDevice = vkb::PhysicalDeviceSelector{instance}
                             .set_minimum_version(1, 3)
                             .set_required_features(deviceFeatures)
                             .set_required_features_12(features12)
                             .set_required_features_13(features13)
                             .set_required_features_14(features14)
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
