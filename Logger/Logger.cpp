#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <fstream>
#include <thread>
#include <mutex>
#include <boost/date_time.hpp>
#include "ver_info.h"


#define DEFAULT_DIR  boost::filesystem::path("./Log")


enum class LogLevel
{
	DEBUG,
	INFO,
	ERROR,
	FATAL,
};

std::string LevelToString(LogLevel level)
{
	switch (level)
	{
		case LogLevel::DEBUG: return std::string{ "DEBUG" };
		case LogLevel::INFO: return std::string{ "INFO" };
		case LogLevel::ERROR: return std::string{ "ERROR" };
		case LogLevel::FATAL: return std::string{ "FATAL" };
		default:return "UKNOWN";
	}
}

//Выводит информацию
class Logger
{

public:


	//TODO :: добавить очередь задач 

	Logger(boost::filesystem::path path) :m_pathToLog(InitLogFile(path))
	{}

	Logger()
	{}

	boost::filesystem::path InitLogFile(boost::filesystem::path &path)
	{
		if (exists(path))
		{
			if (is_regular_file(path))
			{
				return path;
			}
			if (is_directory(path))
			{
				return CreatePathToFile(path);
			}
		}

		return CreatePathToFile(DEFAULT_DIR);

	}


	void Write(std::stringstream& sstrm)
	{
		m_mutex.lock();

		if(m_pathToLog.empty())
		{
			if (WriteToStderr(sstrm))
			{
				//TODO: error Logger return 
				std::cerr << "Write to cderr success";
			}
		}
		else if(WriteToFile(sstrm))
		{
			//TODO: error Logger return 
			std::cerr << "Write to file success";
		}
		
		m_mutex.unlock();
	}
private:
	bool WriteToFile(std::stringstream& sstrm)
	{
		boost::filesystem::ofstream mOfstream;
		mOfstream.open(m_pathToLog);
		mOfstream << sstrm.rdbuf();
		mOfstream.close();
		return true;
	}

	boost::filesystem::path CreatePathToFile(boost::filesystem::path pathToDir)
	{
		boost::filesystem::path tmpPath;

		tmpPath.append(pathToDir);

		if (!exists(tmpPath))
		{
			create_directory(tmpPath);
		}

		std::stringstream fileName;
		fileName << "/" << PROGRAM_NAME << "_" << PROGRAM_VERSION << "_";
		boost::posix_time::ptime date_time = boost::posix_time::second_clock::local_time();
		fileName.imbue(std::locale(fileName.getloc(), new boost::posix_time::time_facet("%d.%m.%Y_%H.%M")));
		fileName << date_time << ".log";

		tmpPath.append(fileName.str());

		boost::filesystem::fstream fstream;
		fstream.open(tmpPath);
		fstream.close();

		return tmpPath;
	}


	bool WriteToStderr(std::stringstream& sstrm)
	{
		std::cerr << sstrm.rdbuf();
		return true;
	}

	std::mutex m_mutex;
	boost::filesystem::path m_pathToLog;

};

std::shared_ptr<Logger> GetWriter(boost::filesystem::path path)
{
	return std::make_shared<Logger>(path);
}

std::shared_ptr<Logger> GetWriter()
{
	return std::make_shared<Logger>();
}



class Collector 
{
public:
	Collector(std::shared_ptr<Logger> writer) : wr(writer)
	{

	}

	std::stringstream& get()
	{
		return  m_sstr;
	}

	~Collector()
	{
		wr->Write(m_sstr);
	}

private:
	std::shared_ptr<Logger> wr;
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
	auto wr_ptr = GetWriter("123");


	Collector abc(wr_ptr);

	TRACE(abc, LogLevel::INFO) << " adadasdasd";
	TRACE(abc, LogLevel::ERROR) << " adadasdasd";
	TRACE(abc, LogLevel::FATAL) << " adadasdasd";



	std::cout << "\n---------------------------------------------------" << std::endl;


	//time_facet* facet = new time_facet("%d/%b/%Y %H:%M:%S")
	std::cout << "Current time with second resolution: " << boost::posix_time::microsec_clock::local_time().time_of_day() << std::endl;





	return 0;

}
