#include <iostream>

#include "executable_name.h"
#include "logger.h"
#include "registry.h"
#include "trace.h"

BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID IpReserved)
{
    BOOL bAllWentWell = TRUE;
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:     // Инициализация процесса. 
        break;
    case DLL_THREAD_ATTACH:     // Инициализация потока.
        break;
    case DLL_THREAD_DETACH:     // Очистка структур потока.
        break;
    case DLL_PROCESS_DETACH:     // Очистка структур процесса.
        break;
    }
    if (bAllWentWell)     return TRUE;
    else            return FALSE;
}

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
