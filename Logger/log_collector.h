#pragma once
#include <memory>
#include <sstream>
#include "i_logger.h"


class LogCollector
	: public IDebugModeDependentEntity
{
public:
	LogCollector(const std::shared_ptr<ILogger>& logger);
	~LogCollector() override;

	template<typename T>
	LogCollector& operator<<(const T& message)
	{
		mMessageStream << message;
		return *this;
	}

	[[nodiscard]] bool IsDebugModeEnabled() const override;

private:
	std::shared_ptr<ILogger> mLogger;
	std::stringstream mMessageStream;
};
