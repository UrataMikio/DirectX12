#include "Main.h"
#include "Window.h"

void ClassCreate()
{
	window = std::make_shared<Window>();
}

int main() {
	// windowを呼び出す
	ClassCreate();

	// msgを初期化
	MSG msg = {};

	// 無限ループ
	while(msg.message != WM_QUIT) {
		// OSからのメッセージをmsgに格納
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);	
			DispatchMessage(&msg);
		}
	}
	
	return 0;
}
