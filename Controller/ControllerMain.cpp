#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <detour/include/detours.h>
#include <glfw/include/GLFW/glfw3.h>

static void (GLFWAPI * TrueSwapBuffers)(GLFWwindow * window) = glfwSwapBuffers;

void DetouredSwapBuffers(
	GLFWwindow* window)
{
	OutputDebugString(L"DetouredSwapBuffers in Controller.\n");

	TrueSwapBuffers(window);
}

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,
	DWORD fdwReason,
	LPVOID lpReserved)
{
	OutputDebugString(L"DllMain called.\n");

	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		OutputDebugString(L"DLL_PROCESS_ATTACH in DllMain.\n");

		DetourRestoreAfterWith();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueSwapBuffers, DetouredSwapBuffers);
		DetourTransactionCommit();

		OutputDebugString(L"DetourAttach completed.\n");

		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:

		OutputDebugString(L"DLL_PROCESS_DETACH in DllMain.\n");

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)TrueSwapBuffers, DetouredSwapBuffers);
		DetourTransactionCommit();

		break;
	}
	return TRUE;
}

