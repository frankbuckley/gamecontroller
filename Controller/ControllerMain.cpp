#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <strsafe.h>
#include <detour/include/detours.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <glfw/include/GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

static BOOL(* TrueGlSwapLayerBuffers)(HDC dc, UINT params);
static bool fly = false;
static bool unload = false;
static GLFWwindow* window;
static ImGuiIO io;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void GlSwapLayerBuffers(
	HDC dc, UINT params)
{
	OutputDebugString(L"[Controller] GlSwapLayerBuffers in Controller.\n");

	if (window)
	{
		OutputDebugString(L"[Controller] Calling glfwPollEvents...\n");

		//glfwPollEvents();

		// Start the Dear ImGui frame

		OutputDebugString(L"[Controller] Calling ImGui_ImplOpenGL3_NewFrame...\n");

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		OutputDebugString(L"[Controller] Started ImGui frame.\n");

		ImGui::NewFrame();
		{
			ImGui::Begin("BlackTeal Client");

			ImGui::Checkbox("Fly", &fly);
			ImGui::SameLine();


			if (ImGui::Button("Unload Cheat"))
				unload = true;

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// Rendering
		ImGui::Render();

		OutputDebugString(L"[Controller] Called ImGui::Render.\n");

		int display_w, display_h;

		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	//TrueGlSwapLayerBuffers(dc, params);
}

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void InitImGui()
{
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return;

	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create window with graphics context
	window = glfwCreateWindow(800, 600, "Hello World :D", NULL, NULL);
	//if (window == NULL)
	//    return 1;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initialize OpenGL loader

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING)
	bool err = false;
	glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
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

		InitImGui();

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

