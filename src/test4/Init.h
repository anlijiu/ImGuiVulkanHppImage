#pragma once

#ifndef _INIT_H_
#define _INIT_H_

#include <cstdint>
#include <optional>

#include <vulkan/vulkan.h>

struct GPUImage;

namespace vkinit
{
VkImageSubresourceRange imageSubresourceRange(VkImageAspectFlags aspectMask);
VkSemaphoreSubmitInfo semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

VkCommandPoolCreateInfo commandPoolCreateInfo(
    VkCommandPoolCreateFlags flags,
    std::uint32_t queueFamilyIndex);

VkCommandBufferSubmitInfo commandBufferSubmitInfo(VkCommandBuffer cmd);
VkCommandBufferAllocateInfo commandBufferAllocateInfo(
    VkCommandPool commandPool,
    std::uint32_t commandBufferCount);

VkSubmitInfo2 submitInfo(
    const VkCommandBufferSubmitInfo* cmd,
    const VkSemaphoreSubmitInfo* waitSemaphoreInfo,
    const VkSemaphoreSubmitInfo* signalSemaphoreInfo);

VkImageCreateInfo imageCreateInfo(
    VkFormat format,
    VkImageUsageFlags usageFlags,
    VkExtent3D extent,
    std::uint32_t mipLevels = 1);
VkImageViewCreateInfo imageViewCreateInfo(
    VkFormat format,
    VkImage image,
    VkImageAspectFlags aspectFlags);

VkRenderingAttachmentInfo attachmentInfo(
    VkImageView view,
    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    std::optional<VkClearValue> clearValue = std::nullopt);

VkRenderingAttachmentInfo depthAttachmentInfo(
    VkImageView view,
    VkImageLayout layout,
    std::optional<float> depthClearValue = 0.f);

VkRenderingInfo renderingInfo(
    VkExtent2D renderExtent,
    const VkRenderingAttachmentInfo* colorAttachment,
    const VkRenderingAttachmentInfo* depthAttachment);

VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(
    VkShaderStageFlagBits stage,
    VkShaderModule shaderModule);
} // end of namespace vkinit
 
#endif /* ifndef _INIT_H_ */
