#include "Application.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const uint32_t WINDOW_WIDTH = 800;
const uint32_t WINDOW_HEIGHT = 600;

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};

const std::vector<const char*> desiredValidationLayersArray = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> desiredDeviceExtensionsArray = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enbaleValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const int MAX_FRAMES_IN_FLIGHT = 2;

static std::vector<char> ReadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		throw new std::runtime_error("Failed to open file!");

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	Application* app = static_cast<Application*>(pUserData);
	return app->HandleDebugMessage(messageSeverity, messageType, pCallbackData);
}

static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->frameBufferResized = true;
}

Application::Application()
{
	this->window = nullptr;
	this->instance = VK_NULL_HANDLE;
	this->debugMessenger = VK_NULL_HANDLE;
	this->physicalDevice = VK_NULL_HANDLE;
	this->logicalDevice = VK_NULL_HANDLE;
	this->graphicsQueue = VK_NULL_HANDLE;
	this->presentQueue = VK_NULL_HANDLE;
	this->transferQueue = VK_NULL_HANDLE;
	this->surface = VK_NULL_HANDLE;
	this->swapChain = VK_NULL_HANDLE;
	this->pipelineLayout = VK_NULL_HANDLE;
	this->renderPass = VK_NULL_HANDLE;
	this->descriptorSetLayout = VK_NULL_HANDLE;
	this->graphicsPipeline = VK_NULL_HANDLE;
	this->graphicsCommandPool = VK_NULL_HANDLE;
	this->transferCommandPool = VK_NULL_HANDLE;
	this->frameCount = 0;
	this->frameBufferResized = false;
	this->vertexBuffer = VK_NULL_HANDLE;
	this->vertexBufferMemory = nullptr;
	this->indexBuffer = VK_NULL_HANDLE;
	this->indexBufferMemory = nullptr;
	this->descriptorPool = VK_NULL_HANDLE;
	this->textureImage = VK_NULL_HANDLE;
	this->textureImageMemory = nullptr;
	this->textureImageView = VK_NULL_HANDLE;
	this->textureSampler = VK_NULL_HANDLE;

	::memset(&this->debugUtilsMessengerCreateInfo, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
	this->debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	this->debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	this->debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	this->debugUtilsMessengerCreateInfo.pfnUserCallback = &DebugCallback;
	this->debugUtilsMessengerCreateInfo.pUserData = this;
}

/*virtual*/ Application::~Application()
{
}

void Application::Run()
{
	this->InitWindow();
	this->InitVulkan();
	this->MainLoop();
	this->Cleanup();
}

void Application::InitWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	this->window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Tutorial", nullptr, nullptr);
	glfwSetWindowUserPointer(this->window, this);
	glfwSetFramebufferSizeCallback(this->window, &FrameBufferResizeCallback);
}

void Application::InitVulkan()
{
	this->CreateInstance();
	this->SetupDebugMessenger();
	this->CreateSurface();	// Do this before selecting GPU, because the surface capabilities can influence our GPU choice.
	this->PickPhsyicalDevice();
	this->CreateLogicalDevice();
	this->CreateSwapChain();
	this->CreateImageViews();
	this->CreateRenderPass();
	this->CreateDescriptorSetLayout();
	this->CreateGraphicsPipeline();
	this->CreateFramebuffers();
	this->CreateCommandPools();
	this->CreateTextureImage();
	this->CreateTextureImageView();
	this->CreateTextureSampler();
	this->CreateVertexBuffer();
	this->CreateIndexBuffer();
	this->CreateUniformBuffer();
	this->CreateDescriptorPool();
	this->CreateDescriptorSets();
	this->CreateCommandBuffers();
	this->CreateSyncObjects();
}

void Application::MainLoop()
{
	while (!glfwWindowShouldClose(this->window))
	{
		glfwPollEvents();
		this->DrawFrame();
	}

	vkDeviceWaitIdle(this->logicalDevice);
}

