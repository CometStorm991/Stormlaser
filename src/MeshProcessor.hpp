#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "BufferProcessor.hpp"
#include "ItemProcessRequest.hpp"
#include "VulkanItem.hpp"

class MeshProcessor
{
public:
	MeshProcessor(vk::PhysicalDevice physicalDevice, const vk::raii::Device& device, vk::Queue queue, vk::CommandPool commandPool);
	VulkanMeshes processMeshes(const std::vector<MeshProcessRequest>& meshProcessRequests);
private:
	BufferProcessor bufferProcessor;
};
