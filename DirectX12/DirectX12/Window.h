#pragma once
#include "Windows.h"
#include <DirectXMath.h>

class Window
{
public:
	Window();
	~Window();

	// ハンドル取得
	HWND GetHandle() {
		return handle;
	}

	DirectX::XMINT2 GetSize(void) const {
		return size;
	}

private:
	// プロシージャルはstaticでやるとよい　必ず必要
	// コールバック関数
	static LRESULT WindowProcedual(HWND handle, UINT unit, WPARAM wPanam, LPARAM lPanam);

	void SetWindow(void);
	void SetRect(void);
	void Create(void);

	WNDCLASSEX window;	// windowの種類の構造体
	RECT rect;	// windowのサイズの構造体
	HWND handle;	// windowのハンドル

	DirectX::XMINT2 size;
};

