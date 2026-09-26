#pragma once

#include <iostream>

#include <vulkan/vulkan_raii.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "ItemProcessRequest.hpp"

class GLTFProcessor
{
public:
	bool loadGLTF(std::filesystem::path path, fastgltf::Asset& asset);
	ImageProcessRequest processImage(const fastgltf::Image& image, const fastgltf::Asset& asset, const std::string& parentDirectory);
	SamplerProcessRequest processSample(const fastgltf::Sampler& sampler);
	TextureProcessRequest processTexture(const fastgltf::Texture& texture);
	MaterialProcessRequest processMaterial(const fastgltf::Material& material);
	MeshProcessRequest processMesh(const fastgltf::Mesh& mesh);
private:
	std::pair<vk::Filter, vk::SamplerMipmapMode> toVkFilter(fastgltf::Filter filter);
	vk::SamplerAddressMode toVkSamplerAddressMode(fastgltf::Wrap wrap);
	vk::PrimitiveTopology toVkPrimitiveTopology(fastgltf::PrimitiveType type);
};