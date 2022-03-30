#pragma once

enum class LogLevel
{
	DEBUG,
	INFO,
	ERROR,
};

inline std::string LvlToString(LogLevel lvl)
{
	switch (lvl)
	{
	case LogLevel::DEBUG: return std::string{ "DBG" };
	case LogLevel::INFO: return std::string{ "INF" };
	case LogLevel::ERROR: return std::string{ "ERR" };
	}
}

#define TRACE(collector,lvl)\
	collector\
		<< "\n" << boost::posix_time::microsec_clock::local_time().time_of_day()\
		<< "\t" << std::hex << std::this_thread::get_id()\
		<< "\t" <<LvlToString(lvl) \
		<< "\t" <<__func__ << "\t"


