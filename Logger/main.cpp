#include <iostream>
#include "Logger.cpp"
#include "ExecutableName.h"
#include "Trace.h"


int main()
{
	const auto wr_ptr = std::make_shared<Logger>();

	Collector abc(wr_ptr);



	TRACE(abc,LogLevel::DEBUG) << GetExecutableName();

	abc << "asdad" << 123;

	std::cout << "\n---------------------------------------------------" << std::endl;
	std::cout << std::hex << std::this_thread::get_id() << " 123" << std::endl;
	std::cout << GetExecutableName() << std::endl;


	std::cout << "Current time with second resolution: " << std::endl;

	return 0;
}
