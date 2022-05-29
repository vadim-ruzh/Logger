#pragma once

#ifdef LOGGER_LIBRARY
#define LOGGER_API __declspec(dllexport)
#else
#define LOGGER_API __declspec(dllimport)
#endif
