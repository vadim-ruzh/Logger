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


enum WriterError
{
	sOk,
	eErrorFileWrite,
};


//Выводит информацию
class Writer
{

public:

	Writer(boost::filesystem::path path)
	{
		InitLogFile(path);
		UseFile();
	}


	Writer()
	{
		UseStderr();
	}


	bool InitLogFile(boost::filesystem::path path)
	{
		if (exists(path))
		{
			pathToLogFile = path;
		}
		else
		{
			pathToLogFile.append("./Log");

			if (!boost::filesystem::exists(pathToLogFile))
			{
				boost::filesystem::create_directory(pathToLogFile);
			}

			auto time = boost::posix_time::second_clock::local_time();

			std::stringstream fileName;

			fileName <<"/" << PROGRAM_NAME << "-" PROGRAM_VERSION << "_"
				<< to_simple_string(time.date()) << "_"
				<< time.time_of_day().hours() << "-" << time.time_of_day().minutes() << ".log";

			pathToLogFile.append(fileName.str());
			boost::filesystem::fstream fstream;
			fstream.open("./log/file.txt");
			fstream.close();
		}

	}

	void UseFile()
	{
		this->useFile = 1;
	}

	void UseStderr()
	{
		this->useFile = 0;
	}

	void Write(std::stringstream& sstrm)
	{
		m_mutex.lock();

		if (this->useFile)
		{
			if (WriteToFile(sstrm))
			{
				//TODO: error Writer return 
				std::cerr << "Error write to file";
			}
		}

		if (WriteToStderr(sstrm))
		{
			//TODO: error Writer return 
			std::cerr << "Error write to file";
		}

		m_mutex.unlock();
	}




private:
	bool WriteToFile(std::stringstream& sstrm)
	{
		boost::filesystem::ofstream mOfstream;
		mOfstream.open(pathToLogFile);
		mOfstream << sstrm.rdbuf();
		mOfstream.close();
		return false;
	}

	bool WriteToStderr(std::stringstream& sstrm)
	{
		std::cerr << sstrm.rdbuf();
		return false;
	}

	bool useFile = 0;
	//boost::filesystem::path  destination;
	std::mutex m_mutex;

	boost::filesystem::path pathToLogFile;

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
	auto wr_ptr = std::make_shared<Writer>();

	logger abc(wr_ptr);

	TRACE(abc, LogLevel::INFO) << " adadasdasd";
	//std::this_thread::sleep_for(std::chrono::seconds(1));
	TRACE(abc, LogLevel::ERROR) << " adadasdasd";
	//std::this_thread::sleep_for(std::chrono::seconds(1));
	TRACE(abc, LogLevel::FATAL) << " adadasdasd";



	//std::this_thread::sleep_for(std::chrono::seconds(1));

	std::thread thr(demofunc, wr_ptr);

	std::thread th2(demofunc, wr_ptr);

	thr.join();

	//std::this_thread::sleep_for(std::chrono::seconds(1));
	//std::this_thread::sleep_for(std::chrono::seconds(1));

	th2.join();




	std::cout << "\n---------------------------------------------------" << std::endl;


	//time_facet* facet = new time_facet("%d/%b/%Y %H:%M:%S")
	std::cout << "Current time with second resolution: " << boost::posix_time::microsec_clock::local_time().time_of_day() << std::endl;





	return 0;

}