void Application::Cleanup()
{
	vkDestroyBuffer(this->logicalDevice, this->vertexBuffer, nullptr);
	vkFreeMemory(this->logicalDevice, this->vertexBufferMemory, nullptr);		// Now we can free the memory since it is no longer bound.

	vkDestroyBuffer(this->logicalDevice, this->indexBuffer, nullptr);
	vkFreeMemory(this->logicalDevice, this->indexBufferMemory, nullptr);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(this->logicalDevice, this->imageAvailableSemaphore[i], nullptr);
		vkDestroySemaphore(this->logicalDevice, this->renderFinishedSemaphore[i], nullptr);
		vkDestroyFence(this->logicalDevice, this->inFlightFence[i], nullptr);
		vkDestroyBuffer(this->logicalDevice, this->uniformBuffers[i], nullptr);
		vkFreeMemory(this->logicalDevice, this->uniformBuffersMemory[i], nullptr);
	}

	vkDestroyCommandPool(this->logicalDevice, this->graphicsCommandPool, nullptr);
	vkDestroyCommandPool(this->logicalDevice, this->transferCommandPool, nullptr);

	this->CleanupSwapChain();

	vkDestroySampler(this->logicalDevice, this->textureSampler, nullptr);
	vkDestroyImageView(this->logicalDevice, this->textureImageView, nullptr);
	vkDestroyImage(this->logicalDevice, this->textureImage, nullptr);
	vkFreeMemory(this->logicalDevice, this->textureImageMemory, nullptr);
	vkDestroyDescriptorPool(this->logicalDevice, this->descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(this->logicalDevice, this->descriptorSetLayout, nullptr);
	vkDestroyPipeline(this->logicalDevice, this->graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(this->logicalDevice, this->pipelineLayout, nullptr);
	vkDestroyRenderPass(this->logicalDevice, this->renderPass, nullptr);

	vkDestroyDevice(this->logicalDevice, nullptr);

	if (enableValidationLayers)
	{
		auto vkDestroyDebugUtilsMessangerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkDestroyDebugUtilsMessengerEXT");
		if(vkDestroyDebugUtilsMessangerEXT != nullptr)
			vkDestroyDebugUtilsMessangerEXT(this->instance, this->debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
	vkDestroyInstance(this->instance, nullptr);

	glfwDestroyWindow(this->window);
	glfwTerminate();
}

void Application::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = this->BeginSingleTimeCommands(this->graphicsCommandPool);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	this->EndSingleTimeCommands(commandBuffer, this->graphicsCommandPool, this->graphicsQueue);
}

void Application::CreateTextureSampler()
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_NEAREST; // VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_NEAREST; // VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (VK_SUCCESS != vkCreateSampler(this->logicalDevice, &samplerInfo, nullptr, &this->textureSampler))
		throw new std::runtime_error("Failed to create texture sampler!");
}

void Application::CreateTextureImageView()
{
	this->textureImageView = this->CreateImageView(this->textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

VkImageView Application::CreateImageView(VkImage image, VkFormat format)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	VkImageView imageView = VK_NULL_HANDLE;
	if (VK_SUCCESS != vkCreateImageView(this->logicalDevice, &viewInfo, nullptr, &imageView))
		throw new std::runtime_error("Failed to create texture image view!");

	return imageView;
}

void Application::CreateTextureImage()
{
	int texWidth = 0, texHeight = 0, texChannels = 0;
	stbi_uc* pixels = stbi_load("texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels)
		throw new std::runtime_error("Failed to load texture image!");

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	VkBuffer stagingBuffer= VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = nullptr;

	this->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(this->logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(this->logicalDevice, stagingBufferMemory);

	stbi_image_free(pixels);

	this->CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->textureImage, this->textureImageMemory);

	// TODO: After getting the texture mapping working, revisit this and see if you can do it all in one command buffer.
	//       For now, three command buffers are used and executed synchronously, but using one and doing it asynchronously is more efficient.

	this->TransitionImageLayout(this->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	this->CopyBufferToImage(stagingBuffer, this->textureImage, (uint32_t)texWidth, (uint32_t)texHeight);

	this->TransitionImageLayout(this->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(this->logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(this->logicalDevice, stagingBufferMemory, nullptr);
}

void Application::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // Optional

	if (VK_SUCCESS != vkCreateImage(this->logicalDevice, &imageInfo, nullptr, &image))
		throw new std::runtime_error("Failed to create image!");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(this->logicalDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = this->FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (VK_SUCCESS != vkAllocateMemory(this->logicalDevice, &allocInfo, nullptr, &imageMemory))
		throw new std::runtime_error("Failed to allocate image memory!");

	vkBindImageMemory(this->logicalDevice, image, imageMemory, 0);
}

void Application::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = this->BeginSingleTimeCommands(this->graphicsCommandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
		throw new std::invalid_argument("Unsupported layout transition!");

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	this->EndSingleTimeCommands(commandBuffer, this->graphicsCommandPool, this->graphicsQueue);
}

void Application::CreateSurface()
{
	if (VK_SUCCESS != glfwCreateWindowSurface(this->instance, this->window, nullptr, &surface))
		throw new std::runtime_error("Failed to create window surface!");
}

void Application::CreateLogicalDevice()
{
	QueueFamilyIndices indices = this->FindQueueFamilies(this->physicalDevice);

	float queuePriority = 1.0f;

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfosArray;
	std::set<uint32_t> uniqueQueueFamiliesIndexSet = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() };

	for (uint32_t queueFamilyIndex : uniqueQueueFamiliesIndexSet)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfosArray.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfosArray.data();
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfosArray.size();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = (uint32_t)desiredDeviceExtensionsArray.size();
	createInfo.ppEnabledExtensionNames = desiredDeviceExtensionsArray.data();

	if (enableValidationLayers)
	{
		// Not really necessary for newest versions of VK.
		createInfo.enabledLayerCount = (uint32_t)desiredValidationLayersArray.size();
		createInfo.ppEnabledLayerNames = desiredValidationLayersArray.data();
	}

	if (VK_SUCCESS != vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->logicalDevice))
		throw new std::runtime_error("Failed to create logical device!");

	vkGetDeviceQueue(this->logicalDevice, indices.graphicsFamily.value(), 0, &this->graphicsQueue);
	vkGetDeviceQueue(this->logicalDevice, indices.presentFamily.value(), 0, &this->presentQueue);
	vkGetDeviceQueue(this->logicalDevice, indices.transferFamily.value(), 0, &this->transferQueue);
}

void Application::PickPhsyicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);
	if (deviceCount == 0)
		throw new std::runtime_error("Failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devicesArray(deviceCount);
	vkEnumeratePhysicalDevices(this->instance, &deviceCount, devicesArray.data());

	VkPhysicalDevice chosenDevice = VK_NULL_HANDLE;
	for (const auto& candidateDevice : devicesArray)
	{
		if (this->IsDeviceSuitable(candidateDevice))
		{
			chosenDevice = candidateDevice;
			break;
		}
	}

	if (chosenDevice == VK_NULL_HANDLE)
		throw new std::runtime_error("Failed to find a suitable GPU!");

	this->physicalDevice = chosenDevice;
}

bool Application::IsDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(device, VK_FORMAT_R8G8B8A8_SRGB, &formatProperties);

	bool linearSamplingAvailable = 0 != (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);

	QueueFamilyIndices indices = this->FindQueueFamilies(device);
	
	bool extensionsSupported = this->CheckDeviceExtensionsSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails details = this->QuerySwapChainSupport(device);
		swapChainAdequate = !details.formatsArray.empty() && !details.presentModesArray.empty();
	}

	return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy && linearSamplingAvailable;
}

bool Application::CheckDeviceExtensionsSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensionsArray(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensionsArray.data());

	std::set<std::string> desiredExtensions(desiredDeviceExtensionsArray.begin(), desiredDeviceExtensionsArray.end());

	for (const auto& extension : availableExtensionsArray)
		desiredExtensions.erase(extension.extensionName);

	return desiredExtensions.empty();
}

VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;

	return availableFormats[0];
}

VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(this->window, &width, &height);
		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
	}
}

void Application::RecreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(this->logicalDevice);

	this->CleanupSwapChain();

	this->CreateSwapChain();
	this->CreateImageViews();
	this->CreateFramebuffers();
}

void Application::CleanupSwapChain()
{
	for (auto framebuffer : this->swapChainFramebuffers)
		vkDestroyFramebuffer(this->logicalDevice, framebuffer, nullptr);

	this->swapChainFramebuffers.clear();

	for (auto imageView : this->swapChainImageViews)
		vkDestroyImageView(this->logicalDevice, imageView, nullptr);

	this->swapChainImages.clear();

	vkDestroySwapchainKHR(this->logicalDevice, this->swapChain, nullptr);

	this->swapChain = VK_NULL_HANDLE;
}

void Application::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = this->QuerySwapChainSupport(this->physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = this->ChooseSwapSurfaceFormat(swapChainSupport.formatsArray);
	VkPresentModeKHR presentMode = this->ChooseSwapPresentMode(swapChainSupport.presentModesArray);
	this->swapChainExtent = this->ChooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	
	// Note that zero here would mean that there is no limit to the number of images in the swap-chain.
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;
	
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = this->surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = swapChainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = this->FindQueueFamilies(this->physicalDevice);
	std::vector<uint32_t> queueFamilyIndices;
	std::set<uint32_t> uniqueQueueFamiliesIndexSet = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() };
	if (uniqueQueueFamiliesIndexSet.size() > 1)
	{
		for (uint32_t uniqueFamilyIndex : uniqueQueueFamiliesIndexSet)
			queueFamilyIndices.push_back(uniqueFamilyIndex);

		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;	// No transfermation needed, so use current.
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;		// Will need to deal with this if the window was re-sizable.

	if (VK_SUCCESS != vkCreateSwapchainKHR(this->logicalDevice, &createInfo, nullptr, &this->swapChain))
		throw new std::runtime_error("Failed to create swap-chain!");

	vkGetSwapchainImagesKHR(this->logicalDevice, this->swapChain, &imageCount, nullptr);
	this->swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(this->logicalDevice, this->swapChain, &imageCount, swapChainImages.data());

	this->swapChainImageFormat = surfaceFormat.format;
}

