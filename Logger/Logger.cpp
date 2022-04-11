#include <fstream>
#include <sstream>
#include <thread>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include "ver_info.h"
#include <memory>
#include <string>
#include <mutex>
#include "ExecutableName.h"
#include "Registry.cpp"

class Logger
{
public:
	Logger(const std::string& programName = "") :
	dbgMode(CheckDbg(programName.empty() ? GetExecutableName() : programName)),
	m_pathToLog(InitPathToLog(programName.empty() ? GetExecutableName() : programName)){}

	void WriteToFile(const std::string& message)
	{
		m_mutex.lock();

		boost::filesystem::ofstream mOfstrm;
		mOfstrm.rdbuf()->open(m_pathToLog.string(), std::ios_base::app, _SH_DENYWR);
		mOfstrm << message;
		mOfstrm.close();

		m_mutex.unlock();
	}

	bool dbgMode;
private:
	boost::filesystem::path InitPathToLog(const std::string& programName)
	{
		boost::filesystem::path logPath("C:/ProgramData");
		logPath /= programName;
		logPath /= "Log";
		if(!exists(logPath))
		{
			create_directories(logPath);
		}

		logPath /= createLogFileName(programName);
		if(!exists(logPath))
		{
			boost::filesystem::fstream fstream(logPath);
			fstream.close();
		}

		return logPath;
	}

	std::string createLogFileName(const std::string& programName)
	{
		std::stringstream fileName;

		fileName << programName << "_";
#ifdef PROGRAM_VERSION
		fileName << PROGRAM_VERSION <<"_";
#endif
		const boost::posix_time::ptime date_time = boost::posix_time::second_clock::local_time();
		fileName.imbue(std::locale(fileName.getloc(), new boost::posix_time::time_facet("%d.%m.%Y_%H.%M")));
		fileName << date_time << ".log";

		return fileName.str();
	}

	bool CheckDbg(const std::string& programName)
	{
		LSTATUS lstatus;

		//registry path
		auto regPath = utf8toUtf16("SOFTWARE\\" + programName);
		//open/create key
		WinRegManager reg(Hkeys::HkeyCurrentUser, regPath, AccessRights::AccessAll);

		//read from a variable
		const auto res = reg.GetDword(L"DebugMode",lstatus);

		// 0 - ok , !0 - error
		if(lstatus)
		{	//create variable DebugMode = 999
			reg.SetDword(L"DebugMode", 999);
			return false;
		}
		if(res < 1000)
		{
			return false;
		}

		return true;
	}

	std::mutex m_mutex;
	boost::filesystem::path m_pathToLog;
};

class Collector
{
public:
	Collector(const std::shared_ptr<Logger> logger) : m_loggerPtr(logger)
	{
		dbgMode = m_loggerPtr->dbgMode;
	}

	template<typename T>
	Collector& operator<<(const T& message)
	{
		m_sstr << message;
		return *this;
	}


	~Collector()
	{
		m_loggerPtr->WriteToFile(m_sstr.str());
	}

	bool dbgMode;
private:

	std::shared_ptr<Logger> m_loggerPtr;
	std::stringstream m_sstr;
};