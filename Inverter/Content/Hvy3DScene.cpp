#include "pch.h"
#include "Hvy3DScene.h"

#include "..\Common\DirectXHelper.h"

using namespace Inverter;

using namespace DirectX;
using namespace Windows::Foundation;


Hvy3DScene::Hvy3DScene(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}







// Initializes view parameters when the window size changes.

void Hvy3DScene::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.




	// Hvy3DScene makes use of a right-handed coordinate system using row-major matrices.

	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);





    /*
	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
    */







	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixIdentity());
}
















void Hvy3DScene::Update(DX::StepTimer const& timer)
{

    //===================================================================
    //===================================================================
    //          
    //                      The Model Transform 
    //      
    // (keywords: modeltransform worldtransform world transform)
    //      


    float uniFactor = 0.7f * 0.8f;


    DirectX::XMStoreFloat4x4(
        &m_constantBufferData.model,
        XMMatrixTranspose(
            XMMatrixScaling(uniFactor, uniFactor, 1.0f)
        )
    );

    //          
    //===================================================================



}























void Hvy3DScene::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.

	if (!m_loadingComplete)
	{
		return;
	}



	auto context = m_deviceResources->GetD3DDeviceContext();

	context->UpdateSubresource1( m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0 );


	UINT stride = sizeof(Inverter::Vertex_Pos_Tex_Normal_t); // fail! this was the bug! 

	UINT offset = 0;

	context->IASetVertexBuffers( 0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset );

    
    context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
		);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());


	context->VSSetShader( m_vertexShader.Get(), nullptr, 0 );
	context->VSSetConstantBuffers1( 0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr );


	context->PSSetShader( m_pixelShader.Get(), nullptr, 0 );
    ID3D11ShaderResourceView*   twoSRVs[] = { e_SRV_Grid.Get(), e_SRV_Gallo.Get() }; 
    context->PSSetShaderResources(0, 2, twoSRVs);  
    context->PSSetSamplers(0, 1, e_SamplerState.GetAddressOf());


	context->DrawIndexed( m_indexCount, 0, 0 );
}




















void Hvy3DScene::CreateDeviceDependentResources()
{

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


    Microsoft::WRL::ComPtr<ID3D11Resource>  temp_resource;


    DX::ThrowIfFailed(
        CreateWICTextureFromFile(
            m_deviceResources->GetD3DDevice(),
            L"Assets\\a_grid.png", 
            temp_resource.ReleaseAndGetAddressOf(),
            e_SRV_Grid.ReleaseAndGetAddressOf(),
            0)
    );


    DX::ThrowIfFailed(
        CreateWICTextureFromFile(
            m_deviceResources->GetD3DDevice(),
            L"Assets\\a_gallo.png", 
            temp_resource.ReleaseAndGetAddressOf(),
            e_SRV_Gallo.ReleaseAndGetAddressOf(),
            0)
    );


    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    
    
    
    //  Sampler State for the Pixel Shader to render the Fundamental Domain Triangle and its reflections: 

    {
        D3D11_SAMPLER_DESC sampDescFunDomain;
        ZeroMemory(&sampDescFunDomain, sizeof(sampDescFunDomain));
        sampDescFunDomain.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;


        //  To render the bitmap via pixel shader use D3D11_TEXTURE_ADDRESS_???

#if 3 == 4

        sampDescFunDomain.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDescFunDomain.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDescFunDomain.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

#else 

        sampDescFunDomain.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDescFunDomain.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDescFunDomain.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
#endif 


        sampDescFunDomain.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampDescFunDomain.MinLOD = 0;
        sampDescFunDomain.MaxLOD = D3D11_FLOAT32_MAX;

        DX::ThrowIfFailed(
            m_deviceResources->GetD3DDevice()->CreateSamplerState(
                &sampDescFunDomain,
                e_SamplerState.ReleaseAndGetAddressOf()
            )
        );
    }

    
    
    
    
    
    
    
    
    
    
    // Load shaders asynchronously.

	auto loadVSTask = DX::ReadDataAsync(L"t1VertexShader.cso");

	auto loadPSTask = DX::ReadDataAsync(L"t1PixelShader.cso");



	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
				)
			);




        static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0,                             D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0,  D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },

        };





		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
				)
			);
	});














	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer) , D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
				)
			);
	});












	auto createCubeTask = (createPSTask && createVSTask).then([this] () {

        this->MeshMonoQuad();  // ghv : one quad composed of two triangles;

	});













	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this] () {
		m_loadingComplete = true;
	});
}








