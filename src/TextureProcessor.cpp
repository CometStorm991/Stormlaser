#include "TextureProcessor.hpp"

std::vector<VulkanTexture> TextureProcessor::processImages(const std::vector<TextureProcessRequest>& textureProcessRequests)
{
    vk::DeviceSize totalSize = 0;
    for (const TextureProcessRequest& textureProcessRequest : textureProcessRequests)
    {
        vk::DeviceSize paddedImageSize = getPaddedImageSize(textureProcessRequest.width, textureProcessRequest.height);
        totalSize += paddedImageSize;
    }

    auto [stagingBuffer, stagingBufferMemory] =
        createBuffer(totalSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = stagingBufferMemory.mapMemory(0, totalSize);
    
    vk::DeviceSize offset = 0;
    std::vector<VulkanTexture> vulkanTextures;
    std::vector<vk::ImageMemoryBarrier2> toDstBarriers;
    std::vector<vk::ImageMemoryBarrier2> toReadBarriers;
    for (const TextureProcessRequest& textureProcessRequest : textureProcessRequests)
    {
        vk::DeviceSize paddedImageSize = getPaddedImageSize(textureProcessRequest.width, textureProcessRequest.height);
        vk::DeviceSize imageSize = textureProcessRequest.width * textureProcessRequest.height * textureProcessRequest.nrChannels;
        memcpy(static_cast<char*>(data) + offset, textureProcessRequest.pixelData, imageSize);
        offset += paddedImageSize;
        
        auto [image, imageMemory] = createImage(textureProcessRequest.width,
            textureProcessRequest.height,
            textureProcessRequest.format,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal);
        
        //image must be std moved
        vk::raii::ImageView imageView = createImageView(image, textureProcessRequest.format, vk::ImageAspectFlagBits::eColor);

        toDstBarriers.push_back(createBarrier(image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal));
        toReadBarriers.push_back(createBarrier(image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal));

        vulkanTextures.emplace_back(std::move(image), std::move(imageView), textureProcessRequest.format, 1);
    }

    vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);
    commandBuffer.pipelineBarrier2({ .imageMemoryBarrierCount = static_cast<uint32_t>(toDstBarriers.size()), .pImageMemoryBarriers = toDstBarriers.data() });

    for (int i = 0; i < textureProcessRequests.size(); ++i)
    {
        vk::Image image = vulkanTextures[i].image;
        TextureProcessRequest textureProcessRequest = textureProcessRequests[i];
        copyBufferToImage(commandBuffer, stagingBuffer, image, static_cast<uint32_t>(textureProcessRequest.width), static_cast<uint32_t>(textureProcessRequest.height));
    }

    endSingleTimeCommands(std::move(commandBuffer), queue);

    stagingBufferMemory.unmapMemory();

    return vulkanTextures;
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> TextureProcessor::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::BufferCreateInfo bufferInfo{ .size = size,
                                .usage = usage,
                                .sharingMode = vk::SharingMode::eExclusive }; // eExclusive has to do with queues
    vk::raii::Buffer  buffer = vk::raii::Buffer(device, bufferInfo);

    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo memoryAllocateInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties) };

    vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, memoryAllocateInfo);
    buffer.bindMemory(*bufferMemory, 0);

    return { std::move(buffer), std::move(bufferMemory) };
}

uint32_t TextureProcessor::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

vk::DeviceSize TextureProcessor::getPaddedImageSize(int width, int height)
{
    return roundUp(width * height * desiredChannels, physicalDevice.getProperties().limits.optimalBufferCopyOffsetAlignment);
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory> TextureProcessor::createImage(
    uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::ImageCreateInfo imageInfo{ .imageType = vk::ImageType::e2D,
                                  .format = format,
                                  .extent = {width, height, 1},
                                  .mipLevels = 1,
                                  .arrayLayers = 1,
                                  .samples = vk::SampleCountFlagBits::e1,
                                  .tiling = tiling,
                                  .usage = usage,
                                  .sharingMode = vk::SharingMode::eExclusive };

    vk::raii::Image image = vk::raii::Image(device, imageInfo);

    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size,
                                     .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties) };
    vk::raii::DeviceMemory imageMemory = vk::raii::DeviceMemory(device, allocInfo);
    image.bindMemory(imageMemory, 0);

    return { std::move(image), std::move(imageMemory) };
}

vk::raii::CommandBuffer TextureProcessor::beginSingleTimeCommands(vk::CommandPool commandPool)
{
    vk::CommandBufferAllocateInfo allocInfo{ .commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
    vk::raii::CommandBuffer commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());

    vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
    commandBuffer.begin(beginInfo);

    return std::move(commandBuffer);
}

void TextureProcessor::endSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer, vk::Queue queue)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer };
    queue.submit(submitInfo, nullptr);
    queue.waitIdle();
}

vk::ImageMemoryBarrier2 TextureProcessor::createBarrier(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::AccessFlagBits2 srcAccessMask;
    vk::AccessFlagBits2 dstAccessMask;
    vk::PipelineStageFlagBits2 srcStageMask;
    vk::PipelineStageFlagBits2 dstStageMask;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        srcAccessMask = vk::AccessFlagBits2::eNone;
        dstAccessMask = vk::AccessFlagBits2::eTransferWrite;

        srcStageMask = vk::PipelineStageFlagBits2::eNone;
        dstStageMask = vk::PipelineStageFlagBits2::eCopy;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
        dstAccessMask = vk::AccessFlagBits2::eShaderSampledRead;

        srcStageMask = vk::PipelineStageFlagBits2::eCopy;
        dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    return vk::ImageMemoryBarrier2{
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = image,
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1} };
}

void TextureProcessor::copyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer, vk::Image image, uint32_t width, uint32_t height)
{
    vk::BufferImageCopy region{ .bufferOffset = 0,
                           .bufferRowLength = 0,
                           .bufferImageHeight = 0,
                           .imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
                           .imageOffset = {0, 0, 0},
                           .imageExtent = {width, height, 1} };

    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}

vk::raii::ImageView TextureProcessor::createImageView(vk::Image const& image, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageViewCreateInfo viewInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = {.aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1} };
    return vk::raii::ImageView(device, viewInfo);
}