#pragma once

#include <iostream>
#include <vector>

#include <vulkan/vulkan_raii.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "ItemProcessRequest.hpp"
#include "Vertex.hpp"

class GLTFProcessor
{
public:
	bool loadGLTF(std::filesystem::path path, fastgltf::Asset& asset);
	ImageProcessRequest processImage(const fastgltf::Image& image, const fastgltf::Asset& asset, const std::string& parentDirectory);
	SamplerProcessRequest processSample(const fastgltf::Sampler& sampler);
	TextureProcessRequest processTexture(const fastgltf::Texture& texture);
	MaterialProcessRequest processMaterial(const fastgltf::Material& material);
	MeshProcessRequest processMesh(const fastgltf::Mesh& mesh, const fastgltf::Asset& asset);
private:
	using PrimitiveIterator = std::vector<fastgltf::Primitive, std::pmr::polymorphic_allocator<fastgltf::Primitive>>::const_iterator;

	std::pair<vk::Filter, vk::SamplerMipmapMode> toVkFilter(fastgltf::Filter filter);
	vk::SamplerAddressMode toVkSamplerAddressMode(fastgltf::Wrap wrap);
	vk::PrimitiveTopology toVkPrimitiveTopology(fastgltf::PrimitiveType type);
	std::size_t getTexcoordIndex(const fastgltf::TextureInfo& textureInfo, const fastgltf::Material& material, const fastgltf::Asset& asset);
	void writeTexcoordData(std::size_t texcoordIndex, PrimitiveIterator it, const fastgltf::Asset& asset, std::vector<Vertex>& vertices);
};