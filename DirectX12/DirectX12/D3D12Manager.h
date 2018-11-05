#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <memory>

class Window;



class D3D12Manager
{
public:
	D3D12Manager(std::weak_ptr<Window>win);
	~D3D12Manager();

private:
	// �f�o�C�X����
	HRESULT CreateDev(void);
	// �R�}���h����
	// �L���[
	HRESULT CreateCmmandQueue(void);
	// �A���P�[�^
	HRESULT CreateCommandAllocator(void);
	// ���X�g
	HRESULT CreateCmmandList(void);
	// �X���b�v�`�F�C��
	HRESULT CreateSwapChainHWnd(void);


	// �E�B���h�E
	std::weak_ptr<Window>win;

	HRESULT result;

	D3D_FEATURE_LEVEL level;
	ID3D12Device * device;
	
	// �R�}���h����
	ID3D12CommandAllocator* allocator;
	ID3D12CommandQueue* queue;
	ID3D12GraphicsCommandList* list;
 
	IDXGIFactory4* factory;
	IDXGISwapChain3* swap;
};

