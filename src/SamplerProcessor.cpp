#include "SamplerProcessor.hpp"

std::vector<VulkanSampler> SamplerProcessor::processSamplers(const std::vector<SamplerProcessRequest>& samplerProcessRequests)
{
	std::vector<VulkanSampler> vulkanSamplers;
	for (const SamplerProcessRequest& request : samplerProcessRequests)
	{
		vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
		vk::SamplerCreateInfo        samplerInfo{ .magFilter = request.magFilter,
												 .minFilter = request.minFilter,
												 .mipmapMode = request.mipmapMode,
												 .addressModeU = request.wrapU,
												 .addressModeV = request.wrapV,
												 .addressModeW = vk::SamplerAddressMode::eRepeat,
												 .mipLodBias = 0.0f,
												 .anisotropyEnable = vk::True,
												 .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
												 .compareEnable = vk::False,
												 .compareOp = vk::CompareOp::eAlways,
												 .minLod = 0.0f,
												 .maxLod = 0.0f };

		vulkanSamplers.push_back(VulkanSampler{
			.sampler = vk::raii::Sampler(device, samplerInfo) });
	}
	
	return vulkanSamplers;
}