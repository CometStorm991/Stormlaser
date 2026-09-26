#include "GLTFProcessor.hpp"

// Adapted from fastgltf's gl_viewer.cpp
bool GLTFProcessor::loadGLTF(std::filesystem::path path, fastgltf::Asset& asset)
{
    if (!std::filesystem::exists(path)) {
        std::cout << "Failed to find " << path << '\n';
        return false;
    }

    if constexpr (std::is_same_v<std::filesystem::path::value_type, wchar_t>) {
        std::wcout << "Loading " << path << '\n';
    }
    else {
        std::cout << "Loading " << path << '\n';
    }

    // Parse the glTF file and get the constructed asset
    {
        static constexpr auto supportedExtensions =
            fastgltf::Extensions::KHR_mesh_quantization |
            fastgltf::Extensions::KHR_texture_transform |
            fastgltf::Extensions::KHR_materials_variants;

        fastgltf::Parser parser(supportedExtensions);

        constexpr auto gltfOptions =
            fastgltf::Options::DontRequireValidAssetMember |
            fastgltf::Options::AllowDouble |
            fastgltf::Options::LoadExternalBuffers |
            fastgltf::Options::LoadExternalImages |
            fastgltf::Options::GenerateMeshIndices;

        auto gltfFile = fastgltf::MappedGltfFile::FromPath(path);
        if (!bool(gltfFile)) {
            std::cerr << "Failed to open glTF file: " << fastgltf::getErrorMessage(gltfFile.error()) << '\n';
            return false;
        }

        auto expectedAsset = parser.loadGltf(gltfFile.get(), path.parent_path(), gltfOptions);
        if (expectedAsset.error() != fastgltf::Error::None) {
            std::cerr << "Failed to load glTF: " << fastgltf::getErrorMessage(expectedAsset.error()) << '\n';
            return false;
        }

        asset = std::move(expectedAsset.get());
    }

    return true;
}

// Adapted from fastgltf's gl_viewer.cpp
ImageProcessRequest GLTFProcessor::processImage(const fastgltf::Image& image, const fastgltf::Asset& asset, const std::string& parentDirectory)
{
    int width, height, nrChannels;
    unsigned char* data;
    
    std::visit(fastgltf::visitor{
        [&](const fastgltf::sources::URI& filePath) {
            assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
            assert(filePath.uri.isLocalPath()); // We're only capable of loading local files.

            std::string path(filePath.uri.path().begin(), filePath.uri.path().end()); // Thanks C++.
            path = parentDirectory + path;
            data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
        },
        [&](const fastgltf::sources::Array& vector) {
            data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data()), static_cast<int>(vector.bytes.size()), &width, &height, &nrChannels, STBI_rgb_alpha);
        },
        [&](const fastgltf::sources::BufferView& view) {
            auto& bufferView = asset.bufferViews[view.bufferViewIndex];
            auto& buffer = asset.buffers[bufferView.bufferIndex];

            std::visit(fastgltf::visitor {
                // We only care about VectorWithMime here, because we specify LoadExternalBuffers, meaning
                // all buffers are already loaded into a vector.
                [](auto& arg) {},
                [&](fastgltf::sources::Array& vector) {
                    data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data() + bufferView.byteOffset),
                                                                static_cast<int>(bufferView.byteLength), &width, &height, &nrChannels, STBI_rgb_alpha);
                }
            }, buffer.data);
        },
        [](auto& arg) {
            std::cerr << "GLTF Image failed to load!\n";
        }
        }, image.data);

    return ImageProcessRequest{
        .pixelData = data,
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .nrChannels = static_cast<uint32_t>(nrChannels),
        .format = vk::Format::eR8G8B8A8Srgb,
        .generateMipmaps = false
    };
}

SamplerProcessRequest GLTFProcessor::processSample(const fastgltf::Sampler& sampler)
{
    SamplerProcessRequest request{};

    std::tie(request.magFilter, std::ignore) = sampler.magFilter.has_value() ?
        toVkFilter(sampler.magFilter.value()): std::make_pair(vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);
    std::tie(request.minFilter, request.mipmapMode) = sampler.minFilter.has_value() ?
        toVkFilter(sampler.minFilter.value()) : std::make_pair(vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);

    request.wrapU = toVkSamplerAddressMode(sampler.wrapS);
    request.wrapV = toVkSamplerAddressMode(sampler.wrapT);

    return request;
}

