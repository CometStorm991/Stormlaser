#pragma once

#include "ItemProcessRequest.hpp"
#include "VulkanItem.hpp"

#include <vulkan/vulkan_raii.hpp>

class SamplerProcessor
{
public:
	SamplerProcessor(vk::PhysicalDevice physicalDevice, const vk::raii::Device& device)
		: physicalDevice(physicalDevice), device(device) {};

	std::vector<VulkanSampler> processSamplers(const std::vector<SamplerProcessRequest>& samplerProcessRequests);
private:
	vk::PhysicalDevice physicalDevice;
	const vk::raii::Device& device;
};