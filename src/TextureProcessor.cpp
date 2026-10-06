#include "TextureProcessor.hpp"

std::vector<VulkanTexture> TextureProcessor::processTextures(const std::vector<TextureProcessRequest>& textureProcessRequests)
{
	std::vector<VulkanTexture> vulkanTextures;
	for (uint32_t i = 0; i < textureProcessRequests.size(); ++i)
	{
		const TextureProcessRequest& request = textureProcessRequests[i];
		const VulkanImage& image = vulkanCollection.vulkanImages.get(request.imageIndex);
		const VulkanSampler& sampler = vulkanCollection.vulkanSamplers.get(request.samplerIndex);
		vk::DescriptorImageInfo imageInfo = {
			.sampler = *sampler.sampler, .imageView = *image.imageView, .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal };
		VulkanTexture vulkanTexture{
			.imageInfo = imageInfo
		};
		vulkanTextures.push_back(vulkanTexture);
	}

	return vulkanTextures;
}