#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vertex.hpp"

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
	alignas(16) glm::vec4 baseColorFactor;
	uint32_t baseColorTextureIndex;

	uint32_t normalTextureIndex;
	float normalScale;

	float metallicFactor;
	float roughnessFactor;
	uint32_t metallicRoughnessTextureIndex;
};

struct Primitive
{
	uint32_t materialIndex;
	std::vector<Vertex> vertices; // Interleaved data
	std::vector<uint32_t> indices;
	uint32_t baseColorTexcoordIndex;
	uint32_t normalTexcoordIndex;
	uint32_t metallicRoughnessTexcoordIndex;
	vk::PrimitiveTopology topology;
	bool hasNormals;
	bool hasTangents;
};

struct MeshProcessRequest
{
	std::vector<Primitive> primitives;
};