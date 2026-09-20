#include <iostream>
#include <vector>

#include <vulkan/vulkan_raii.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <stb_image.h>

class TextureProcessor
{
public:
	TextureProcessor(vk::PhysicalDevice physicalDevice, const vk::raii::Device& device)
		: physicalDevice(physicalDevice), device(device) {};

	void processImages(fastgltf::Asset asset, vk::Queue queue, vk::CommandPool commandPool);
private:
	vk::PhysicalDevice physicalDevice;
	const vk::raii::Device& device;

	int desiredChannels = STBI_rgb_alpha;

	struct TextureData
	{
		int width, height, nrChannels;
		unsigned char* data;
		TextureData(int width, int height, int nrChannels, unsigned char* data)
			: width(width), height(height), nrChannels(nrChannels), data(data) {}
		~TextureData()
		{
			stbi_image_free(data);
		}
	};

	TextureData getDataFromGLTFImage(const fastgltf::Asset& asset, const fastgltf::Image& image);
	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	vk::DeviceSize getPaddedImageSize(int width, int height);
	std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
	vk::raii::CommandBuffer beginSingleTimeCommands(vk::CommandPool commandPool);
	void endSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer, vk::Queue queue);
	vk::ImageMemoryBarrier2 createBarrier(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	void copyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer, vk::Image image, uint32_t width, uint32_t height);

	int roundUp(int num, int x) {
		return ((num + x - 1) / x) * x;
	}
};