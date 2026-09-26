#include <iostream>
#include <vector>

#include <vulkan/vulkan_raii.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <stb_image.h>

#include "ItemProcessRequest.hpp"
#include "VulkanItem.hpp"

class ImageProcessor
{
public:
	ImageProcessor(vk::PhysicalDevice physicalDevice, const vk::raii::Device& device, vk::Queue queue, vk::CommandPool commandPool)
		: physicalDevice(physicalDevice), device(device), queue(queue), commandPool(commandPool) {}

	std::vector<VulkanImage> processImages(const std::vector<ImageProcessRequest>& imageProcessRequests);
private:
	vk::PhysicalDevice physicalDevice;
	const vk::raii::Device& device;
	vk::Queue queue;
	vk::CommandPool commandPool;

	int desiredChannels = STBI_rgb_alpha;

	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	vk::DeviceSize getPaddedImageSize(int width, int height);
	std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
	vk::raii::CommandBuffer beginSingleTimeCommands(vk::CommandPool commandPool);
	void endSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer, vk::Queue queue);
	vk::ImageMemoryBarrier2 createBarrier(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	void copyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer, vk::Image image, uint32_t width, uint32_t height, vk::DeviceSize offset);
	vk::raii::ImageView createImageView(vk::Image const& image, vk::Format format, vk::ImageAspectFlags aspectFlags);

	int roundUp(int num, int x) {
		return ((num + x - 1) / x) * x;
	}
};