#pragma once
#include "Windows.h"
#include <DirectXMath.h>

class Window
{
public:
	Window();
	~Window();

	// �n���h���擾
	HWND GetHandle() {
		return handle;
	}

	DirectX::XMINT2 GetSize(void) const {
		return size;
	}

private:
	// �v���V�[�W������static�ł��Ƃ悢�@�K���K�v
	// �R�[���o�b�N�֐�
	static LRESULT WindowProcedual(HWND handle, UINT unit, WPARAM wPanam, LPARAM lPanam);

	void SetWindow(void);
	void SetRect(void);
	void Create(void);

	WNDCLASSEX window;	// window�̎�ނ̍\����
	RECT rect;	// window�̃T�C�Y�̍\����
	HWND handle;	// window�̃n���h��

	DirectX::XMINT2 size;
};

