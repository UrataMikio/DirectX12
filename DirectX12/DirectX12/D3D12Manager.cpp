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
	win(win), result(S_OK)
{
	device = nullptr;

	// コマンド周りの初期化
	allocator = nullptr;
	queue = nullptr;
	list = nullptr;

	// デバイス呼び出し
	CreateDev();
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

	return result;
}
