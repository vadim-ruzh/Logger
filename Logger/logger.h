#pragma once

#include <sstream>
#include <memory>
#include <mutex>
#include <string_view>
#include <boost/filesystem.hpp>
#include "ILogger.h"

class Logger
	: public ILogger
	, public IDebugModeDependentEntity
{
public:
	Logger(std::string_view programName);
	Logger();
	~Logger() override;

	void Write(std::string_view message) override;
	[[nodiscard]] bool IsDebugModeEnabled() const override;

private:
	std::mutex mMutex;
	boost::filesystem::path mPathToLog;
	bool mIsDebugModeEnabled;
};

class Collector
	: public IDebugModeDependentEntity
{
public:
	Collector(const std::shared_ptr<Logger>& logger);
	~Collector() override;

	template<typename T>
	Collector& operator<<(const T& message)
	{
		mMessageStream << message;
		return *this;
	}

	[[nodiscard]] bool IsDebugModeEnabled() const override;
private:

	std::shared_ptr<Logger> mLogger;
	std::stringstream mMessageStream;
};