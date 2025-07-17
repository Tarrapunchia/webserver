#include "ServerMan.hpp"

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Wrong input!";
		return 0;
	}
	try
	{
		ServerMan serverMan(argv[1]);
		serverMan.server_init();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}
