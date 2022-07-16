#include "Application.h"
#include <vector>

const uint32_t WINDOW_WIDTH = 800;
const uint32_t WINDOW_HEIGHT = 600;

const std::vector<const char*> desiredValidationLayersArray = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enbaleValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	Application* app = static_cast<Application*>(pUserData);
	return app->HandleDebugMessage(messageSeverity, messageType, pCallbackData);
}

Application::Application()
{
	this->window = nullptr;
	this->instance = nullptr;
	this->debugMessenger = nullptr;

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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	this->window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Tutorial", nullptr, nullptr);


}

void Application::InitVulkan()
{
	this->CreateInstance();
	this->SetupDebugMessenger();
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
	if (enableValidationLayers)
	{
		auto vkDestroyDebugUtilsMessangerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkDestroyDebugUtilsMessengerEXT");
		if(vkDestroyDebugUtilsMessangerEXT != nullptr)
			vkDestroyDebugUtilsMessangerEXT(this->instance, this->debugMessenger, nullptr);
	}

	vkDestroyInstance(this->instance, nullptr);

	glfwDestroyWindow(this->window);
	glfwTerminate();
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