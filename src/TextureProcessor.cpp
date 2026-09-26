#include "TextureProcessor.hpp"

std::vector<VulkanTexture> TextureProcessor::processTextures(const std::vector<TextureProcessRequest>& textureProcessRequests)
{
	std::vector<VulkanTexture> vulkanTextures;
	for (const TextureProcessRequest& request : textureProcessRequests)
	{
		texture = request.imageIndex;
	}
	
	vk::DescriptorImageInfo imageInfos;
}