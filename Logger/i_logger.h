#pragma once

#include <string_view>
#include "i_debug_mode_dependent_entity.h"

class ILogger
	: public IDebugModeDependentEntity
{
public:
	~ILogger() override = default;
	virtual void Write(std::string_view message) = 0;
};
