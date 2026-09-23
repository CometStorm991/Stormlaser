#pragma once

#include <queue>

#include "VulkanTexture.hpp"

class BindlessRegistry {
public:
    uint32_t add(VulkanTexture&& tex, const vk::raii::Sampler& sampler);
    void flush(const vk::raii::Device& device, vk::DescriptorSet set);
private:
    struct PendingInfo
    {
        vk::Sampler sampler;
        vk::ImageView imageView;
        vk::ImageLayout imageLayout;
    };

    uint32_t next = 0;
    std::queue<uint32_t> freeList;
    std::vector<vk::DescriptorImageInfo> pendingInfos;
    std::vector<uint32_t> pendingSlots;
    std::vector<VulkanTexture> textures;
};