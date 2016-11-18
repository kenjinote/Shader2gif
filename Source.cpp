#ifndef UNICODE
#define UNICODE
#endif

#pragma comment(lib, "gdiplus")
#pragma comment(lib, "glew32s")
#pragma comment(lib, "shlwapi")

#define GLEW_STATIC

#include <vector>
#include <string>
#include <windows.h>
#include <shlwapi.h>
#include <gdiplus.h>
#include <richedit.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "GifEncoder.h"
#include "resource.h"

#define PREVIEW_WIDTH 512
#define PREVIEW_HEIGHT 384
#define WM_CREATED WM_APP

HDC hDC;
BOOL active;
GLuint program;
GLuint vao;
GLuint vbo;
const TCHAR szClassName[] = TEXT("Window");
const GLfloat position[][2] = { { -1.f, -1.f }, { 1.f, -1.f }, { 1.f, 1.f }, { -1.f, 1.f } };
const int vertices = sizeof position / sizeof position[0];
const GLchar vsrc[] = "in vec4 position;void main(void){gl_Position = position;}";
GLuint texture1;

inline GLint GetShaderInfoLog(GLuint shader)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == 0) OutputDebugString(TEXT("Compile Error\n"));
	GLsizei bufSize;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &bufSize);
	if (bufSize > 1)
	{
		LPSTR infoLog = (LPSTR)GlobalAlloc(0, bufSize);
		GLsizei length;
		glGetShaderInfoLog(shader, bufSize, &length, infoLog);
		OutputDebugStringA(infoLog);
		GlobalFree(infoLog);
	}
	return status;
}

inline GLint GetProgramInfoLog(GLuint program)
{
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == 0) OutputDebugString(TEXT("Link Error\n"));
	GLsizei bufSize;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufSize);
	if (bufSize > 1)
	{
		LPSTR infoLog = (LPSTR)GlobalAlloc(0, bufSize);
		GLsizei length;
		glGetProgramInfoLog(program, bufSize, &length, infoLog);
		OutputDebugStringA(infoLog);
		GlobalFree(infoLog);
	}
	return status;
}

inline GLuint CreateProgram(LPCSTR vsrc, LPCSTR fsrc)
{
	const GLuint vobj = glCreateShader(GL_VERTEX_SHADER);
	if (!vobj) return 0;
	glShaderSource(vobj, 1, &vsrc, 0);
	glCompileShader(vobj);
	if (GetShaderInfoLog(vobj) == 0)
	{
		glDeleteShader(vobj);
		return 0;
	}
	const GLuint fobj = glCreateShader(GL_FRAGMENT_SHADER);
	if (!fobj)
	{
		glDeleteShader(vobj);
		return 0;
	}
	glShaderSource(fobj, 1, &fsrc, 0);
	glCompileShader(fobj);
	if (GetShaderInfoLog(fobj) == 0)
	{
		glDeleteShader(vobj);
		glDeleteShader(fobj);
		return 0;
	}
	GLuint program = glCreateProgram();
	if (program)
	{
		glAttachShader(program, vobj);
		glAttachShader(program, fobj);
		glLinkProgram(program);
		if (GetProgramInfoLog(program) == 0)
		{
			glDetachShader(program, fobj);
			glDetachShader(program, vobj);
			glDeleteProgram(program);
			program = 0;
		}
	}
	glDeleteShader(vobj);
	glDeleteShader(fobj);
	return program;
}

VOID SetTexture(HBITMAP hBitmap)
{
	BITMAP inf;
	GetObject(hBitmap, sizeof(BITMAP), &inf);
	if (inf.bmBitsPixel != 24){ return; }
	if (texture1)
	{
		glDeleteTextures(1, &texture1);
	}
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_1D, texture1);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage1D(GL_TEXTURE_1D, 0, 3, inf.bmWidth, inf.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, inf.bmBits);
	glEnable(GL_TEXTURE_1D);
}

inline BOOL InitGL(GLvoid)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* 2 *
		vertices, position, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	HBITMAP hBitmap = (HBITMAP)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	SetTexture(hBitmap);
	DeleteObject(hBitmap);
	return TRUE;
}

inline VOID DrawGLScene()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
	glUniform1f(glGetUniformLocation(program, "time"), GetTickCount() / 1000.0f);
	glBindVertexArray(vao);
	glDrawArrays(GL_QUADS, 0, vertices);
	glBindVertexArray(0);
	glUseProgram(0);
	glFlush();
	SwapBuffers(hDC);
}

