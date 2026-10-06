#pragma once

#include <vector>

#include "ItemProcessRequest.hpp"
#include "ItemRegistry.hpp"
#include "VulkanItem.hpp"

#include <vulkan/vulkan_raii.hpp>

class TextureProcessor
{
public:
	TextureProcessor(const VulkanCollection& vulkanCollection)
		: vulkanCollection(vulkanCollection) {};

	std::vector<VulkanTexture> processTextures(const std::vector<TextureProcessRequest>& textureProcessRequests);
private:
	const VulkanCollection& vulkanCollection;
};