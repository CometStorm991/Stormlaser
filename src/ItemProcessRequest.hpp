#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

struct SamplerProcessRequest
{
	vk::Filter magFilter;
	vk::Filter minFilter;
	vk::SamplerMipmapMode mipmapMode;
	vk::SamplerAddressMode wrapU;
	vk::SamplerAddressMode wrapV;
};

struct ImageProcessRequest
{
	const void* pixelData;
	uint32_t width;
	uint32_t height;
	uint32_t nrChannels;
	vk::Format format;
	bool generateMipmaps;
};

struct TextureProcessRequest
{
	uint32_t samplerIndex;
	uint32_t imageIndex;
};

struct MaterialProcessRequest
{
	glm::vec4 baseColorFactor;
	uint32_t baseColorTextureIndex;
	uint32_t normalTextureIndex;
	float metallicFactor;
	float roughnessFactor;
	uint32_t metallicRoughnessTextureIndex;
};

struct Primitive
{
	uint32_t materialIndex;
	std::vector<uint32_t> vertices; // Interleaved data
	std::vector<uint32_t> indices;
	vk::PrimitiveTopology topology;
};

struct MeshProcessRequest
{

	std::vector<Primitive> primitives;
};