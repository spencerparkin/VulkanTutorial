#include "Application.h"

const uint32_t WINDOW_WIDTH = 800;
const uint32_t WINDOW_HEIGHT = 600;

Application::Application()
{
	this->window = nullptr;
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
	glfwDestroyWindow(this->window);
	glfwTerminate();
}
