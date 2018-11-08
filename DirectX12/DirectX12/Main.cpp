#include "Main.h"
#include "Window.h"
#include "D3D12Manager.h"

void ClassCreate()
{
	window = std::make_shared<Window>();
	manager = std::make_shared<D3D12Manager>(window);
}

int main() {
	// windowを呼び出す
	ClassCreate();

	// msgを初期化
	MSG msg = {};

	//D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;

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
