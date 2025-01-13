#pragma once

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "Log.h"

using uint = u_int32_t;

inline static void CheckVk(VkResult err) {
    if (err != 0) throw std::runtime_error(std::format("Vulkan error: {}", int(err)));
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

inline static bool IsExtensionAvailable(const std::vector<vk::ExtensionProperties> &properties, const char *extension) {
    for (const vk::ExtensionProperties &p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    return false;
}

struct VulkanContext {
    VulkanContext(std::vector<const char *> extensions);
    ~VulkanContext() = default; // Using unique handles, so no need to manually destroy anything.

    vk::UniqueInstance Instance;//VkInstance ---  vk::Instance 的智能指针版本，自动释放
    vk::PhysicalDevice PhysicalDevice;// 物理设备指针   03_physical_device_selection.cpp
    vk::UniqueDevice Device;// 逻辑设备指针   04_logical_device.cpp
    uint QueueFamily = (uint)-1;//队列家族(队列系列) index
    vk::Queue Queue;//队列 -- 从VulkanContext.cpp可以看出是图形队列 graphicsQueue
    vk::UniquePipelineCache PipelineCache;//pipeline 缓存 ImGui_ImplVulkan_Init用，显著加快vkCreateGraphicsPipelines
    vk::UniqueDescriptorPool DescriptorPool;//创建描述符集的时候用 vkAllocateDescriptorSets

    // Find a discrete GPU, or the first available (integrated) GPU.
    vk::PhysicalDevice FindPhysicalDevice() const;
    uint FindMemoryType(uint type_filter, vk::MemoryPropertyFlags) const;
};