void Hvy3DScene::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}











void Hvy3DScene::MeshMonoQuad()
{
    // ghv : Values of texco texture coordinates will be 
    // set inside the VS vertex shader. 

    float a = 0.577f;  //  sqrt of one-third; 

    float u_min = 0.0f; float u_max = 1.f;
    float v_min = 0.0f; float v_max = 1.f;


    // u_min = 0.2f; u_max = 0.8f; v_min = 0.2f; v_max = 0.8f;


    static const Inverter::Vertex_Pos_Tex_Normal_t   monoQuadVertices[] =
    {
        //   increasing negative values of z push the mesh deeper into the background: 

        {XMFLOAT3(-1.0f, -1.0f,  -1.f), XMFLOAT2(u_min, v_max), XMFLOAT3(0.0f, 0.0f, 0.0f)},
        {XMFLOAT3(+1.0f, -1.0f,  -1.f), XMFLOAT2(u_max, v_max), XMFLOAT3(0.0f, 0.0f, 0.0f)},
        {XMFLOAT3(-1.0f, +1.0f,  -1.f), XMFLOAT2(u_min, v_min), XMFLOAT3(0.0f, 0.0f, 0.0f)},

        {XMFLOAT3(-1.0f, +1.0f,  -1.f), XMFLOAT2(u_min, v_min), XMFLOAT3(1.0f, 1.0f, 1.0f)},
        {XMFLOAT3(+1.0f, -1.0f,  -1.f), XMFLOAT2(u_max, v_max), XMFLOAT3(1.0f, 1.0f, 1.0f)},
        {XMFLOAT3(+1.0f, +1.0f,  -1.f), XMFLOAT2(u_max, v_min), XMFLOAT3(1.0f, 1.0f, 1.0f)},
    };

    D3D11_SUBRESOURCE_DATA quad_vb_data = { 0 };
    quad_vb_data.pSysMem = monoQuadVertices;
    quad_vb_data.SysMemPitch = 0;
    quad_vb_data.SysMemSlicePitch = 0;

    CD3D11_BUFFER_DESC quad_vb_descr(sizeof(monoQuadVertices), D3D11_BIND_VERTEX_BUFFER);

    DX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateBuffer( 
            &quad_vb_descr, 
            &quad_vb_data, 
            &m_vertexBuffer 
        )
    );



    //===============================================
    //  Each triangle below is FRONT_FACE_CLOCKWISE: 
    //          
    //  (cf D3D11_RASTERIZER_DESC.FrontCounterClockwise);
    //      
    //  Each trio of index entries represents one triangle. 
    //===============================================

    static const unsigned short quadIndices[] =
    {
        0,2,1,
        5,4,3,
    };

    m_indexCount = ARRAYSIZE(quadIndices);


    D3D11_SUBRESOURCE_DATA quad_ib_data = { 0 };
    quad_ib_data.pSysMem = quadIndices;
    quad_ib_data.SysMemPitch = 0;
    quad_ib_data.SysMemSlicePitch = 0;

    CD3D11_BUFFER_DESC quad_ib_descr(sizeof(quadIndices), D3D11_BIND_INDEX_BUFFER);

    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&quad_ib_descr, &quad_ib_data, &m_indexBuffer));
}
