std::pair<vk::Filter, vk::SamplerMipmapMode> GLTFProcessor::toVkFilter(fastgltf::Filter filter)
{
    switch (filter)
    {
    case fastgltf::Filter::Linear:
        return std::make_pair(vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);
    case fastgltf::Filter::Nearest:
        return std::make_pair(vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest);
    case fastgltf::Filter::LinearMipMapLinear:
        return std::make_pair(vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);
    case fastgltf::Filter::LinearMipMapNearest:
        return std::make_pair(vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest);
    case fastgltf::Filter::NearestMipMapLinear:
        return std::make_pair(vk::Filter::eNearest, vk::SamplerMipmapMode::eLinear);
    case fastgltf::Filter::NearestMipMapNearest:
        return std::make_pair(vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest);
    }
}

vk::SamplerAddressMode GLTFProcessor::toVkSamplerAddressMode(fastgltf::Wrap wrap)
{
    switch (wrap)
    {
    case fastgltf::Wrap::Repeat:
        return vk::SamplerAddressMode::eRepeat;
    case fastgltf::Wrap::ClampToEdge:
        return vk::SamplerAddressMode::eClampToEdge;
    }
}

TextureProcessRequest GLTFProcessor::processTexture(const fastgltf::Texture& texture)
{
    return TextureProcessRequest{
        .samplerIndex = static_cast<uint32_t>(texture.samplerIndex.has_value() ? texture.samplerIndex.value() + 1 : 0),
        .imageIndex = static_cast<uint32_t>(texture.imageIndex.has_value() ? texture.imageIndex.value() + 1 : 0)
    };
}

MaterialProcessRequest GLTFProcessor::processMaterial(const fastgltf::Material& material)
{
    MaterialProcessRequest request;
    request.baseColorFactor = glm::make_vec4(material.pbrData.baseColorFactor.data());
    request.baseColorTextureIndex = material.pbrData.baseColorTexture.has_value() ?
        material.pbrData.baseColorTexture.value().textureIndex + 1 : 0;

    request.normalTextureIndex = material.normalTexture.has_value() ?
        material.normalTexture.value().textureIndex + 1 : 0;

    request.metallicFactor = material.pbrData.metallicFactor;
    request.roughnessFactor = material.pbrData.roughnessFactor;
    request.metallicRoughnessTextureIndex = material.pbrData.metallicRoughnessTexture.has_value() ?
        material.pbrData.metallicRoughnessTexture.value().textureIndex + 1 : 0;

    return request;
}

// Adapted from fastgltf's gl_viewer.cpp
MeshProcessRequest GLTFProcessor::processMesh(const fastgltf::Mesh& mesh)
{
    MeshProcessRequest meshProcessRequest;
    meshProcessRequest.primitives.resize(mesh.primitives.size());
    for (auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it)
    {
        auto* positionIt = it->findAttribute("POSITION");
        assert(positionIt != it->attributes.end()); // A mesh primitive is required to hold the POSITION attribute.
        assert(it->indicesAccessor.has_value()); // We specify GenerateMeshIndices, so we should always have indices

        auto index = std::distance(mesh.primitives.begin(), it);
        auto& primitive = meshProcessRequest.primitives[index];
        primitive.topology = toVkPrimitiveTopology(it->type);

        if (it->materialIndex.has_value())
        {
            primitive.materialIndex = it->materialIndex.value() + 1; // 1-indexed
            
        }
        else
        {
            primitive.materialIndex = 0;
        }
    }
}

vk::PrimitiveTopology GLTFProcessor::toVkPrimitiveTopology(fastgltf::PrimitiveType type)
{
    switch (type)
    {
    case fastgltf::PrimitiveType::Triangles:
        return vk::PrimitiveTopology::eTriangleList;
    }
}