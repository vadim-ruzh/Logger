#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <fstream>
#include <thread>
#include <mutex>
#include <boost/date_time.hpp>
#include "ver_info.h"





enum class LogLevel
{
	DEBUG,
	INFO,
	ERROR,
};


std::string LevelToString(LogLevel level)
{
	switch (level)
	{
	case LogLevel::DEBUG: return std::string{ "DBG" };
	case LogLevel::INFO: return std::string{ "INF" };
	case LogLevel::ERROR: return std::string{ "ERR" };
	default:return "UNKNOWN";
	}
}

class Logger
{
public:
	Logger(const std::string& path) :m_pathToLog(InitLogFile(path))
	{
	}

	void WriteToFile(const std::string& str)
	{
		m_mutex.lock();

		boost::filesystem::ofstream mOfstream;
		mOfstream.rdbuf()->open(m_pathToLog.string(), std::ios_base::app, _SH_DENYWR);
		mOfstream << str;
		mOfstream.close();

		m_mutex.unlock();
	}

private:
	boost::filesystem::path InitLogFile(const std::string& path)
	{
		boost::filesystem::path tmp_path(path);

		if (exists(tmp_path))
		{
			if (is_regular_file(tmp_path))
			{
				return tmp_path;
			}
			if (is_directory(tmp_path))
			{
				return CreatePathToFile(tmp_path);
			}
		}

		return CreatePathToFile(DEFAULT_DIR);
	}

	boost::filesystem::path CreatePathToFile(const boost::filesystem::path& pathToDir)
	{
		boost::filesystem::path tmpPath;

		tmpPath.append(pathToDir);

		if (!exists(tmpPath))
		{
			create_directory(tmpPath);
		}

		std::stringstream fileName;
		fileName << "/" << PROGRAM_NAME << "_" << PROGRAM_VERSION << "_";
		const boost::posix_time::ptime date_time = boost::posix_time::second_clock::local_time();
		fileName.imbue(std::locale(fileName.getloc(), new boost::posix_time::time_facet("%d.%m.%Y_%H.%M")));
		fileName << date_time << ".log";

		tmpPath.append(fileName.str());

		boost::filesystem::fstream fstream;
		fstream.open(tmpPath);
		fstream.close();

		return tmpPath;
	}

	std::mutex m_mutex;
	boost::filesystem::path m_pathToLog;
};

class Collector
{
public:
	Collector(const std::shared_ptr<Logger> logger) : m_loggerPtr(logger)
	{

	}

	std::stringstream& get()
	{
		return  m_sstr;
	}

	~Collector()
	{
		m_loggerPtr->WriteToFile(m_sstr.str());
	}

private:
	std::shared_ptr<Logger> m_loggerPtr;
	std::stringstream m_sstr;
};


#define TRACE(tracer,level)\
	tracer.get()\
		<< "\n" << boost::posix_time::microsec_clock::local_time().time_of_day()\
		<< "\t" << std::this_thread::get_id()\
		<< "\t"<< LevelToString(level) \
		<< "\t" <<__func__ << "\t"

int main()
{
	// TEST
	// TEST 2
	std::string path("./logg/text.txt");
	auto wr_ptr = std::make_shared<Logger>(path);

	Collector abc(wr_ptr);

	TRACE(abc, LogLevel::INFO) << " adadasdasd";
	TRACE(abc, LogLevel::ERROR) << " adadasdasd";
	TRACE(abc, LogLevel::DEBUG) << " adadasdasd";



	std::cout << "\n---------------------------------------------------" << std::endl;


	std::cout << "Current time with second resolution: " << boost::posix_time::microsec_clock::local_time().time_of_day() << std::endl;

	return 0;
}
