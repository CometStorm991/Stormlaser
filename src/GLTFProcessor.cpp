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
    request.normalScale = material.normalTexture.has_value() ?
        material.normalTexture.value().scale : 1.0f;

    request.metallicFactor = material.pbrData.metallicFactor;
    request.roughnessFactor = material.pbrData.roughnessFactor;
    request.metallicRoughnessTextureIndex = material.pbrData.metallicRoughnessTexture.has_value() ?
        material.pbrData.metallicRoughnessTexture.value().textureIndex + 1 : 0;

    return request;
}

// Adapted from fastgltf's gl_viewer.cpp
MeshProcessRequest GLTFProcessor::processMesh(const fastgltf::Mesh& mesh, const fastgltf::Asset& asset)
{
    MeshProcessRequest request;
    request.primitives.resize(mesh.primitives.size());
    for (auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it)
    {
        auto index = std::distance(mesh.primitives.begin(), it);
        assert(it->indicesAccessor.has_value()); // We specify GenerateMeshIndices, so we should always have indices
        auto& primitive = request.primitives[index];
        primitive.topology = toVkPrimitiveTopology(it->type);

        // -------------------- Index --------------------
        auto& indicesAccessor = asset.accessors[it->indicesAccessor.value()];
        if (!indicesAccessor.bufferViewIndex.has_value())
        {
            std::cerr << "[GLTFProcessor] Indices accessor doesn't have a buffer view index" << std::endl;
            continue;
        }
        primitive.indices.resize(indicesAccessor.count);
        fastgltf::iterateAccessorWithIndex<std::uint32_t>(asset, indicesAccessor,
            [&](std::uint32_t value, std::size_t index)
            {
                primitive.indices[index] = value;
            });
        
        // -------------------- Position --------------------
        auto* positionIt = it->findAttribute("POSITION");
        assert(positionIt != it->attributes.end()); // A mesh primitive is required to hold the POSITION attribute.

        auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
        if (!positionAccessor.bufferViewIndex.has_value())
        {
            std::cerr << "[GLTFProcessor] Position accessor doesn't have a buffer view index" << std::endl;
            continue;
        }
        primitive.vertices.resize(positionAccessor.count);
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, positionAccessor,
            [&](fastgltf::math::fvec3 pos, std::size_t index)
            {
                Vertex& vertex = primitive.vertices[index];
                vertex.position = glm::vec3{ pos.x(), pos.y(), pos.z() };
                vertex.normal = glm::vec3{ 0.0f, 0.0f, 0.0f };
                vertex.tangent = glm::vec3{ 0.0f, 0.0f, 0.0f };
                for (glm::vec2& texcoord : vertex.texcoords)
                {
                    texcoord = glm::vec2{ 0.0f, 0.0f };
                }
                vertex.color0 = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
            });

        // -------------------- Normals --------------------
        auto* normalIt = it->findAttribute("NORMAL");
        if (normalIt == it->attributes.end())
        {
            std::cerr << "[GLTFProcessor] No normal attribute was found in GLTF model" << std::endl;
            primitive.hasNormals = false;
        } 
        else
        {
            auto& normalAccessor = asset.accessors[normalIt->accessorIndex];
            if (!normalAccessor.bufferViewIndex.has_value())
            {
                std::cerr << "[GLTFProcessor] Normal accessor doesn't have a buffer view index" << std::endl;
                primitive.hasNormals = false;
            }
            else
            {
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, normalAccessor,
                    [&](fastgltf::math::fvec3 norm, std::size_t index)
                    {
                        Vertex& vertex = primitive.vertices[index];
                        vertex.normal = glm::vec3{ norm.x(), norm.y(), norm.z() };
                    });
                primitive.hasNormals = true;
            }
        }

        // -------------------- Tangents --------------------
        auto* tangentIt = it->findAttribute("TANGENT");
        if (tangentIt == it->attributes.end())
        {
            std::cerr << "[GLTFProcessor] No tangent attribute was found in GLTF model" << std::endl;
            primitive.hasTangents = false;
        }
        else
        {
            auto& tangentAccessor = asset.accessors[tangentIt->accessorIndex];
            if (!tangentAccessor.bufferViewIndex.has_value())
            {
                std::cerr << "[GLTFProcessor] Tangent accessor doesn't have a buffer view index" << std::endl;
                primitive.hasTangents = false;
            }
            else
            {
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, tangentAccessor,
                    [&](fastgltf::math::fvec3 tang, std::size_t index)
                    {
                        Vertex& vertex = primitive.vertices[index];
                        vertex.tangent = glm::vec3{ tang.x(), tang.y(), tang.z() };
                    });
                primitive.hasTangents = true;
            }
        }

        // -------------------- Textures --------------------
        // TEXCOORD_0 must be valid since it's the fallback
        std::size_t baseColorTexcoordIndex = 0, normalTexcoordIndex = 0, metallicRoughnessTexcoordIndex = 0;

        if (it->materialIndex.has_value())
        {
            primitive.materialIndex = it->materialIndex.value() + 1; // 1-indexed
            auto& material = asset.materials[it->materialIndex.value()];

            // It's okay if any of these has_value()s fail, since the default material can be sampled with anything
            if (material.pbrData.baseColorTexture.has_value())
            {
                baseColorTexcoordIndex = getTexcoordIndex(material.pbrData.baseColorTexture.value(), material, asset);
            }
            if (material.normalTexture.has_value())
            {
                normalTexcoordIndex = getTexcoordIndex(material.normalTexture.value(), material, asset);
            }
            if (material.pbrData.metallicRoughnessTexture.has_value())
            {
                metallicRoughnessTexcoordIndex = getTexcoordIndex(material.pbrData.metallicRoughnessTexture.value(), material, asset);
            }
        }
        else
        {
            primitive.materialIndex = 0;
        }

        writeTexcoordData(baseColorTexcoordIndex, it, asset, primitive.vertices);
        writeTexcoordData(normalTexcoordIndex, it, asset, primitive.vertices);
        writeTexcoordData(metallicRoughnessTexcoordIndex, it, asset, primitive.vertices);

        // -------------------- Colors --------------------
        // Only supporting one color for now
        if (auto* colorIt = it->findAttribute("COLOR_0"); colorIt != it->attributes.end())
        {
            if (auto& colorAccessor = asset.accessors[colorIt->accessorIndex]; colorAccessor.bufferViewIndex.has_value())
            {
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(asset, colorAccessor,
                    [&](fastgltf::math::fvec4 color, std::size_t index)
                    {
                        Vertex& vertex = primitive.vertices[index];
                        vertex.color0 = glm::vec4{ color.x(), color.y(), color.z(), color.w()};
                    });
            }
        }
    }

    return request;
}

