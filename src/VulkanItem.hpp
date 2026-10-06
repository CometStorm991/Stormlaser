#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "ItemRegistry.hpp"

struct VulkanSampler
{
	vk::raii::Sampler sampler = nullptr;
};

struct VulkanImage
{
	vk::raii::Image image = nullptr;
	vk::raii::DeviceMemory imageMemory = nullptr;
	vk::raii::ImageView imageView = nullptr;
	vk::Format format;
	uint32_t mipLevels;
};

struct VulkanTexture
{
	vk::DescriptorImageInfo imageInfo;
};

struct VulkanMaterials
{
	vk::raii::Buffer materialBuffer = nullptr;
	vk::raii::DeviceMemory materialMemory = nullptr;
};

struct VulkanMeshRange
{
	uint32_t firstIndex;
	uint32_t indexCount;
	int32_t vertexOffset;

	uint32_t materialIndex;
	bool hasNormals;
	bool hasTangents;
};

struct VulkanMeshes
{
	vk::raii::Buffer vertexBuffer = nullptr;
	vk::raii::DeviceMemory vertexMemory = nullptr;
	vk::raii::Buffer indexBuffer = nullptr;
	vk::raii::DeviceMemory indexMemory = nullptr;
	std::vector<VulkanMeshRange> meshRanges;
};

struct VulkanCollection
{
	ItemRegistry<VulkanImage> vulkanImages;
	ItemRegistry<VulkanSampler> vulkanSamplers;
	ItemRegistry<VulkanTexture> vulkanTextures;
	VulkanMaterials vulkanMaterials;
	VulkanMeshes vulkanMeshes;
};