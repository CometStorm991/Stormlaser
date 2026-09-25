#pragma once

#include <vulkan/vulkan_raii.hpp>

struct VulkanTexture
{
	vk::raii::Image image;
	vk::raii::DeviceMemory imageMemory;
	vk::raii::ImageView imageView;
	vk::Format format;
	uint32_t mipLevels;
};