#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "ItemRegistry.hpp"

struct VulkanSampler
{
	vk::raii::Sampler sampler;
};

struct VulkanImage
{
	vk::raii::Image image;
	vk::raii::DeviceMemory imageMemory;
	vk::raii::ImageView imageView;
	vk::Format format;
	uint32_t mipLevels;
};

struct VulkanTexture
{

};

struct VulkanCollection
{
	ItemRegistry<VulkanImage> vulkanImages;
	ItemRegistry<VulkanSampler> vulkanSamplers;
	ItemRegistry<VulkanTexture> vulkanTextures;
};