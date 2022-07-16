#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

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

	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo;

public:
	VkBool32 HandleDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);
};