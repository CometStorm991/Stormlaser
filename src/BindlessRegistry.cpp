#include "BindlessRegistry.hpp"

uint32_t BindlessRegistry::add(VulkanTexture&& tex, const vk::raii::Sampler& sampler) {
    uint32_t slot;
    if (freeList.empty())
    {
        slot = next++;
    }
    else
    {
        slot = freeList.front();
        freeList.pop();
    }

    pendingInfos.push_back({
        .sampler = *sampler,
        .imageView = *tex.imageView,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal });
    pendingSlots.push_back(slot);
    // BindlessRegistry owns the image lifetime now
    if (!textures.contains(slot))
    {
        textures.insert({ slot, std::move(tex) });
    }
    else
    {
        textures.at(slot) = std::move(tex);
    }
    return slot;
}

void BindlessRegistry::flush(const vk::raii::Device& device, vk::DescriptorSet set)
{
    std::vector<vk::WriteDescriptorSet> writes;
    for (size_t i = 0; i < pendingSlots.size(); ++i)
        writes.push_back({
            .dstSet = set,
            .dstBinding = 0,
            .dstArrayElement = pendingSlots[i],
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &pendingInfos[i] });
    device.updateDescriptorSets(writes, {});
    pendingInfos.clear();
    pendingSlots.clear();
}