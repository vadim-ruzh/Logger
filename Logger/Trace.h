#pragma once

enum class LogLevel
{
	debug,
	info,
	error
};

inline std::string LvlToString(LogLevel lvl)
{
	switch (lvl)
	{
	case LogLevel::debug: return std::string{ "DBG" };
	case LogLevel::info: return std::string{ "INF" };
	case LogLevel::error: return std::string{ "ERR" };
	}
}


#define TRACE(collector,lvl)\
	if(collector.dbgMode == true && lvl == LogLevel::debug){}\
	else\
	collector\
		<< "\n" << boost::posix_time::microsec_clock::local_time().time_of_day()\
		<< "\t" << std::hex << std::this_thread::get_id()\
		<< "\t" <<LvlToString(lvl) \
		<< "\t" <<__func__ << "\t"