void Application::CreateImageViews()
{
	this->swapChainImageViews.resize(this->swapChainImages.size());
	for (size_t i = 0; i < this->swapChainImages.size(); i++)
		this->swapChainImageViews[i] = this->CreateImageView(this->swapChainImages[i], this->swapChainImageFormat);
}

void Application::CreateIndexBuffer()
{
	this->CreateGeneralBuffer(indices.data(), sizeof(indices[0]) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, this->indexBuffer, this->indexBufferMemory);
}

void Application::CreateVertexBuffer()
{
	this->CreateGeneralBuffer(vertices.data(), sizeof(vertices[0]) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, this->vertexBuffer, this->vertexBufferMemory);
}

void Application::CreateGeneralBuffer(const void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkBuffer& targetBuffer, VkDeviceMemory& targetBufferMemory)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	this->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(this->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	::memcpy(data, bufferData, (size_t)bufferSize);
	vkUnmapMemory(this->logicalDevice, stagingBufferMemory);

	// Can't use vkMapMemory on this buffer, because it is only GPU accessible and therefore quicker access for the GPU.
	this->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, targetBuffer, targetBufferMemory);

	this->CopyBuffer(stagingBuffer, targetBuffer, bufferSize);

	vkDestroyBuffer(this->logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(this->logicalDevice, stagingBufferMemory, nullptr);
}

VkCommandBuffer Application::BeginSingleTimeCommands(VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(this->logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Application::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue queue)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(this->logicalDevice, commandPool, 1, &commandBuffer);
}

void Application::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = this->BeginSingleTimeCommands(this->transferCommandPool);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	this->EndSingleTimeCommands(commandBuffer, this->transferCommandPool, this->transferQueue);
}

void Application::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (VK_SUCCESS != vkCreateBuffer(this->logicalDevice, &bufferInfo, nullptr, &buffer))
		throw new std::runtime_error("Failed to create buffer!");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(this->logicalDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = this->FindMemoryType(memRequirements.memoryTypeBits, properties);

	// Note that the tutorial says that in practice, you should not call vkAllocateMemory for every
	// single resource (e.g., buffer), because it can only be called a limited number of times.
	// Rather, you should allocate a big huge chunk up front, and then manage the memory yourself
	// as you create buffers and other things.  Look at VulkanMemoryAllocator for help doing this.
	if (VK_SUCCESS != vkAllocateMemory(this->logicalDevice, &allocInfo, nullptr, &bufferMemory))
		throw new std::runtime_error("Failed to allocate buffer memory!");

	vkBindBufferMemory(this->logicalDevice, buffer, bufferMemory, 0);
}

