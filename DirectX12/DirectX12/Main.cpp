#include "Main.h"
#include "Window.h"

void ClassCreate()
{
	window = std::make_shared<Window>();
}

int main() {
	// window���Ăяo��
	ClassCreate();

	// msg��������
	MSG msg = {};

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
