#include "Application.h"
#include <vector>

const uint32_t WINDOW_WIDTH = 800;
const uint32_t WINDOW_HEIGHT = 600;

Application::Application()
{
	this->window = nullptr;
	this->instance = nullptr;
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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	this->window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Tutorial", nullptr, nullptr);


}

void Application::InitVulkan()
{
	this->CreateInstance();
}

void Application::MainLoop()
{
	while (!glfwWindowShouldClose(this->window))
	{
		glfwPollEvents();
	}
}

void Application::Cleanup()
{
	vkDestroyInstance(this->instance, nullptr);

	glfwDestroyWindow(this->window);
	glfwTerminate();
}

void Application::CreateInstance()
{
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
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::cout << "Dump of available GLFW extensions..." << std::endl;
	for (uint32_t i = 0; i < glfwExtensionCount; i++)
	{
		bool supportedByVKToo = false;
		for (const auto& vkExtension : vkExtensionsArray)
		{
			if (0 == ::strcmp(vkExtension.extensionName, glfwExtensions[i]))
			{
				supportedByVKToo = true;
				break;
			}
		}

		std::cout << '\t' << glfwExtensions[i] << " (" << (supportedByVKToo ? "VK supported too!" : "NOT VK supported!") << ")\n";
	}

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &instance))
		throw new std::runtime_error("Failed to creatre VK instance!");


}