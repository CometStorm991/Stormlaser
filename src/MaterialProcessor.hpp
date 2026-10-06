#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "BufferProcessor.hpp"
#include "ItemProcessRequest.hpp"
#include "VulkanItem.hpp"

class MaterialProcessor
{
public:
	MaterialProcessor(vk::PhysicalDevice physicalDevice, const vk::raii::Device& device, vk::Queue queue, vk::CommandPool commandPool);
	VulkanMaterials processMaterials(const std::vector<MaterialProcessRequest>& materialProcessRequests);
private:
	BufferProcessor bufferProcessor;
};