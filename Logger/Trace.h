#pragma once

#include <iomanip>
#include <boost/date_time/posix_time/posix_time.hpp>

enum class LogLevel
{
	Debug,
	Info,
	Error,
};

inline const char* ToString(LogLevel lvl)
{
	switch (lvl)
	{
	case LogLevel::Debug: return "DBG";
	case LogLevel::Info: return "INF";
	case LogLevel::Error: return "ERR";
	default: return "UWN";
	}
}

#define TRACE(collector,lvl)\
	if((collector).IsDebugModeEnabled() == true && (lvl) == LogLevel::Debug){}\
	else\
	(collector)\
		<< "\n" << boost::posix_time::microsec_clock::local_time().time_of_day()\
		<< "\t" << std::hex << std::this_thread::get_id()\
		<< "\t" << ToString(lvl) \
		<< "\t" << __func__ << "\t"

#define TRACE_ERR(collector) TRACE(collector, LogLevel::Error)
#define TRACE_INF(collector) TRACE(collector, LogLevel::Info)
#define TRACE_DBG(collector) TRACE(collector, LogLevel::Debug)