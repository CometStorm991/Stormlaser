#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <unordered_map>

#include <vulkan/vulkan_raii.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <stb_image.h>

#include "BindlessRegistry.hpp"
#include "CameraController.hpp"
#include "GLTFProcessor.hpp"
#include "TextureProcessor.hpp"
#include "VulkanTexture.hpp"
#include "Window.hpp"

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr uint32_t WIDTH = 1600;
constexpr uint32_t HEIGHT = 1280;
const std::string  MODEL_PATH = "assets/models/AyaModel/091_W_Aya_100K.obj";
const std::string  TEXTURE_PATH = "assets/models/AyaModel/tex/091_W_Aya_2K_01.jpg";

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<char const*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;


	static vk::VertexInputBindingDescription getBindingDescription()
	{
		return { .binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex };
	}

	static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		return { {{.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, pos)},
				 {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)},
				 {.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, texCoord) } } };
	}

	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};
namespace std
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class Application
{
public:
	void run()
	{
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	//GLFWwindow* window = nullptr;
	Window window{};
	CameraController camController{};

	vk::raii::Context context;

	vk::raii::Instance instance = nullptr;

	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

	vk::raii::SurfaceKHR surface = nullptr;

	vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::Device device = nullptr;
	std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };

	vk::raii::Queue queue = nullptr;
	uint32_t queueIndex;

	vk::raii::SwapchainKHR swapChain = nullptr;
	std::vector<vk::Image> swapChainImages;
	std::vector<vk::raii::ImageView> swapChainImageViews;
	vk::Extent2D swapChainExtent;
	vk::SurfaceFormatKHR swapChainSurfaceFormat;

	vk::raii::DescriptorSetLayout bindlessDescriptorSetLayout = nullptr;
	vk::raii::DescriptorSetLayout perFrameDescriptorSetLayout = nullptr;

	vk::raii::PipelineLayout pipelineLayout = nullptr;
	vk::raii::Pipeline graphicsPipeline = nullptr;

	vk::raii::CommandPool commandPool = nullptr;

	vk::raii::Image        depthImage = nullptr;
	vk::raii::DeviceMemory depthImageMemory = nullptr;
	vk::raii::ImageView    depthImageView = nullptr;
	vk::Format depthFormat = vk::Format::eUndefined;

	uint32_t maxTextures;
	std::vector<VulkanTexture> vulkanTextures;
	std::vector<uint32_t> textureSlots;
	BindlessRegistry bindlessRegistry;

	vk::raii::ImageView textureImageView = nullptr;
	vk::raii::Sampler textureSampler = nullptr;

	std::vector<Vertex> vertices;
	vk::raii::Buffer vertexBuffer = nullptr;
	vk::raii::DeviceMemory vertexBufferMemory = nullptr;
	std::vector<uint32_t> indices;
	vk::raii::Buffer       indexBuffer = nullptr;
	vk::raii::DeviceMemory indexBufferMemory = nullptr;

	std::vector<vk::raii::Buffer>       uniformBuffers;
	std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
	std::vector<void*>                 uniformBuffersMapped;

	vk::raii::DescriptorPool bindlessDescriptorPool = nullptr;
	std::vector<vk::raii::DescriptorSet> bindlessDescriptorSets;
	vk::raii::DescriptorPool perFrameDescriptorPool = nullptr;
	std::vector<vk::raii::DescriptorSet> perFrameDescriptorSets;

	std::vector<vk::raii::CommandBuffer> commandBuffers;

	std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
	std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
	std::vector<vk::raii::Fence> inFlightFences;

	uint32_t frameIndex = 0;

	bool framebufferResized = false;
	std::chrono::high_resolution_clock::time_point startTimePoint;
	std::chrono::high_resolution_clock::time_point previousFrameTimePoint = std::chrono::high_resolution_clock::now();



	void initVulkan();

	void createInstance();
	std::vector<const char*> getRequiredInstanceExtensions();

	void setupDebugMessenger();
	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
		vk::DebugUtilsMessageTypeFlagsEXT              type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	void createSurface();

	void pickPhysicalDevice();
	bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);

	void createLogicalDevice();

	void createSwapChain();
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
	vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
	vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities);
	uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);

	void createImageViews();
	vk::raii::ImageView createImageView(vk::Image const& image, vk::Format format, vk::ImageAspectFlags aspectFlags);

	void createDescriptorSetLayout();

	void createGraphicsPipeline();
	static std::vector<char> readFile(const std::string& filename);
	[[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;

	void createCommandPool();

	void createDepthResources();
	vk::Format findDepthFormat();
	vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	//void createTextureImage();
	std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(
		uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
	void transitionImageLayout(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	void copyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height);

	//void createTextureImageView();
	void createTextureSampler();

	bool loadGltf(std::filesystem::path path, fastgltf::Asset& asset);
	void loadModel();

	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffers();
	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
	void copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size);
	vk::raii::CommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer);
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	void createDescriptorPool();
	void createDescriptorSets();

	void createCommandBuffers();
	void recordCommandBuffer(uint32_t imageIndex);
	void transitionImageLayout(
		vk::Image image,
		vk::ImageLayout         old_layout,
		vk::ImageLayout         new_layout,
		vk::AccessFlags2        src_access_mask,
		vk::AccessFlags2        dst_access_mask,
		vk::PipelineStageFlags2 src_stage_mask,
		vk::PipelineStageFlags2 dst_stage_mask,
		vk::ImageAspectFlags imageAspectFlags
	);

	void createSyncObjects();



	void mainLoop();

	void drawFrame();

	void updateUniformBuffer(uint32_t currentImage, float time);

	void recreateSwapChain();
	void cleanupSwapChain();
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	bool checkMinimized();


	void cleanup();
};