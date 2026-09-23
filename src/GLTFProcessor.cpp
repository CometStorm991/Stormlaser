#include "GLTFProcessor.hpp"

std::vector<TextureProcessRequest> GLTFProcessor::processImages(const fastgltf::Asset& asset)
{
    std::vector<TextureProcessRequest> textureProcessRequests;
    for (const fastgltf::Image& image : asset.images)
    {
        textureProcessRequests.push_back(processImage(image, asset));
    }
    return textureProcessRequests;
}

TextureProcessRequest GLTFProcessor::processImage(const fastgltf::Image& image, const fastgltf::Asset& asset)
{
    int width, height, nrChannels;
    unsigned char* data;

    std::visit(fastgltf::visitor{
        [&](fastgltf::sources::URI& filePath) {
            assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
            assert(filePath.uri.isLocalPath()); // We're only capable of loading local files.

            const std::string path(filePath.uri.path().begin(), filePath.uri.path().end()); // Thanks C++.
            data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
        },
        [&](fastgltf::sources::Array& vector) {
            data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data()), static_cast<int>(vector.bytes.size()), &width, &height, &nrChannels, STBI_rgb_alpha);
        },
        [&](fastgltf::sources::BufferView& view) {
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

    return TextureProcessRequest{
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .nrChannels = static_cast<uint32_t>(nrChannels),
        .format = vk::Format::eR8G8B8A8Srgb,
        .generateMipmaps = false
    };
}