uint32_t Application::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		if (0 != (typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;

	throw std::runtime_error("failed to find suitable memory type!");
}

void Application::CreateGraphicsPipeline()
{
	auto vertShaderCode = ReadFile("vert.spv");
	auto fragShaderCode = ReadFile("frag.spv");

	VkShaderModule vertShaderModule = this->CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = this->CreateShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	std::vector<VkDynamicState> dynamicStatesArray =
	{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStatesArray.size());
	dynamicState.pDynamicStates = dynamicStatesArray.data();

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	/*VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;*/

	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)this->swapChainExtent.width;
	viewport.height = (float)this->swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	/*	PSUEDO CODE for blending...
	* if (blendEnable) {
		finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
		finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
	} else {
		finalColor = newColor;
	}
	finalColor = finalColor & colorWriteMask;
	*/

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// This is for shader uniforms -- variables that can be set dynamically at run-time to change
	// the behavior of vertex or pixel shaders, such as setting a transform matrix or texture sampler, etc.
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &this->descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
	if (VK_SUCCESS != vkCreatePipelineLayout(this->logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout))
		throw new std::runtime_error("Failed to create pipeline!");

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = this->pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(this->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->graphicsPipeline) != VK_SUCCESS)
		throw new std::runtime_error("Failed to create graphics pipeline!");

	// In pipeline creation, the shaders are taken to GPU-specific byte codes, I think.  In any case, we can remove these now.
	vkDestroyShaderModule(this->logicalDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(this->logicalDevice, fragShaderModule, nullptr);
}

void Application::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = this->swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;	// I think this is an index into the renderPassInfo.pAttachments array.
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Note that "layout(location = 0) out vec4 outColor" is referencing the array specified here!
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	if (VK_SUCCESS != vkCreateRenderPass(this->logicalDevice, &renderPassInfo, nullptr, &this->renderPass))
		throw new std::runtime_error("failed to create render pass!");
}

void Application::CreateFramebuffers()
{
	this->swapChainFramebuffers.resize(this->swapChainImageViews.size());

	for (size_t i = 0; i < this->swapChainImageViews.size(); i++)
	{
		VkImageView attachments[] =
		{
			this->swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = this->swapChainExtent.width;
		framebufferInfo.height = this->swapChainExtent.height;
		framebufferInfo.layers = 1;
		if (VK_SUCCESS != vkCreateFramebuffer(this->logicalDevice, &framebufferInfo, nullptr, &this->swapChainFramebuffers[i]))
			throw new std::runtime_error("Failed to create framebuffer!");
	}
}

void Application::CreateCommandPools()
{
	QueueFamilyIndices queueFamilyIndices = this->FindQueueFamilies(this->physicalDevice);

	VkCommandPoolCreateInfo graphicsPoolInfo{};
	graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	graphicsPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	graphicsPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (VK_SUCCESS != vkCreateCommandPool(this->logicalDevice, &graphicsPoolInfo, nullptr, &this->graphicsCommandPool))
		throw new std::runtime_error("Failed to create graphics command pool!");

	VkCommandPoolCreateInfo transferPoolInfo{};
	transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	transferPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	transferPoolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

	if (VK_SUCCESS != vkCreateCommandPool(this->logicalDevice, &transferPoolInfo, nullptr, &this->transferCommandPool))
		throw new std::runtime_error("Failed to create transfer command pool!");
}

void Application::CreateCommandBuffers()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = this->graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	this->commandBuffer.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (VK_SUCCESS != vkAllocateCommandBuffers(this->logicalDevice, &allocInfo, &this->commandBuffer[i]))
			throw new std::runtime_error("Failed to allocate command buffers!");
	}
}

