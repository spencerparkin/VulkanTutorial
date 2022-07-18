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

private:
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
	void CreateCommandPool();
	void CreateCommandBuffer();
	void RecordCommandBuffer(VkCommandBuffer givenCommandBuffer, uint32_t imageIndex);

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

		bool IsComplete()
		{
			return this->graphicsFamily.has_value() &&
				this->presentFamily.has_value();
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
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;

public:
	VkBool32 HandleDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);
};