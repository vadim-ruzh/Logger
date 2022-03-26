#pragma once

enum class loglevel
{
	debug,
	info,
	error,
};

inline std::string leveltostring(loglevel level)
{
	switch (level)
	{
	case loglevel::debug: return std::string{ "dbg" };
	case loglevel::info: return std::string{ "inf" };
	case loglevel::error: return std::string{ "err" };
	default:return std::string{ "unknown" };
	}
}

#define TRACE(tracer,level)\
	tracer.get()\
		<< "\n" << boost::posix_time::microsec_clock::local_time().time_of_day()\
		<< "\t" << std::this_thread::get_id()\
		<< "\t" <<leveltostring(level) \
		<< "\t" <<__func__ << "\t"