void Application::RecordCommandBuffer(VkCommandBuffer givenCommandBuffer, uint32_t imageIndex, uint32_t i)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional
	if (VK_SUCCESS != vkBeginCommandBuffer(givenCommandBuffer, &beginInfo))
		throw new std::runtime_error("Failed to begin recording command buffer!");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = this->renderPass;
	renderPassInfo.framebuffer = this->swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = this->swapChainExtent;

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(givenCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(givenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);

	VkBuffer vertexBuffersArray[] = { this->vertexBuffer };
	VkDeviceSize offsetsArray[] = { 0 };
	vkCmdBindVertexBuffers(givenCommandBuffer, 0, 1, vertexBuffersArray, offsetsArray);

	vkCmdBindIndexBuffer(givenCommandBuffer, this->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(this->swapChainExtent.width);
	viewport.height = static_cast<float>(this->swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(givenCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(givenCommandBuffer, 0, 1, &scissor);

	vkCmdBindDescriptorSets(givenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineLayout, 0, 1, &this->descriptorSets[i], 0, nullptr);

	vkCmdDrawIndexed(givenCommandBuffer, (uint32_t)indices.size(), 1, 0, 0, 0);

	vkCmdEndRenderPass(givenCommandBuffer);

	if (VK_SUCCESS != vkEndCommandBuffer(givenCommandBuffer))
		throw new std::runtime_error("Failed to record command buffer!");
}

void Application::CreateSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	this->imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	this->renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	this->inFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->imageAvailableSemaphore[i]) != VK_SUCCESS ||
			vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->renderFinishedSemaphore[i]) != VK_SUCCESS ||
			vkCreateFence(this->logicalDevice, &fenceInfo, nullptr, &this->inFlightFence[i]) != VK_SUCCESS)
		{
			throw new std::runtime_error("Failed to create semaphores!");
		}
	}
}

void Application::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (VK_SUCCESS != vkCreateDescriptorSetLayout(this->logicalDevice, &layoutInfo, nullptr, &this->descriptorSetLayout))
		throw new std::runtime_error("Failed to create descriptor set layout!");
}

void Application::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (VK_SUCCESS != vkCreateDescriptorPool(this->logicalDevice, &poolInfo, nullptr, &this->descriptorPool))
		throw new std::runtime_error("Failed to create descriptor pool!");
}

void Application::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, this->descriptorSetLayout);
	
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	// These are freed when the pool is destroyed, so we don't have to free them.
	this->descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (VK_SUCCESS != vkAllocateDescriptorSets(this->logicalDevice, &allocInfo, descriptorSets.data()))
		throw new std::runtime_error("Failed to allocate descriptor sets!");

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = this->textureImageView;
		imageInfo.sampler = this->textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(this->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Application::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	
	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		this->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
}

void Application::UpdateUniformBuffer(uint32_t i)
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), float(swapChainExtent.width) / float(swapChainExtent.height), 0.1f, 10.0f);
	ubo.proj[1][1] *= -1.0f;		// Clip/projected space is up-side-down from the OpenGL standard.

	void* data;
	vkMapMemory(this->logicalDevice, this->uniformBuffersMemory[i], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(this->logicalDevice, uniformBuffersMemory[i]);
}

void Application::DrawFrame()
{
	this->frameCount++;

	uint32_t i = this->frameCount % MAX_FRAMES_IN_FLIGHT;

	this->UpdateUniformBuffer(i);

	// Wait for previous frame to finish.  Note that the fence was created signaled, so we don't wait before doing the first ever frame.
	vkWaitForFences(this->logicalDevice, 1, &this->inFlightFence[i], VK_TRUE, UINT64_MAX);

	// Passed in semaphore is signaled when the "presentation engine" is finished using the image.
	// The returned image index is the image in the swap chain we created that is ready for us to render into.
	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(this->logicalDevice, this->swapChain, UINT64_MAX, this->imageAvailableSemaphore[i], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		this->RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw new std::runtime_error("Faield to acquire swap chain image!");

	// Only reset if we know we're going to submit work that will signal the fence.
	vkResetFences(this->logicalDevice, 1, &this->inFlightFence[i]);

	vkResetCommandBuffer(this->commandBuffer[i], 0);
	this->RecordCommandBuffer(this->commandBuffer[i], imageIndex, i);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore[i] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer[i];
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[i] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (VK_SUCCESS != vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, this->inFlightFence[i]))
		throw new std::runtime_error("Failed to submit draw command buffer!");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChainsArray[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChainsArray;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	result = vkQueuePresentKHR(this->presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->frameBufferResized)
	{
		this->frameBufferResized = false;
		this->RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw new std::runtime_error("Failed to present swap chain image!");
}

VkShaderModule Application::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());	// Should already be properly aligned.

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	if (VK_SUCCESS != vkCreateShaderModule(this->logicalDevice, &createInfo, nullptr, &shaderModule))
		throw new std::runtime_error("Failed to create shader module!");

	return shaderModule;
}

