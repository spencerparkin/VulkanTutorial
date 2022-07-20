#include "Application.h"

int main()
{
	Application app;

	try
	{
		app.Run();
	}
	catch (std::exception* e)
	{
		std::cerr << e->what() << std::endl;
		delete e;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}