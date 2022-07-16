#pragma once

#include <vulkan/vulkan.h>

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
	void InitVulkan();
	void MainLoop();
	void Cleanup();
};