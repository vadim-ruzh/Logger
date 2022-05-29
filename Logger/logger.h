#pragma once

#include <sstream>
#include <memory>
#include <mutex>
#include <string_view>
#include <boost/filesystem.hpp>
#include "i_logger.h"


class  Logger
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
