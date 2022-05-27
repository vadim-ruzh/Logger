#include <iostream>

#include "executable_name.h"
#include "logger.h"
#include "trace.h"

int main()
{
	const auto wr_ptr = std::make_shared<Logger>();

	Collector abc(wr_ptr);

	TRACE(abc, LogLevel::Debug) << GetExecutableName();
	TRACE(abc, LogLevel::Error) << GetExecutableName();

	abc << "asdad" << 123;

	std::cout << "---------------------------------------------------" << std::endl;
	std::cout << std::hex << std::this_thread::get_id() << " 123" << std::endl;
	std::cout << GetExecutableName() << std::endl;


	return 0;
}
