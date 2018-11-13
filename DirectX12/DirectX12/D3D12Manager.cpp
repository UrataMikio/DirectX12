#include "D3D12Manager.h"
#include "Window.h"
#include "DirectXMath.h"
#include <d3dcompiler.h>
#include <tchar.h>
#include <DirectXTex.h>

#pragma comment(lib,"d3d12.lib") 
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")

//DirectX::LoadFromWICFile(
//wfileName,	←unicodeで書く！
//0,
//&DirectX::TexMetadata(),
//DirectX::ScratchImage()
//)

const FLOAT color[] = { 0.0f,1.0f,0.0f,1.0f };

namespace {
	Vertex vertex[] = {
		{{-1.0f / 2, 1.0f / 2, 0.0f}},
		{{ 1.0f / 2, 1.0f / 2, 0.0, }},
		{{-1.0f / 2, -1.0f / 2, 0.0f}},
		{{ 1.0f / 2, -1.0f / 2, 0.0f}}
	};
}

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

#ifdef _DEBUG
	ID3D12Debug* debug = nullptr;
	auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
	debug->EnableDebugLayer();

#endif

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
	// フェン
	device->CreateFence(_fenceValue, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	// シェーダ呼び出し
	ShaderCompile();
	// パイプライン呼び出し
	CreatePipeLine();
	// 頂点呼び出し
	CreateVertex();
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
	ID3DBlob* signature = nullptr;
	ID3DBlob* error = nullptr;

	desc.Flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12SerializeRootSignature( &desc, D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1 , &signature, &error);
	result = device->CreateRootSignature(0, signature->GetBufferPointer(),signature->GetBufferSize(),IID_PPV_ARGS(&rootSignature));
}

// シェーダー読み込み
HRESULT D3D12Manager::ShaderCompile(void)
{
	result = D3DCompileFromFile(_T("Texture.hlsl"), nullptr, nullptr, "VS", "vs_5_1", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vShader, nullptr);	result = D3DCompileFromFile(_T("Texture.hlsl"), nullptr, nullptr, "PS", "ps_5_1", D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pShader, nullptr);
	return result;
}

void D3D12Manager::CreatePipeLine(void)
{
	D3D12_INPUT_ELEMENT_DESC input = {};
	input.SemanticName = "POSITION";
	input.SemanticIndex = 0;
	input.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
	input.InputSlot = 0;
	input.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	input.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	input.InstanceDataStepRate = 0;

	D3D12_RASTERIZER_DESC rasterizer = {};
	rasterizer.AntialiasedLineEnable = FALSE;
	rasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE::D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	rasterizer.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	rasterizer.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizer.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizer.DepthClipEnable = TRUE;
	rasterizer.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
	rasterizer.ForcedSampleCount = 0;
	rasterizer.FrontCounterClockwise = FALSE;
	rasterizer.MultisampleEnable = FALSE;
	rasterizer.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

	//レンダーターゲットブレンド設定用構造体
	D3D12_RENDER_TARGET_BLEND_DESC renderBlend = {};
	renderBlend.BlendEnable = TRUE;
	renderBlend.BlendOp = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
	renderBlend.BlendOpAlpha = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
	renderBlend.DestBlend = D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
	renderBlend.DestBlendAlpha = D3D12_BLEND::D3D12_BLEND_ZERO;
	renderBlend.LogicOp = D3D12_LOGIC_OP::D3D12_LOGIC_OP_NOOP;
	renderBlend.LogicOpEnable = FALSE;
	renderBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE::D3D12_COLOR_WRITE_ENABLE_ALL;
	renderBlend.SrcBlend = D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
	renderBlend.SrcBlendAlpha = D3D12_BLEND::D3D12_BLEND_ONE;

	//ブレンドステート設定用構造体
	D3D12_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = FALSE;
	blend.IndependentBlendEnable = FALSE;
	for (UINT i = 0; i < 2; i++)
	{
		blend.RenderTarget[i] = renderBlend;
	}

	//パイプラインステート設定用構造体
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.BlendState = blend;
	desc.DepthStencilState.DepthEnable = false;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
	desc.DepthStencilState.StencilEnable = FALSE;
	desc.DSVFormat = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	desc.InputLayout = { &input, 1 };
	desc.NumRenderTargets = 1;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.pRootSignature = rootSignature;
	desc.PS.pShaderBytecode = pShader->GetBufferPointer();
	desc.PS.BytecodeLength = pShader->GetBufferSize();
	desc.RasterizerState = rasterizer;
	desc.RTVFormats[0] = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleMask = UINT_MAX;
	desc.SampleDesc.Count = 1;
	desc.VS.pShaderBytecode = vShader->GetBufferPointer();
	desc.VS.BytecodeLength = vShader->GetBufferSize();

	result = device->CreateGraphicsPipelineState(&desc,IID_PPV_ARGS(&pipeLine));
}

HRESULT D3D12Manager::CreateVertex(void)
{
	D3D12_HEAP_PROPERTIES prop = {};
	//lightbuck に L0 の 1,1
	prop.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = 256;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	result = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vRsc)
	);
	void* data = nullptr;
	vRsc->Map(0, nullptr, &data);

	memcpy(data, &vertex[0], sizeof(vertex));

	vRsc->Unmap(0, nullptr);

	return result;
}

// 毎日処理
void D3D12Manager::OnRender(void)
{
	int bbIndex = swap->GetCurrentBackBufferIndex();

	allocator->Reset();
	list->Reset(allocator, pipeLine);

	list->SetGraphicsRootSignature(rootSignature);

	list->SetPipelineState(pipeLine);

	//ビューポートのセット
	{
		D3D12_VIEWPORT view = {};
		view.Height = static_cast<float>(480);
		view.MaxDepth = 1.0f;
		view.MinDepth = 0.0f;
		view.TopLeftX = 0.0f;
		view.TopLeftY = 0.0f;
		view.Width = static_cast<float>(640);

		list->RSSetViewports(1, &view);
	}

	//シザーセット
	{
		RECT scissor = {};
		scissor.bottom = static_cast<LONG>(480);
		scissor.left = 0;
		scissor.right = static_cast<LONG>(640);
		scissor.top = 0;

		list->RSSetScissorRects(1, &scissor);
	}


	//バリアの設定
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = rtvResource[bbIndex];
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
		list->ResourceBarrier(1, &barrier);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	list->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
	list->ClearRenderTargetView(rtvHandle, color, 0, nullptr);

	//頂点バッファビューのセット
	{
		//頂点バッファ設定用構造体
		D3D12_VERTEX_BUFFER_VIEW view = {};
		view.BufferLocation = vRsc->GetGPUVirtualAddress();
		view.SizeInBytes = sizeof(vertex);
		view.StrideInBytes = sizeof(Vertex);

		list->IASetVertexBuffers(0, 1, &view);
	}

	//描画
	{
		list->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		list->DrawInstanced(_countof(vertex), 1, 0, 0);
	}

	//バリア設置
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = rtvResource[bbIndex];
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
		list->ResourceBarrier(1, &barrier);
	}


	list->Close();
	queue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&list);
	swap->Present(1, 0);
	// フェンス
	++_fenceValue;
	queue->Signal(_fence, _fenceValue);
	while (_fence->GetCompletedValue() != _fenceValue) {

	}
}
