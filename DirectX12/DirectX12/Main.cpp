#include "Main.h"
#include "Window.h"
#include "D3D12Manager.h"

void ClassCreate()
{
	window = std::make_shared<Window>();
	manager = std::make_shared<D3D12Manager>(window);
}

int main() {
	// window���Ăяo��
	ClassCreate();

	// msg��������
	MSG msg = {};

	//D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;

	// �������[�v
	while(msg.message != WM_QUIT) {
		// OS����̃��b�Z�[�W��msg�Ɋi�[
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);	
			DispatchMessage(&msg);
		}
	}
	
	return 0;
}