vk::PrimitiveTopology GLTFProcessor::toVkPrimitiveTopology(fastgltf::PrimitiveType type)
{
    switch (type)
    {
    case fastgltf::PrimitiveType::Triangles:
        return vk::PrimitiveTopology::eTriangleList;
    }
}

std::size_t GLTFProcessor::getTexcoordIndex(const fastgltf::TextureInfo& textureInfo, const fastgltf::Material& material, const fastgltf::Asset& asset)
{
    auto& texture = asset.textures[textureInfo.textureIndex];
    if (texture.imageIndex.has_value())
    {
        if (textureInfo.transform && textureInfo.transform->texCoordIndex.has_value())
        {
            return textureInfo.transform->texCoordIndex.value();
        }
        else
        {
            return material.pbrData.baseColorTexture->texCoordIndex;
        }
    }
    else
    {
        std::cerr << "[GLTFProcessor] Texture doesn't have a value for imageIndex" << std::endl;
    }
}

void GLTFProcessor::writeTexcoordData(std::size_t texcoordIndex, PrimitiveIterator it, const fastgltf::Asset& asset, std::vector<Vertex>& vertices)
{

    if (texcoordIndex >= Vertex::texcoordCount)
    {
        writeTexcoordData(0, it, asset, vertices);
    }
    auto texcoordAttribute = std::string("TEXCOORD_") + std::to_string(texcoordIndex);
    if (const auto* texcoord = it->findAttribute(texcoordAttribute); texcoord != it->attributes.end())
    {
        auto& texcoordAccessor = asset.accessors[texcoord->accessorIndex];
        if (!texcoordAccessor.bufferViewIndex.has_value())
        {
            writeTexcoordData(0, it, asset, vertices);
        }

        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset, texcoordAccessor,
            [&](fastgltf::math::fvec2 texcoordData, std::size_t index)
            {
                vertices[index].texcoords[texcoordIndex].x = texcoordData.x();
                vertices[index].texcoords[texcoordIndex].y = texcoordData.y();
            });
    }
    else
    {
        writeTexcoordData(0, it, asset, vertices);
    }
}
