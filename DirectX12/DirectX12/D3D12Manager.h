#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "DirectXMath.h"
#include <memory>

class Window;

struct Vertex {
	DirectX::XMFLOAT3 pos;
};

class D3D12Manager
{
public:
	D3D12Manager(std::weak_ptr<Window>win);
	~D3D12Manager();

private:
	// 初期化
	void Initialize(void);
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
	// デスクリプタヒープ
	HRESULT CreateDescriptorHeap(void);
	// レンダーターゲットビュー
	void CreateRenderTargetView(void);
	// ルートシグネチャ
	void CreateRootSignature(void);
	// レンダーターゲットビューのクリアコマンド発行
	// フェンス発行
	void ClearRTV();
	// 
	void CreatePolygon(void);


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
	UINT backNum;

	ID3D12DescriptorHeap* rtvHeap;
	ID3D12Resource* rtvResource[2];
};

