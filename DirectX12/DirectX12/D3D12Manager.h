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
	// デバイス生成
	HRESULT CreateDev(void);
	// コマンド生成
	// キュー
	HRESULT CreateCmmandQueue(void);
	// アロケータ
	HRESULT CreateCommandAllocator(void);
	// リスト
	HRESULT CreateCmmandList(void);
	// スワップチェイン
	HRESULT CreateSwapChainHWnd(void);


	// ウィンドウ
	std::weak_ptr<Window>win;

	HRESULT result;

	D3D_FEATURE_LEVEL level;
	ID3D12Device * device;
	
	// コマンド周り
	ID3D12CommandAllocator* allocator;
	ID3D12CommandQueue* queue;
	ID3D12GraphicsCommandList* list;
 
	IDXGIFactory4* factory;
	IDXGISwapChain3* swap;
};

