#include "D3D12Manager.h"
#include "Window.h"

#pragma comment(lib,"d3d12.lib") 
#pragma comment(lib,"dxgi.lib")

D3D_FEATURE_LEVEL levels[] = {
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};

D3D12Manager::D3D12Manager(std::weak_ptr<Window>win) :
	win(win), result(S_OK), backNum(0)
{
	device = nullptr;

	// コマンド周りの初期化
	allocator = nullptr;
	queue = nullptr;
	list = nullptr;

	Initialize();
}

D3D12Manager::~D3D12Manager()
{
	swap->Release();
	factory->Release();
	list->Release();
	allocator->Release();
	queue->Release();
	// オブジェクト解放の際は、deleteではなくRelease()を使おう
	device->Release();
}

void D3D12Manager::Initialize(void)
{
	// デバイス呼び出し
	CreateDev();
	// コマンドキュー呼び出し
	CreateCmmandQueue();
	// アロケーター呼び出し
	CreateCommandAllocator();
	// コマンドリスト呼び出し
	CreateCmmandList();
	// スワップチェイン呼び出し
	CreateSwapChainHWnd();
	// ディスクリプタヒープ呼び出し
	CreateDescriptorHeap();
	// レンダーターゲットビュー呼び出し
	CreateRenderTargetView();
	// ルートシグネチャ呼び出し
	CreateRootSignature();
	// レンダーターゲットビューのクリアコマンド呼び出し
	ClearRTV();
}

HRESULT D3D12Manager::CreateDev(void)
{
	for (auto& l : levels) {
		result = D3D12CreateDevice(nullptr, l, IID_PPV_ARGS(&device));
		if (result == S_OK) {
			level = l;
			break;
		}
	}
	
	return result;
}

HRESULT D3D12Manager::CreateCmmandQueue(void)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.NodeMask = 0;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	result = device->CreateCommandQueue( &desc, IID_PPV_ARGS(&queue));

	return result;
}

HRESULT D3D12Manager::CreateCommandAllocator(void)
{
	result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));

	return result;
}


HRESULT D3D12Manager::CreateCmmandList(void)
{
	result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&list));

	// 第4引数が nullptr で良いのは、「作ってないですよっ！！」と言われなっくてすむように

	list->Close();

	return result;
}

HRESULT D3D12Manager::CreateSwapChainHWnd(void)
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.Width = win.lock()->GetSize().x;
	desc.Height = win.lock()->GetSize().y;
	desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = 0;
	desc.SampleDesc = { 1,0 };
	desc.BufferUsage = 0;
	desc.BufferCount = 2;
	desc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	result = CreateDXGIFactory1(IID_PPV_ARGS(&factory));

	result = factory->CreateSwapChainForHwnd(queue, win.lock()->GetHandle(), &desc, nullptr, nullptr, (IDXGISwapChain1**)(&swap));

	backNum = desc.BufferCount;

	return result;
}

HRESULT D3D12Manager::CreateDescriptorHeap(void)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;
	desc.NumDescriptors = backNum;	// バッファの数分
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	result = device->CreateDescriptorHeap(&desc,IID_PPV_ARGS(&rtvHeap));

	return result;
}

void D3D12Manager::CreateRenderTargetView(void)
{
	UINT size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DMS;
	auto handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	
	for (UINT i = 0; i < backNum; ++i) {
		result = swap->GetBuffer(i,IID_PPV_ARGS(&rtvResource[i]));
		device->CreateRenderTargetView(rtvResource[i],&desc,handle);
		handle.ptr += size;
	}
}

void D3D12Manager::CreateRootSignature(void)
{
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	ID3D12RootSignature* rootSignature = nullptr;
	ID3DBlob* signature = nullptr;
	ID3DBlob* error = nullptr;

	desc.Flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12SerializeRootSignature( &desc, D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1 , &signature, &error);
	result = device->CreateRootSignature(0, signature->GetBufferPointer(),signature->GetBufferSize(),IID_PPV_ARGS(&rootSignature));
}

void D3D12Manager::ClearRTV()
{
	const FLOAT color[] = { 0.0f,1.0f,0.0f,1.0f };
	int bbIndex = swap->GetCurrentBackBufferIndex();
	// フェンス
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceValue = 0;
	device->CreateFence(_fenceValue, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	allocator->Reset();
	list->Reset(allocator,nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	list->OMSetRenderTargets(1,&rtvHandle, false,nullptr);
	list->ClearRenderTargetView(rtvHandle, color, 0, nullptr);

	list->Close();
	queue->ExecuteCommandLists(1,(ID3D12CommandList* const*)&list);
	swap->Present(1, 0);
	// フェンス
	++_fenceValue;
	queue->Signal(_fence, _fenceValue);
	while (_fence->GetCompletedValue() != _fenceValue) {
		
	}
}

