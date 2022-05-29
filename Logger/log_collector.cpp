#include "log_collector.h"

LogCollector::LogCollector(const std::shared_ptr<ILogger>& logger)
	: mLogger(logger)
{
}

LogCollector::~LogCollector()
{
	mLogger->Write(mMessageStream.str());
}

bool LogCollector::IsDebugModeEnabled() const
{
	return mLogger->IsDebugModeEnabled();
}