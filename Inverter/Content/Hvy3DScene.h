﻿#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"


namespace Inverter
{





    struct Vertex_Pos_Tex_Normal_t
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 tex;
        DirectX::XMFLOAT3 normal;
    };




















	class Hvy3DScene
	{
	public:
		Hvy3DScene(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }


	private:


        void                        MeshMonoQuad();


	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;


        Microsoft::WRL::ComPtr<ID3D11SamplerState>              e_SamplerState;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>        e_SRV_Grid; 
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>        e_SRV_Gallo; 




		// System resources for cube geometry.

		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;
	};
}


