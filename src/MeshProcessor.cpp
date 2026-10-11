#include "MeshProcessor.hpp"

MeshProcessor::MeshProcessor(vk::PhysicalDevice physicalDevice, const vk::raii::Device& device, vk::Queue queue, vk::CommandPool commandPool)
	: bufferProcessor(physicalDevice, device, queue, commandPool) {}

VulkanMeshes MeshProcessor::processMeshes(const std::vector<MeshProcessRequest>& meshProcessRequests)
{
	
	vk::DeviceSize totalVertexSize = 0;
	vk::DeviceSize totalIndexSize = 0;
	uint32_t totalPrimitiveCount = 0;
	for (const MeshProcessRequest& request : meshProcessRequests)
	{
		for (const Primitive& primitive : request.primitives)
		{
			totalVertexSize += primitive.vertices.size() * sizeof(Vertex);
			totalIndexSize += primitive.indices.size() * sizeof(uint32_t);
			totalPrimitiveCount++;
		}
	}

	auto [vertexStagingBuffer, vertexStagingMemory] = bufferProcessor.createBuffer(
		totalVertexSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	auto [indexStagingBuffer, indexStagingMemory] = bufferProcessor.createBuffer(
		totalIndexSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* vertexDataStaging = vertexStagingMemory.mapMemory(0, totalVertexSize);
	void* indexDataStaging = indexStagingMemory.mapMemory(0, totalIndexSize);

	auto [vertexBuffer, vertexBufferMemory] = bufferProcessor.createBuffer(
		totalVertexSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
	auto [indexBuffer, indexBufferMemory] = bufferProcessor.createBuffer(
		totalIndexSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::DeviceSize vertexOffset = 0;
	vk::DeviceSize indexOffset = 0;
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
	std::vector<VulkanMeshRange> ranges;
	ranges.reserve(totalPrimitiveCount);
	for (const MeshProcessRequest& request : meshProcessRequests)
	{
		for (const Primitive& primitive : request.primitives)
		{
			const std::vector<Vertex>& vertices = primitive.vertices;
			const std::vector<uint32_t> indices = primitive.indices;
			vk::DeviceSize verticesSize = vertices.size() * sizeof(Vertex);
			vk::DeviceSize indicesSize = indices.size() * sizeof(uint32_t);
			memcpy(static_cast<char*>(vertexDataStaging) + vertexOffset, vertices.data(), vertices.size() * sizeof(Vertex));
			memcpy(static_cast<char*>(indexDataStaging) + indexOffset, indices.data(), indices.size() * sizeof(uint32_t));

			ranges.push_back(VulkanMeshRange{ 
				indexCount, static_cast<uint32_t>(indices.size()), static_cast<int>(vertexCount),
				primitive.materialIndex,
				primitive.baseColorTexcoordIndex, primitive.normalTexcoordIndex, primitive.metallicRoughnessTexcoordIndex,
				primitive.hasNormals, primitive.hasTangents });

			vertexCount += vertices.size();
			indexCount += indices.size();
			vertexOffset += verticesSize;
			indexOffset += indicesSize;
		}
	}
	vertexStagingMemory.unmapMemory();
	indexStagingMemory.unmapMemory();

	bufferProcessor.copyBuffer(vertexStagingBuffer, vertexBuffer, totalVertexSize);
	bufferProcessor.copyBuffer(indexStagingBuffer, indexBuffer, totalIndexSize);

	return VulkanMeshes{
		.vertexBuffer = std::move(vertexBuffer),
		.vertexMemory = std::move(vertexBufferMemory),
		.indexBuffer = std::move(indexBuffer),
		.indexMemory = std::move(indexBufferMemory),
		.meshRanges = std::move(ranges)
	};
}

