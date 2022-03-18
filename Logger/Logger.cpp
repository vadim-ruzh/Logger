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
class Writer
{

public:


	//TODO :: добавить очередь задач 

	Writer(boost::filesystem::path path) :m_pathToLog(InitLogFile(path))
	{}

	Writer()
	{}

	boost::filesystem::path InitLogFile(boost::filesystem::path &path)
	{
		boost::filesystem::path tmpPath;

		if (exists(path))
		{
			tmpPath = path;
			return tmpPath;
		}
		
		tmpPath.append("./Log");
		if (!exists(tmpPath))
		{
			create_directory(tmpPath);
		}

		auto time = boost::posix_time::second_clock::local_time();

		std::stringstream fileName;

		fileName <<"/" PROGRAM_NAME << "-" PROGRAM_VERSION << "_"
			<< to_simple_string(time.date()) << "_"
			<< time.time_of_day().hours() << "-" << time.time_of_day().minutes() << ".log";

		tmpPath.append(fileName.str());
		boost::filesystem::fstream fstream;
		fstream.open(tmpPath);
		fstream.close();

		return tmpPath;

	}


	void Write(std::stringstream& sstrm)
	{
		m_mutex.lock();

		if(m_pathToLog.empty())
		{
			if (WriteToStderr(sstrm))
			{
				//TODO: error Writer return 
				std::cerr << "Write to cderr succes";
			}
		}
		else if(WriteToFile(sstrm))
		{
			//TODO: error Writer return 
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

	bool WriteToStderr(std::stringstream& sstrm)
	{
		std::cerr << sstrm.rdbuf();
		return false;
	}

	std::mutex m_mutex;
	boost::filesystem::path m_pathToLog;
};


class logger 
{
public:
	logger(std::shared_ptr<Writer> writer) : wr(writer)
	{

	}

	std::stringstream& get()
	{
		return  m_sstr;
	}

	~logger()
	{
		wr->Write(m_sstr);
	}

private:
	std::shared_ptr<Writer> wr;
	std::stringstream m_sstr;
};


#define TRACE(tracer,level)\
	tracer.get()\
		<< "\n" << boost::posix_time::microsec_clock::local_time().time_of_day()\
		<< "\t" << std::this_thread::get_id()\
		<< "\t"<< LevelToString(level) \
		<< "\t" <<__func__ << "\t"


 void demofunc(std::shared_ptr<Writer> wr_ptr)
{
	logger abc(wr_ptr);
	TRACE(abc, LogLevel::DEBUG) << " adadasdasd";
	TRACE(abc, LogLevel::FATAL) << " adadasdasd";
	TRACE(abc, LogLevel::INFO) << " adadasdasd";
}



int main()

{
	auto wr_ptr = std::make_shared<Writer>(".123");


	logger abc(wr_ptr);

	TRACE(abc, LogLevel::INFO) << " adadasdasd";
	TRACE(abc, LogLevel::ERROR) << " adadasdasd";
	TRACE(abc, LogLevel::FATAL) << " adadasdasd";



	std::cout << "\n---------------------------------------------------" << std::endl;


	//time_facet* facet = new time_facet("%d/%b/%Y %H:%M:%S")
	std::cout << "Current time with second resolution: " << boost::posix_time::microsec_clock::local_time().time_of_day() << std::endl;





	return 0;

}
