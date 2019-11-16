
#include <windows.h>
#include <vector>
#include <functional>

#define MAX_LOADSTRING 100

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void RefreshView(HWND hWnd, HDC hdc);
// 全局变量:
static HINSTANCE hInst;                                // 当前实例
static WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
static WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

static std::vector<std::function<int(int,int)>> msg;

void AddMsgFunc(std::function<int(int, int)> f) {
	msg.push_back(f);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此处放置代码。

	// 初始化全局字符串
	//LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
	wsprintfW(szWindowClass, L"_12345");
	wsprintfW(szWindowClass, L"title");
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	//HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

	MSG msg;

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;// LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
	wcex.hCursor = NULL;// LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = NULL;// (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL;// LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中
	int cxScreen, cyScreen;
	cxScreen = GetSystemMetrics(SM_CXSCREEN);
	cyScreen = GetSystemMetrics(SM_CYSCREEN);
	int w = 720/10*5;
	int h = 1080/10*5;
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		cxScreen/2-w/2, cyScreen/2 - h/2, w, h, nullptr, nullptr, hInstance, nullptr);
	//SetTimer(hWnd, 1, 60, NULL);
	if (!hWnd)
	{
		return FALSE;
	}
	//CreateEGLContext(hWnd, &eglDisplay, &eglContext, &eglSurface);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


static HDC hdcBuffer=0;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//HPEN newPen;       // 用于创建新画笔   
	//HPEN *pOldPen;     // 用于存放旧画笔   
	//HBRUSH newBrush;   // 用于创建新画刷   
	//HBRUSH *pOldBrush; // 用于存放旧画刷   
	RECT rect;
	HDC hdc;
	HBITMAP hBitMap;
	HGDIOBJ hOldBitMap;
	int W;
	int H;
	GetClientRect(hWnd, &rect);

	switch (message)
	{
	case WM_CREATE:
		W = LOWORD(lParam); // width of client area
		H = HIWORD(lParam); // height of client area
		hdc = GetDC(hWnd); //获取设备

		hdcBuffer = CreateCompatibleDC(hdc); //给设备分配一个内存空间

		 hBitMap = CreateCompatibleBitmap(hdc, W, H); //创建一个cxClient, cyClient大小并且适应DC设备环境的位图

		ReleaseDC(hWnd, hdc);

		SelectObject(hdcBuffer, hBitMap); //将位图设置为hdcBuffer的画刷

		RefreshView(hWnd, hdcBuffer);
		//————————————————
		//	版权声明：本文为CSDN博主「Swell_Fish」的原创文章，遵循 CC 4.0 BY - SA 版权协议，转载请附上原文出处链接及本声明。
		//	原文链接：https ://blog.csdn.net/polley88/article/details/49719299
		return  DefWindowProc(hWnd, message, wParam, lParam);

	case WM_ERASEBKGND:
			return 1;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
			/*case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;*/
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_KEYDOWN:
		for (int i = 0; i < msg.size();i++) {
			msg[i](wParam, lParam);
		}
		break;
	case WM_SIZE:
		W = LOWORD(lParam); // width of client area
		H = HIWORD(lParam); // height of client area

		hdc = GetDC(hWnd); //获取设备
		hBitMap = CreateCompatibleBitmap(hdc, W, H); //创建一个cxClient, cyClient大小并且适应DC设备环境的位图
		ReleaseDC(hWnd, hdc);
		//ReleaseDC(hWnd, hdc);

		hOldBitMap = SelectObject(hdcBuffer, hBitMap); //将位图设置为hdcBuffer的画刷
		DeleteObject(hOldBitMap);
		/*if (eglSurface) {
			glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		}*/
		InvalidateRect(hWnd, NULL, false);
		//PatBlt(hdcBuffer, 0, 0, rect.right, rect.bottom, WHITENESS);
		//RefreshView(hWnd, hdcBuffer);
		//Invalidate(true);
		//PostMessage(hWnd, WM_PAINT, 0, 0);
		//UpdateWindow(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_TIMER:
		if (hdcBuffer!=0) {
			//PatBlt(hdcBuffer, 0, 0, rect.right, rect.bottom, WHITENESS);
			//RefreshView(hWnd, hdcBuffer);
		}
		InvalidateRect(hWnd, NULL, false);
		//PostMessage(hWnd, WM_PAINT, 0, 0);
		//return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		//PatBlt(hdcBuffer, 0, 0, rect.right, rect.bottom, WHITENESS);
		//Rectangle(hdc, 0, 0, rect.right, rect.bottom);
		RefreshView(hWnd, hdcBuffer);
		BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcBuffer, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);


	}
	break;
	

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

