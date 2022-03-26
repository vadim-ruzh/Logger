#pragma once
#include <boost/dll.hpp>
inline std::string GetExecutableName()
{
	return boost::dll::program_location().filename().replace_extension().string();
}