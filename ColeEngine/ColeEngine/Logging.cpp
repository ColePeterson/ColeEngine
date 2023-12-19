#include "Logging.h"
#include "windows.h"




// White message
void Log::msg(std::string msg)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	std::cout << msg << "\n";
}

// Yellow message
void Log::warning(std::string msg)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
	std::cout << msg << "\n";
}

// Red message
void Log::error(std::string msg)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
	std::cout << msg << "\n";
}

// Red message, blue background
void Log::fatal(std::string msg)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 20);
	std::cout << msg << "\n";
}

// Green message
void Log::info(std::string msg)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
	std::cout << msg << "\n";
}


