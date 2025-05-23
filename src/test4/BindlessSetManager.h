#pragma once

#ifndef _BINDLESS_SET_MANAGER_H
#define _BINDLESS_SET_MANAGER_H 

#include <cstdint>

#include <vulkan/vulkan.h>

class BindlessSetManager {
public:
    void init(VkDevice device, float maxAnisotropy);
    void cleanup(VkDevice device);

    VkDescriptorSetLayout getDescSetLayout() const { return descSetLayout; }
    const VkDescriptorSet& getDescSet() const { return descSet; }

    void addImage(VkDevice device, std::uint32_t id, const VkImageView imageView);
    void addSampler(VkDevice device, std::uint32_t id, VkSampler sampler);

private:
    void initDefaultSamplers(VkDevice device, float maxAnisotropy);

    VkDescriptorPool descPool;
    VkDescriptorSetLayout descSetLayout;
    VkDescriptorSet descSet;

    VkSampler nearestSampler;
    VkSampler linearSampler;
    VkSampler shadowMapSampler;
};

#endif /* ifndef _BINDLESS_SET_MANAGER_H */
