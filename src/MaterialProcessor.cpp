#include "MaterialProcessor.hpp"

MaterialProcessor::MaterialProcessor(vk::PhysicalDevice physicalDevice, const vk::raii::Device& device, vk::Queue queue, vk::CommandPool commandPool)
	: bufferProcessor(physicalDevice, device, queue, commandPool) {}

VulkanMaterials MaterialProcessor::processMaterials(const std::vector<MaterialProcessRequest>& materialProcessRequests)
{
	// Using MaterialProcessRequest directly as Material struct for now

	vk::DeviceSize bufferSize = sizeof(MaterialProcessRequest) * materialProcessRequests.size();
	auto [stagingBuffer, stagingBufferMemory] = bufferProcessor.createBuffer(
		bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
	memcpy(dataStaging, materialProcessRequests.data(), bufferSize);
	stagingBufferMemory.unmapMemory();

	auto [materialBuffer, materialBufferMemory] = bufferProcessor.createBuffer(
		bufferSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferProcessor.copyBuffer(stagingBuffer, materialBuffer, bufferSize);

	return VulkanMaterials{
		std::move(materialBuffer), std::move(materialBufferMemory), bufferSize
	};
}