Application::QueueFamilyIndices Application::FindQueueFamilies(VkPhysicalDevice device)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamiliesArray(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamiliesArray.data());

	QueueFamilyIndices indices;

	int i = 0;
	for (const auto& queueFamily : queueFamiliesArray)
	{
		if (0 != (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			indices.graphicsFamily = i;

		// Look for a queue family that supports transfer operations, but not graphics operations.
		// This is just for demonstration purposes, since any queue family supporting graphics operations will also
		// handle transfer operations, even if the transfer bit is not set.
		if (0 != (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && 0 == (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			indices.transferFamily = i;

		VkBool32 presentSupported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->surface, &presentSupported);
		if (presentSupported)
			indices.presentFamily = i;

		if (indices.IsComplete())
			break;

		i++;
	}

	return indices;
}

Application::SwapChainSupportDetails Application::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formatsArray.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formatsArray.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModesArray.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModesArray.data());
	}

	return details;
}

VkBool32 Application::HandleDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
{
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void Application::SetupDebugMessenger()
{
	if (enableValidationLayers)
	{
		auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT");
		if (vkCreateDebugUtilsMessengerEXT != nullptr)
		{
			if (VK_SUCCESS != vkCreateDebugUtilsMessengerEXT(this->instance, &this->debugUtilsMessengerCreateInfo, nullptr, &this->debugMessenger))
				throw new std::runtime_error("Failed to setup debug messenger!");
		}
	}
}

void Application::CreateInstance()
{
	if (enableValidationLayers && !this->CheckValidationLayerSupport())
		throw new std::runtime_error("Desired validation layers not available!");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Tutorial";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Find out what extensions VK supports...
	uint32_t vkExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);
	std::vector<VkExtensionProperties> vkExtensionsArray(vkExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, vkExtensionsArray.data());
	std::cout << "Dump of available VK extensions.." << std::endl;
	for (const auto& vkExtension : vkExtensionsArray)
		std::cout << '\t' << vkExtension.extensionName << '\n';

	// Find out what extensions GLFW supports...
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensionsArray = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::cout << "Dump of available GLFW extensions..." << std::endl;
	for (uint32_t i = 0; i < glfwExtensionCount; i++)
	{
		bool supportedByVKToo = false;
		for (const auto& vkExtension : vkExtensionsArray)
		{
			if (0 == ::strcmp(vkExtension.extensionName, glfwExtensionsArray[i]))
			{
				supportedByVKToo = true;
				break;
			}
		}

		std::cout << '\t' << glfwExtensionsArray[i] << " (" << (supportedByVKToo ? "VK supported too!" : "NOT VK supported!") << ")\n";
	}

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> extensionsArray(glfwExtensionsArray, glfwExtensionsArray + glfwExtensionCount);

	if (enableValidationLayers)
		extensionsArray.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	createInfo.enabledExtensionCount = (uint32_t)extensionsArray.size();
	createInfo.ppEnabledExtensionNames = extensionsArray.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(desiredValidationLayersArray.size());
		createInfo.ppEnabledLayerNames = desiredValidationLayersArray.data();
		createInfo.pNext = &this->debugUtilsMessengerCreateInfo;
	}
	
	if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &instance))
		throw new std::runtime_error("Failed to creatre VK instance!");
}

bool Application::CheckValidationLayerSupport()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayersArray(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayersArray.data());

	// Note that what validation layers are supported are configured using the HKLM/SOFTWARE/Khronos/Volkan/ExplicitLayers registry folder.
	// Keys in this folder point to JSON files in the installed Vulkan SDK folder, which in turn probably point to validation DLLs.
	std::cout << "Dump of supported validation layers...\n";
	for (const auto& layerProperties : availableLayersArray)
		std::cout << '\t' << layerProperties.layerName << '\n';

	std::cout << "Dump of desired validation layers...\n";
	uint32_t failCount = 0;
	for (const char* desiredLayerName : desiredValidationLayersArray)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayersArray)
		{
			if (0 == strcmp(desiredLayerName, layerProperties.layerName))
			{
				layerFound = true;
				break;
			}
		}

		std::cout << '\t' << desiredLayerName << " (" << (layerFound ? "Supported!" : "Not supported!") << ")\n";

		if (!layerFound)
			failCount++;
	}

	return failCount == 0;
}