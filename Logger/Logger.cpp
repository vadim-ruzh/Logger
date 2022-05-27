#include "logger.h"
#include "error_processing.h"
#include "executable_name.h"
#include "registry.h"
#include "ver_info.h"

#include <string_view>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <thread>
#include <memory>
#include <string>
#include <mutex>
#include <boost/date_time.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <boost/scope_exit.hpp>

namespace
{
	static constexpr std::wstring_view registryNameOfDebugModeKey = L"DebugMode";
	static constexpr std::string_view registryPathToCurrentUserSoftware = "HKEY_CURRENT_USER\\SOFTWARE\\";
	static constexpr std::string_view pathToProgramData = "C:/ProgramData";
	static constexpr std::string_view logFolderName = "Log";
	static constexpr DWORD registryValueWhenDebugIsEnabled = 1000;

	std::string GetNameOfLogFile(std::string_view programName)
	{
		std::stringstream fileName;

		fileName << programName << "_";
		fileName << PROGRAM_VERSION << "_";

		const boost::posix_time::ptime date_time = boost::posix_time::second_clock::local_time();
		fileName.imbue(std::locale(fileName.getloc(), new boost::posix_time::time_facet("%d.%m.%Y_%H.%M")));
		fileName << date_time << ".log";

		return fileName.str();
	}

	std::filesystem::path GetPathToLog(std::string_view programName)
	{
		const std::string programNameInLogFile = programName.empty()
			                                         ? GetExecutableName()
			                                         : programName.data();

		std::filesystem::path pathToLogFile{ pathToProgramData };
		pathToLogFile /= programNameInLogFile;
		pathToLogFile /= logFolderName;
		if (!exists(pathToLogFile))
		{
			create_directories(pathToLogFile);
		}

		pathToLogFile /= GetNameOfLogFile(programNameInLogFile);
		if (!exists(pathToLogFile))
		{
			std::ofstream logFileStream(pathToLogFile.c_str());
			if (!logFileStream)
				throw std::runtime_error("Can't create log file");

			logFileStream.close();
		}

		return pathToLogFile;
	}

	bool IsDebugModeEnabledInRegistry(std::string_view programName)
	{
		const std::string programNameInLogFile = programName.empty()
								? GetExecutableName()
								: programName.data();

		const auto pathToProgramRegistryKey = boost::locale::conv::utf_to_utf<wchar_t>(
			registryPathToCurrentUserSoftware.data() + programNameInLogFile);

		const registry::RegistryEditor regEditor(pathToProgramRegistryKey);

		DWORD debugModeValue;
		const auto result = regEditor.GetDword(registryNameOfDebugModeKey, debugModeValue);
		if (result != registry::ResultCode::sOk)
		{
			debugModeValue = 1050;
			ERROR_RETURN_EX(regEditor.SetDword(registryNameOfDebugModeKey, debugModeValue), false)
				<< "Can't set " << registryNameOfDebugModeKey.data() << " register key value to " << debugModeValue;
		}

		return debugModeValue < registryValueWhenDebugIsEnabled;
	}
}

Logger::Logger(std::string_view programName)
	: mPathToLog(GetPathToLog(programName))
	, mIsDebugModeEnabled(IsDebugModeEnabledInRegistry(programName))
{}

Logger::Logger()
	: Logger("")
{}

Logger::~Logger() = default;

void Logger::Write(std::string_view message)
{
	std::lock_guard lock{ mMutex };

	std::ofstream fileStream;
	fileStream.rdbuf()->open(mPathToLog.string(), std::ios_base::app, _SH_DENYWR);

	BOOST_SCOPE_EXIT((&fileStream))
	{
		fileStream.close();
	} BOOST_SCOPE_EXIT_END

	fileStream << message;
}


bool Logger::IsDebugModeEnabled() const
{
	return mIsDebugModeEnabled;
}

Collector::Collector(const std::shared_ptr<Logger>& logger)
	: mLogger(logger)
{
}

Collector::~Collector()
{
	mLogger->Write(mMessageStream.str());
}

bool Collector::IsDebugModeEnabled() const
{
	return mLogger->IsDebugModeEnabled();
}
