#include <windows.h>
#include <stdlib.h>
#include <libloaderapi.h>

#include <../include/glad/include/glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <glfw/include/GLFW/glfw3.h>

#include <linmath/linmath.h>

#include <stdlib.h>
#include <stdio.h>

// ----------------------------------------------------------------------------
// Global variables

static wchar_t szWindowClass[] = L"GameControllerLoader";

static wchar_t szTitle[] = L"Game Controller Loader";

static HINSTANCE hInst;

static HMODULE hmoduleController;

static HFONT defaultFont;

static const struct
{
	float x, y;
	float r, g, b;
} vertices[3] =
{
	{ -0.6f, -0.4f, 1.f, 0.f, 0.f },
	{  0.6f, -0.4f, 0.f, 1.f, 0.f },
	{   0.f,  0.6f, 0.f, 0.f, 1.f }
};

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// ----------------------------------------------------------------------------
// Forward declarations of function defined below
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// ----------------------------------------------------------------------------
// The main entry point of the application
// ----------------------------------------------------------------------------
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vcol_location;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(1);

	// NOTE: OpenGL error checks have been omitted for brevity

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	glCompileShader(vertex_shader);

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	mvp_location = glGetUniformLocation(program, "MVP");
	vpos_location = glGetAttribLocation(program, "vPos");
	vcol_location = glGetAttribLocation(program, "vCol");

	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
		sizeof(vertices[0]), (void*)0);
	glEnableVertexAttribArray(vcol_location);
	glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
		sizeof(vertices[0]), (void*)(sizeof(float) * 2));

	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		mat4x4_identity(m);
		mat4x4_rotate_Z(m, m, (float)glfwGetTime());
		mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);

		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}

int CALLBACK BackupWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	WNDCLASSEX wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)) 
	{
		MessageBox(NULL,
			L"Call to RegisterClassEx failed!",
			L"Game Controller Loader",
			NULL);

		return 1;
	}

	hInst = hInstance;

	defaultFont = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));

	HWND hwndMainWindow = CreateWindow(
		szWindowClass,
		szTitle,							// window title bar
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 600,							// initial size (width, length)
		NULL,								// no parent window
		NULL,								// no menu bar
		hInstance,
		NULL
	);

	if (!hwndMainWindow)
	{
		MessageBox(NULL,
			L"Call to CreateWindow failed",
			L"Game Controller Loader",
			NULL);

		return EXIT_FAILURE;
	}
	else 
	{
		OutputDebugString(L"Loaded main window.\n");
	}	
	
	SendMessage(hwndMainWindow, WM_SETFONT, WPARAM(defaultFont), TRUE);

	HWND hwndLoadControllerButton = CreateWindow(
		L"BUTTON",
		L"Load Controller",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		10,
		80,
		150,
		40,
		hwndMainWindow,
		NULL,
		(HINSTANCE)GetWindowLongPtr(hwndMainWindow, GWLP_HINSTANCE),
		NULL);

	if (!hwndLoadControllerButton)
	{

		OutputDebugString(L"Failed to create Load Controller button.\n");

		return EXIT_FAILURE;
	}

	SendMessage(hwndLoadControllerButton, WM_SETFONT, WPARAM(defaultFont), TRUE);

	ShowWindow(hwndMainWindow,
		nCmdShow);

	UpdateWindow(hwndMainWindow);

	// ---------------------------------------------------------------------------
	// Message loop
	// ---------------------------------------------------------------------------

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

typedef int(__stdcall* f_TestFunc)();

// ----------------------------------------------------------------------------
// Message handler
// ----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	wchar_t greeting[] = L"Game Controller Loader";
	f_TestFunc testFunc;
	int result;

	const LPCWSTR dllName = L"Controller";

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		SelectObject(hdc, defaultFont);

		TextOut(hdc,
			10, 10,
			greeting, wcslen(greeting));

		EndPaint(hWnd, &ps);

		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_COMMAND:

		hmoduleController = LoadLibrary(
			dllName
		);

		if (!hmoduleController)
		{
			MessageBox(NULL,
				L"Failed to load Controller library",
				L"Game Controller Loader",
				NULL);

			break;
		}
		else 
		{
			OutputDebugString(L"Loaded library.\n");
		}

		// resolve function address here
		testFunc = (f_TestFunc)GetProcAddress(hmoduleController, "TestFunc");

		if (!testFunc) {
			OutputDebugString(L"Could not get address for TestFunc.\n");
			break;
		}

		result = testFunc();

		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return EXIT_SUCCESS;
}
