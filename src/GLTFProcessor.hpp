#pragma once

#include <iostream>

#include <vulkan/vulkan_raii.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <stb_image.h>

#include "TextureProcessRequest.hpp"

class GLTFProcessor
{
public:
	 std::vector<TextureProcessRequest> processImages(const fastgltf::Asset& asset, const std::string& parentDirectory);
private:
	TextureProcessRequest processImage(const fastgltf::Image& image, const fastgltf::Asset& asset, const std::string& parentDirectory);
};