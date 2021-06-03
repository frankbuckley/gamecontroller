#include <iostream>
#include <windows.h>
#include <stdlib.h>

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,
	DWORD fdwReason,
	LPVOID lpReserved)
{
	OutputDebugString(L"DllMain called.\n");

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		OutputDebugString(L"DLL_PROCESS_ATTACH in DllMain.\n");

		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

__declspec(dllexport) int __cdecl TestFunc() {

	OutputDebugString(L"TestFunc() called.\n");
	return 42;
}

#ifdef __cplusplus
}
#endif