inline VOID DrawGLScene(HDC hdc, GLfloat time)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
	glUniform1f(glGetUniformLocation(program, "time"), time);
	glBindVertexArray(vao);
	glDrawArrays(GL_QUADS, 0, vertices);
	glBindVertexArray(0);
	glUseProgram(0);
	glFlush();
	SwapBuffers(hDC);
	BITMAPINFO bitmapInfo;
	::memset(&bitmapInfo, 0, sizeof(BITMAPINFO));
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biWidth = PREVIEW_WIDTH;
	bitmapInfo.bmiHeader.biHeight = PREVIEW_HEIGHT;
	bitmapInfo.bmiHeader.biSizeImage = PREVIEW_WIDTH * PREVIEW_HEIGHT * 4;
	void *bmBits = 0;
	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP memBM = CreateDIBSection(0, &bitmapInfo, DIB_RGB_COLORS, &bmBits, 0, 0);
	glReadPixels(0, 0, PREVIEW_WIDTH, PREVIEW_HEIGHT, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bmBits);
	HGDIOBJ prevBitmap = SelectObject(memDC, memBM);
	BitBlt(hdc, 0, 0, PREVIEW_WIDTH, PREVIEW_HEIGHT, memDC, 0, 0, SRCCOPY);
	SelectObject(memDC, prevBitmap);
	DeleteObject(memBM);
	DeleteDC(memDC);
}

