#include "Window.h"
#include "tchar.h"


Window::Window() : 
	handle(nullptr)
{
	// 構造体初期化
	window = {};
	rect = {};
	size = DirectX::XMINT2(640, 480);

	SetWindow();
	SetRect();
	Create();
}

Window::~Window()
{
	// メモリから解放する際、これを呼び出す
	UnregisterClass(window.lpszClassName, window.hInstance);
}

LRESULT Window::WindowProcedual(HWND handle, UINT uInt, WPARAM wParam, LPARAM lParam)
{
	// ウィンドウが破棄された際に呼び出す
	if (uInt == WM_DESTROY) {
		PostQuitMessage(0);	// OSに対して、アプリケーション終了を伝える
		return 0;
	}
	return DefWindowProc( handle, uInt, wParam, lParam);	// 基底処理を行う
}

void Window::SetWindow(void)
{
	window.cbSize = sizeof(WNDCLASSEX);	// WNDCLASSEXを使う際、メモリサイズを決めるため、必ず記述する
	window.lpfnWndProc = (WNDPROC)WindowProcedual;	// コールバック関数の指定
	window.lpszClassName = ("DirectXTest");
	window.hInstance = GetModuleHandle(0);	// ハンドルの取得
	RegisterClassEx(&window);	// アプリケーションクラス	※ WNDCLASSEXを使用する場合は、RegisterClassに"Ex"を付ける
}

void Window::SetRect(void)
{
	// ウィンドウサイズを設定
	rect.top = 0;
	rect.right = 640;
	rect.bottom = 480;
	rect.left = 0;
	// ウィンドウサイズの補正
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
}

void Window::Create(void)
{
	handle = CreateWindow(
		window.lpszClassName,	// 使用するクラス名指定
		("DirectX12"),	// タイトルバーの名前指定
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr,
		nullptr,
		window.hInstance,
		nullptr
	);

	// ウィンドウ表示
	ShowWindow(handle, SW_SHOW);
}
