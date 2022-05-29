#pragma once
#include <string_view>

class ILogger
{
public:
	virtual ~ILogger() = default;
	virtual void Write(std::string_view message) = 0;
};

class IDebugModeDependentEntity
{
public:
	virtual ~IDebugModeDependentEntity() = default;
	[[nodiscard]] virtual bool IsDebugModeEnabled() const = 0;
};