inline void CreateAnimationGif(LPCTSTR lpszFilePath, int nTime, int nFrameRate)
{
	CGifEncoder gifEncoder;
	gifEncoder.SetFrameSize(PREVIEW_WIDTH, PREVIEW_HEIGHT);
	gifEncoder.SetFrameRate(nFrameRate);
	gifEncoder.StartEncoder(std::wstring(lpszFilePath));
	Gdiplus::Bitmap *bmp = new Gdiplus::Bitmap(PREVIEW_WIDTH, PREVIEW_HEIGHT);
	for (int time = 0; time < nTime; time++)
	{
		Gdiplus::Graphics g(bmp);
		const HDC hdc = g.GetHDC();
		DrawGLScene(hdc, time / 10.f);
		g.ReleaseHDC(hdc);
		gifEncoder.AddFrame(bmp);
	}
	delete bmp;
	gifEncoder.FinishEncoder();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
		32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0 };
	static GLuint PixelFormat;
	static HWND hStatic;
	static HWND hEdit;
	static HWND hButton;
	static HFONT hFont;
	static HINSTANCE hRtLib;
	static BOOL bEditVisible = TRUE;
	static HGLRC hRC;
	switch (msg)
	{
	case WM_CREATE:
		hRtLib = LoadLibrary(TEXT("RICHED32"));
		hFont = CreateFont(24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Consolas"));
		hStatic = CreateWindow(TEXT("STATIC"), 0, WS_VISIBLE | WS_CHILD | SS_SIMPLE,
			10, 10, PREVIEW_WIDTH, PREVIEW_HEIGHT, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, 0, WS_VISIBLE | WS_CHILD | WS_HSCROLL |
			WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_NOHIDESEL,
			0, 0, 0, 0, hWnd, (HMENU)ID_EDIT, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("GIF出力..."), WS_VISIBLE | WS_CHILD,
			PREVIEW_WIDTH / 2 - 54, PREVIEW_HEIGHT + 20, 128, 32, hWnd, (HMENU)ID_EXPORT, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hEdit, EM_SETTEXTMODE, TM_PLAINTEXT, 0);
		SendMessage(hEdit, EM_LIMITTEXT, -1, 0);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, 0);
		if (!(hDC = GetDC(hStatic)) ||
			!(PixelFormat = ChoosePixelFormat(hDC, &pfd)) ||
			!SetPixelFormat(hDC, PixelFormat, &pfd) ||
			!(hRC = wglCreateContext(hDC)) ||
			!wglMakeCurrent(hDC, hRC) ||
			glewInit() != GLEW_OK ||
			!InitGL()) return -1;
		SetWindowText(hEdit,
			TEXT("#define pi 3.14159265358979\r\n")
			TEXT("uniform float time;\r\n")
			TEXT("void main()\r\n")
			TEXT("{\r\n")
			TEXT("	vec2 p = gl_FragCoord;\r\n")
			TEXT("	float c = 0.0;\r\n")
			TEXT("	for (float i = 0.0; i < 5.0; i++)\r\n")
			TEXT("	{\r\n")
			TEXT("		vec2 b = vec2(\r\n")
			TEXT("		sin(time + i * pi / 7) * 128 + 256,\r\n")
			TEXT("		cos(time + i * pi / 2) * 128 + 192\r\n")
			TEXT("		);\r\n")
			TEXT("		c += 16 / distance(p, b);\r\n")
			TEXT("	}\r\n")
			TEXT("	gl_FragColor = vec4(c*c / sin(time), c*c / 2, c*c, 1.0);\r\n")
			TEXT("}\r\n")
			);
		PostMessage(hWnd, WM_CREATED, 0, 0);
		break;
	case WM_CREATED:
		SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(ID_EDIT, EN_CHANGE), (long)hEdit);
		SendMessage(hEdit, EM_SETEVENTMASK, 0, (LPARAM)(SendMessage(hEdit, EM_GETEVENTMASK, 0, 0) | ENM_CHANGE));
		SetFocus(hEdit);
		DragAcceptFiles(hWnd, TRUE);
		break;
	case WM_SIZE:
		MoveWindow(hEdit, PREVIEW_WIDTH + 20, 10, LOWORD(lParam) - PREVIEW_WIDTH - 30, HIWORD(lParam) - 20, 1);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_EDIT:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				const DWORD dwSize = GetWindowTextLengthA(hEdit);
				if (!dwSize) return 0;
				LPSTR lpszText = (LPSTR)GlobalAlloc(0, (dwSize + 1)*sizeof(CHAR));
				if (!lpszText) return 0;
				GetWindowTextA(hEdit, lpszText, dwSize + 1);
				const GLuint newProgram = CreateProgram(vsrc, lpszText);
				if (newProgram)
				{
					if (program) glDeleteProgram(program);
					program = newProgram;
					SetWindowText(hWnd, TEXT("フラグメントシェーダ [コンパイル成功]"));
				}
				else
				{
					SetWindowText(hWnd, TEXT("フラグメントシェーダ [コンパイル失敗]"));
				}
				GlobalFree(lpszText);
			}
			break;
		case ID_SELECTALL:
			if (IsWindowVisible(hEdit))
			{
				SendMessage(hEdit, EM_SETSEL, 0, -1);
				SetFocus(hEdit);
			}
			break;
		case ID_EXPORT:
			{
				TCHAR szFileName[MAX_PATH] = {0};
				OPENFILENAME ofn;
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = TEXT("GIF(*.gif)\0*.gif\0すべてのファイル(*.*)\0*.*\0\0");
				ofn.lpstrFile = szFileName;
				ofn.nMaxFile = sizeof(szFileName);
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
				if (GetSaveFileName(&ofn))
				{
					CreateAnimationGif(szFileName, 50, 100);
					MessageBox(hWnd, TEXT("完了しました。"), TEXT("確認"), MB_ICONINFORMATION);
				}
			}
			break;
		case ID_IMPORT_TEXTURE:
			{
				TCHAR szFileName[MAX_PATH] = { 0 };
				OPENFILENAME ofn;
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = TEXT("Bitmap(*.bmp)\0*.bmp\0すべてのファイル(*.*)\0*.*\0\0");
				ofn.lpstrFile = szFileName;
				ofn.nMaxFile = sizeof(szFileName);
				ofn.Flags = OFN_FILEMUSTEXIST;
				if (GetOpenFileName(&ofn))
				{
					const HBITMAP hBitmap = (HBITMAP)LoadImage(GetModuleHandle(0), szFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
					SetTexture(hBitmap);
					DeleteObject(hBitmap);
				}
			}
			break;
		}
		break;
	case WM_DROPFILES:
		{
			const HDROP hDrop = (HDROP)wParam;
			TCHAR szFileName[MAX_PATH];
			DragQueryFile(hDrop, 0, szFileName, sizeof(szFileName));
			LPCTSTR lpExt = PathFindExtension(szFileName);
			if (PathMatchSpec(lpExt, TEXT("*.bmp")))
			{
				const HBITMAP hBitmap = (HBITMAP)LoadImage(GetModuleHandle(0), szFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
				SetTexture(hBitmap);
				DeleteObject(hBitmap);
			}
			DragFinish(hDrop);
		}
		break;
	case WM_ACTIVATE:
		active = !HIWORD(wParam);
		break;
	case WM_DESTROY:
		DeleteObject(hFont);
		if (texture1) glDeleteTextures(1, &texture1);
		if (program) glDeleteProgram(program);
		if (vbo) glDeleteBuffers(1, &vbo);
		if (vao) glDeleteVertexArrays(1, &vao);
		if (hRC)
		{
			wglMakeCurrent(0, 0);
			wglDeleteContext(hRC);
		}
		if (hDC) ReleaseDC(hStatic, hDC);
		DestroyWindow(hEdit);
		FreeLibrary(hRtLib);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	ULONG_PTR gdiToken;
	Gdiplus::GdiplusStartupInput gdiSI;
	Gdiplus::GdiplusStartup(&gdiToken, &gdiSI, NULL);
	MSG msg;
	const WNDCLASS wndclass = { 0, WndProc, 0, 0, hInstance, 0, LoadCursor(0, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), MAKEINTRESOURCE(IDR_MENU1), szClassName };
	RegisterClass(&wndclass);
	const HWND hWnd = CreateWindow(szClassName, 0, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 0, 0, hInstance, 0);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	ACCEL Accel[] = { { FVIRTKEY | FCONTROL, 'A', ID_SELECTALL } };
	const HACCEL hAccel = CreateAcceleratorTable(Accel, sizeof(Accel) / sizeof(ACCEL));
	BOOL done = 0;
	while (!done)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				done = TRUE;
			}
			else if (!TranslateAccelerator(hWnd, hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else if (active)
		{
			DrawGLScene();
		}
	}
	DestroyAcceleratorTable(hAccel);
	Gdiplus::GdiplusShutdown(gdiToken);
	return msg.wParam;
}
