#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "ItemProcessRequest.hpp"
#include "VulkanItem.hpp"

class BufferProcessor
{
public:
	BufferProcessor(vk::PhysicalDevice physicalDevice, const vk::raii::Device& device, vk::Queue queue, vk::CommandPool commandPool)
		: physicalDevice(physicalDevice), device(device), queue(queue), commandPool(commandPool) {}
public:
	vk::PhysicalDevice physicalDevice;
	const vk::raii::Device& device;
	vk::Queue queue;
	vk::CommandPool commandPool;

	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	void copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size);
	vk::raii::CommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer);
};