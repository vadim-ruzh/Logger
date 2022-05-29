#pragma once

class IDebugModeDependentEntity
{
public:
	virtual ~IDebugModeDependentEntity() = default;
	[[nodiscard]] virtual bool IsDebugModeEnabled() const = 0;
};