// Minimal Vulkan app using vk-bootstrap + VMA demonstrating dynamic rendering vs render pass
// Requires Vulkan 1.3, VK_KHR_dynamic_rendering, vk-bootstrap, Vulkan Memory Allocator (VMA)

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <iostream>

#define WIDTH 800
#define HEIGHT 600

VkInstance instance;
VkDevice device;
VkPhysicalDevice physicalDevice;
VkQueue graphicsQueue;
VkCommandPool commandPool;
VkCommandBuffer commandBuffer;
VkSurfaceKHR surface;
VmaAllocator allocator;

VkFormat swapchainFormat;
VkExtent2D swapchainExtent;
VkImage swapchainImage;
VkImageView swapchainImageView;
VkFramebuffer framebuffer;

VkPipelineLayout pipelineLayout;
VkPipeline pipeline;

VkRenderPass renderPass;

VkCommandBufferBeginInfo cmdBufBeginInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
};

// GLFW Callback
void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

int main() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Dynamic Rendering vs RenderPass", nullptr, nullptr);

    // vk-bootstrap setup
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("dynamic-rendering-demo")
                          .request_validation_layers(true)
                          .use_default_debug_messenger()
                          .require_api_version(1, 3, 0)
                          .build();

    vkb::Instance vkb_inst = inst_ret.value();
    instance = vkb_inst.instance;

    // Create surface from GLFW
    glfwCreateWindowSurface(instance, window, nullptr, &surface);

    vkb::PhysicalDeviceSelector selector(vkb_inst);
    auto phys_ret = selector.set_surface(surface).select();
    vkb::PhysicalDevice vkb_phys = phys_ret.value();

    vkb::DeviceBuilder dev_builder(vkb_phys);
    auto dev_ret = dev_builder.build();
    vkb::Device vkb_dev = dev_ret.value();

    device = vkb_dev.device;
    physicalDevice = vkb_phys.physical_device;
    graphicsQueue = vkb_dev.get_queue(vkb::QueueType::graphics).value();

    // VMA setup
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    vmaCreateAllocator(&allocatorInfo, &allocator);

    // Command pool and buffer
    VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolInfo.queueFamilyIndex = vkb_phys.get_queue_family(vkb::QueueType::graphics).value();
    vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);

    VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    // Dynamic Rendering Code
    VkRenderingAttachmentInfo colorAttachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = VK_NULL_HANDLE, // should be actual swapchain imageView
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {.color = {{0.1f, 0.2f, 0.3f, 1.0f}}},
    };

    VkRenderingInfo renderInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {.offset = {0, 0}, .extent = {WIDTH, HEIGHT}},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
    };

    // Record commands
    vkBeginCommandBuffer(commandBuffer, &cmdBufBeginInfo);

    vkCmdBeginRendering(commandBuffer, &renderInfo);
    // vkCmdBindPipeline, vkCmdDraw, etc. here
    vkCmdEndRendering(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    // Submit and cleanup
    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    // Cleanup (simplified)
    vmaDestroyAllocator(allocator);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    vkb::destroy_instance(vkb_inst);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
