#pragma once

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

struct TextureProcessRequest
{
	const void* pixelData;
	uint32_t width;
	uint32_t height;
	uint32_t nrChannels;
	vk::Format format;
	bool generateMipmaps;
};