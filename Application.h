#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <set>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <optional>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>

class Application
{
public:
	Application();
	virtual ~Application();

	void Run();
	void InitWindow();
	void InitVulkan();
	void MainLoop();
	void Cleanup();
	void CreateInstance();
	bool CheckValidationLayerSupport();
	void SetupDebugMessenger();
	void PickPhsyicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	void CreateLogicalDevice();
	void CreateSurface();
	bool CheckDeviceExtensionsSupport(VkPhysicalDevice device);
	void CreateSwapChain();
	void CreateImageViews();
	void CreateGraphicsPipeline();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreateCommandPools();
	void CreateCommandBuffers();
	void RecordCommandBuffer(VkCommandBuffer givenCommandBuffer, uint32_t imageIndex);
	void DrawFrame();
	void CreateSyncObjects();
	void RecreateSwapChain();
	void CleanupSwapChain();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateGeneralBuffer(const void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkBuffer& targetBuffer, VkDeviceMemory& targetBufferMemory);
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formatsArray;
		std::vector<VkPresentModeKHR> presentModesArray;
	};

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> transferFamily;

		bool IsComplete()
		{
			return this->graphicsFamily.has_value() &&
				this->presentFamily.has_value() &&
				this->transferFamily.has_value();
		}
	};

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo;
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool graphicsCommandPool;
	VkCommandPool transferCommandPool;
	std::vector<VkCommandBuffer> commandBuffer;
	std::vector<VkSemaphore> imageAvailableSemaphore;
	std::vector<VkSemaphore> renderFinishedSemaphore;
	std::vector<VkFence> inFlightFence;
	uint32_t frameCount;
	bool frameBufferResized;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBool32 HandleDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);
};