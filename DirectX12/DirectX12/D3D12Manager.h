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

	// ��������
	void OnRender(void);

private:
	// ������
	void Initialize(void);
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
	// �f�X�N���v�^�q�[�v
	HRESULT CreateDescriptorHeap(void);
	// �����_�[�^�[�Q�b�g�r���[
	void CreateRenderTargetView(void);
	// ���[�g�V�O�l�`��
	void CreateRootSignature(void);
	// �����_�[�^�[�Q�b�g�r���[�̃N���A�R�}���h���s
	// �t�F���X���s
	// �V�F�[�_�[�ǂݍ���
	HRESULT ShaderCompile(void);
	// �p�C�v���C��
	void CreatePipeLine(void);
	// ���_���\�[�X����
	HRESULT CreateVertex(void);


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
	UINT backNum;
	ID3D12RootSignature* rootSignature;
	ID3D12PipelineState* pipeLine;

	ID3D12DescriptorHeap* rtvHeap;
	ID3D12Resource* rtvResource[2];

	ID3D12Fence* _fence;

	UINT64 _fenceValue = 0;

	// ���_�V�F�[�_�̂��傤�ق�
	ID3DBlob* vShader;
	// �s�N�Z���V�F�[�_�[�̂��傤�ق�
	ID3DBlob* pShader;

	// ���_���\�[�X
	ID3D12Resource* vRsc;
};

