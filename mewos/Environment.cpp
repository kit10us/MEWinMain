#include <mewos/Environment.h>

using namespace mewos;

std::string Environment::GetName() const
{
	return "windows";
}

std::string Environment::GetPlatform() const
{
#if _WIN64
	return "x64";
#elif _WIN32
	return "Win32";
#endif
}

