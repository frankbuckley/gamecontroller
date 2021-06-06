#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <strsafe.h>
#include <detour/include/detours.h>
#include <glfw/include/GLFW/glfw3.h>

static BOOL(* TrueGlSwapLayerBuffers)(HDC dc, UINT params);

void GlSwapLayerBuffers(
	HDC dc, UINT params)
{
	OutputDebugString(L"[Controller] GlSwapLayerBuffers in Controller.\n");

		TrueGlSwapLayerBuffers(dc, params);
}

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,
	DWORD fdwReason,
	LPVOID lpReserved)
{
	OutputDebugString(L"[Controller] DllMain called.\n");

	HMODULE openGlModule = {};

	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		OutputDebugString(L"[Controller] DLL_PROCESS_ATTACH in DllMain.\n");

		openGlModule = GetModuleHandle(L"opengl32.dll");

		if (!openGlModule)
		{
			OutputDebugString(L"[Controller] Failed GetModuleHandle.\n");
			return FALSE;
		}
		
		TrueGlSwapLayerBuffers = (BOOL(__stdcall*)(HDC, UINT))GetProcAddress(openGlModule, "wglSwapBuffers");

		if (!TrueGlSwapLayerBuffers)
		{
			OutputDebugString(L"[Controller] Failed GetProcAddress.\n");
			return FALSE;
		}

		DetourRestoreAfterWith();

		if (DetourTransactionBegin() != NO_ERROR)
		{
			OutputDebugString(L"[Controller] Error calling DetourTransactionBegin.\n");
			return FALSE;
		}

		if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR)
		{
			OutputDebugString(L"[Controller] Error calling DetourUpdateThread.\n");
			return FALSE;
		}

		// Attach detour for glfwSwapBuffers to DetouredSwapBuffers

		if (DetourAttach(&(PVOID&)TrueGlSwapLayerBuffers, GlSwapLayerBuffers) != NO_ERROR)
		{
			OutputDebugString(L"[Controller] Error calling DetourAttach.\n");
			return FALSE;
		}
		
		if (DetourTransactionCommit() != NO_ERROR)
		{
			OutputDebugString(L"[Controller] Error calling DetourTransactionCommit.\n");
			return FALSE;
		}

		OutputDebugString(L"[Controller] DetourAttach completed.\n");

		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:

		OutputDebugString(L"[Controller] DLL_PROCESS_DETACH in DllMain.\n");

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)TrueGlSwapLayerBuffers, GlSwapLayerBuffers);
		DetourTransactionCommit();

		break;
	}
	return TRUE;
}
