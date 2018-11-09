#include "Window.h"
#include "tchar.h"
#include "icon.h"

Window::Window() : 
	handle(nullptr)
{
	// �\���̏�����
	window = {};
	rect = {};
	size = DirectX::XMINT2(640, 480);

	SetWindow();
	SetRect();
	Create();
}

Window::~Window()
{
	// ����������������ہA������Ăяo��
	UnregisterClass(window.lpszClassName, window.hInstance);
}

LRESULT Window::WindowProcedual(HWND handle, UINT uInt, WPARAM wParam, LPARAM lParam)
{
	// �E�B���h�E���j�����ꂽ�ۂɌĂяo��
	if (uInt == WM_DESTROY) {
		PostQuitMessage(0);	// OS�ɑ΂��āA�A�v���P�[�V�����I����`����
		return 0;
	}
	return DefWindowProc( handle, uInt, wParam, lParam);	// ��ꏈ�����s��
}

void Window::SetWindow(void)
{
	window.cbSize = sizeof(WNDCLASSEX);	// WNDCLASSEX���g���ہA�������T�C�Y�����߂邽�߁A�K���L�q����
	window.lpfnWndProc = (WNDPROC)WindowProcedual;	// �R�[���o�b�N�֐��̎w��
	window.lpszClassName = ("DirectXTest");
	window.hIcon = LoadIcon(nullptr,MAKEINTRESOURCE(ICON_ID));
	window.hIconSm = LoadIcon( GetModuleHandle(0), MAKEINTRESOURCE(ICON_ID));
	window.hInstance = GetModuleHandle(0);	// �n���h���̎擾
	RegisterClassEx(&window);	// �A�v���P�[�V�����N���X	�� WNDCLASSEX���g�p����ꍇ�́ARegisterClass��"Ex"��t����
}

void Window::SetRect(void)
{
	// �E�B���h�E�T�C�Y��ݒ�
	rect.top = 0;
	rect.right = 640;
	rect.bottom = 480;
	rect.left = 0;
	// �E�B���h�E�T�C�Y�̕␳
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
}

void Window::Create(void)
{
	handle = CreateWindow(
		window.lpszClassName,	// �g�p����N���X���w��
		("DirectX12"),	// �^�C�g���o�[�̖��O�w��
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

	// �E�B���h�E�\��
	ShowWindow(handle, SW_SHOW);
}
