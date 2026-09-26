#pragma once

#include "ItemProcessRequest.hpp"
#include "ItemRegistry.hpp"
#include "VulkanItem.hpp"

#include <vulkan/vulkan_raii.hpp>

class TextureProcessor
{
public:
	TextureProcessor(vk::PhysicalDevice physicalDevice, const vk::raii::Device& device, const VulkanCollection& vulkanCollection)
		: physicalDevice(physicalDevice), device(device), vulkanCollection(vulkanCollection) {
	};

	std::vector<VulkanTexture> processTextures(const std::vector<TextureProcessRequest>& textureProcessRequests);
private:
	vk::PhysicalDevice physicalDevice;
	const vk::raii::Device& device;
	const VulkanCollection& vulkanCollection;
};