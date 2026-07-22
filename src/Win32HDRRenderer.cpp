#include "Win32HDRRenderer.h"

#include <SDL_syswm.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>

namespace
{
	template<typename Type>
	void Release(Type *&object)
	{
		if (object)
		{
			object->Release();
			object = nullptr;
		}
	}

	struct ViewportRect
	{
		float x = 0.0f;
		float y = 0.0f;
		float width = 1.0f;
		float height = 1.0f;
	};

	struct ShaderConstants
	{
		float logicalSize[2];
		float simulationSize[2];
		float hdrActive;
		float sdrWhiteScale;
		float peakScale;
		float saturation;
	};

	static_assert(sizeof(ShaderConstants) % 16 == 0);

	constexpr float DefaultSdrWhiteNits = 203.0f;
	constexpr float DefaultPeakNits = 400.0f;
	constexpr float MaximumPeakNits = 1000.0f;
	constexpr float HdrSaturation = 1.10f;

	const char ShaderSource[] = R"hlsl(
Texture2D FrameTexture : register(t0);
SamplerState FrameSampler : register(s0);

cbuffer FrameConstants : register(b0)
{
	float2 LogicalSize;
	float2 SimulationSize;
	float HdrActive;
	float SdrWhiteScale;
	float PeakScale;
	float Saturation;
};

struct VertexOutput
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD0;
};

VertexOutput VertexMain(uint vertexID : SV_VertexID)
{
	VertexOutput output;
	output.texcoord = float2((vertexID << 1) & 2, vertexID & 2);
	output.position = float4(
		output.texcoord.x * 2.0f - 1.0f,
		1.0f - output.texcoord.y * 2.0f,
		0.0f,
		1.0f
	);
	return output;
}

float4 PixelMain(VertexOutput input) : SV_Target
{
	// The SRV is _SRGB, so this sample is already linear light. The FP16
	// swapchain is tagged as scRGB, where 1.0 is 80 nits.
	float3 linearColour = FrameTexture.Sample(FrameSampler, input.texcoord).rgb;
	if (HdrActive < 0.5f)
	{
		return float4(linearColour, 1.0f);
	}

	float2 logicalPixel = input.texcoord * LogicalSize;
	bool simulationPixel = logicalPixel.x < SimulationSize.x && logicalPixel.y < SimulationSize.y;
	if (!simulationPixel)
	{
		// Match the Windows HDR/SDR content brightness level for menus and text.
		return float4(linearColour * SdrWhiteScale, 1.0f);
	}

	// Keep dark simulation detail near SDR white, then use FP16 headroom for
	// bright particle/emissive pixels. A small saturation expansion can create
	// valid negative scRGB components, allowing output outside the sRGB gamut.
	float brightness = max(linearColour.r, max(linearColour.g, linearColour.b));
	float highlight = smoothstep(0.08f, 1.0f, brightness);
	float boost = lerp(SdrWhiteScale, PeakScale, pow(highlight, 0.72f));
	float luma = dot(linearColour, float3(0.2126f, 0.7152f, 0.0722f));
	float3 expanded = luma.xxx + (linearColour - luma.xxx) * Saturation;
	expanded = clamp(
		expanded,
		float3(-0.25f, -0.25f, -0.25f),
		float3(4.0f, 4.0f, 4.0f)
	);
	float maxExpanded = max(expanded.r, max(expanded.g, expanded.b));
	boost = min(boost, PeakScale / max(maxExpanded, 0.001f));
	return float4(expanded * boost, 1.0f);
}
)hlsl";

	HRESULT CompileShader(const char *entryPoint, const char *profile, ID3DBlob **result)
	{
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if !defined(NDEBUG)
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ID3DBlob *errors = nullptr;
		auto status = D3DCompile(
			ShaderSource,
			sizeof(ShaderSource) - 1,
			"TPT Win32 HDR presenter",
			nullptr,
			nullptr,
			entryPoint,
			profile,
			flags,
			0,
			result,
			&errors
		);
		if (FAILED(status) && errors)
		{
			fprintf(stderr, "D3DCompile(%s) failed: %.*s\n", entryPoint,
				int(errors->GetBufferSize()), static_cast<const char *>(errors->GetBufferPointer()));
		}
		Release(errors);
		return status;
	}
}

struct Win32HDRRenderer
{
	HWND hwnd = nullptr;
	int logicalWidth = 0;
	int logicalHeight = 0;
	int simulationWidth = 0;
	int simulationHeight = 0;
	int backBufferWidth = 0;
	int backBufferHeight = 0;
	bool outputDirty = true;
	bool hdrActive = false;
	unsigned int outputRefreshCountdown = 0;
	float sdrWhiteScale = DefaultSdrWhiteNits / 80.0f;
	float peakScale = DefaultPeakNits / 80.0f;
	ViewportRect viewport;

	ID3D11Device *device = nullptr;
	ID3D11DeviceContext *context = nullptr;
	IDXGIFactory2 *swapChainFactory = nullptr;
	IDXGIFactory1 *outputFactory = nullptr;
	IDXGISwapChain3 *swapChain = nullptr;
	ID3D11RenderTargetView *renderTarget = nullptr;
	ID3D11Texture2D *sourceTexture = nullptr;
	ID3D11ShaderResourceView *sourceView = nullptr;
	ID3D11VertexShader *vertexShader = nullptr;
	ID3D11PixelShader *pixelShader = nullptr;
	ID3D11Buffer *constantBuffer = nullptr;
	ID3D11SamplerState *pointSampler = nullptr;
	ID3D11SamplerState *linearSampler = nullptr;
	ID3D11RasterizerState *rasterizer = nullptr;

	~Win32HDRRenderer()
	{
		if (context)
		{
			context->ClearState();
		}
		Release(rasterizer);
		Release(linearSampler);
		Release(pointSampler);
		Release(constantBuffer);
		Release(pixelShader);
		Release(vertexShader);
		Release(sourceView);
		Release(sourceTexture);
		Release(renderTarget);
		Release(swapChain);
		Release(outputFactory);
		Release(swapChainFactory);
		Release(context);
		Release(device);
	}

	ViewportRect CalculateViewport(bool integerScaling) const
	{
		RECT clientRect = {};
		GetClientRect(hwnd, &clientRect);
		float clientWidth = float(std::max<LONG>(clientRect.right - clientRect.left, 1));
		float clientHeight = float(std::max<LONG>(clientRect.bottom - clientRect.top, 1));
		float scale = std::min(clientWidth / float(logicalWidth), clientHeight / float(logicalHeight));
		if (integerScaling && scale >= 1.0f)
		{
			scale = std::floor(scale);
		}
		ViewportRect result;
		result.width = std::max(float(logicalWidth) * scale, 1.0f);
		result.height = std::max(float(logicalHeight) * scale, 1.0f);
		result.x = (clientWidth - result.width) * 0.5f;
		result.y = (clientHeight - result.height) * 0.5f;
		return result;
	}

	bool CreateRenderTarget()
	{
		ID3D11Texture2D *backBuffer = nullptr;
		auto status = swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		if (SUCCEEDED(status))
		{
			status = device->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
		}
		Release(backBuffer);
		return SUCCEEDED(status);
	}

	bool ResizeIfNeeded()
	{
		RECT clientRect = {};
		GetClientRect(hwnd, &clientRect);
		int width = int(clientRect.right - clientRect.left);
		int height = int(clientRect.bottom - clientRect.top);
		if (width <= 0 || height <= 0)
		{
			return false;
		}
		if (width == backBufferWidth && height == backBufferHeight && renderTarget)
		{
			return true;
		}

		context->OMSetRenderTargets(0, nullptr, nullptr);
		Release(renderTarget);
		auto status = swapChain->ResizeBuffers(0, UINT(width), UINT(height), DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(status))
		{
			return false;
		}
		backBufferWidth = width;
		backBufferHeight = height;
		return CreateRenderTarget();
	}

	void RecreateOutputFactory()
	{
		Release(outputFactory);
		CreateDXGIFactory1(IID_PPV_ARGS(&outputFactory));
	}

	void RefreshOutputInfo()
	{
		outputDirty = false;
		outputRefreshCountdown = 30;
		hdrActive = false;
		sdrWhiteScale = DefaultSdrWhiteNits / 80.0f;
		peakScale = DefaultPeakNits / 80.0f;

		if (!outputFactory || !outputFactory->IsCurrent())
		{
			RecreateOutputFactory();
		}
		if (!outputFactory)
		{
			return;
		}

		HMONITOR windowMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		for (UINT adapterIndex = 0; ; ++adapterIndex)
		{
			IDXGIAdapter1 *adapter = nullptr;
			auto adapterStatus = outputFactory->EnumAdapters1(adapterIndex, &adapter);
			if (FAILED(adapterStatus))
			{
				break;
			}
			if (!adapter)
			{
				continue;
			}
			for (UINT outputIndex = 0; ; ++outputIndex)
			{
				IDXGIOutput *output = nullptr;
				auto enumStatus = adapter->EnumOutputs(outputIndex, &output);
				if (enumStatus == DXGI_ERROR_NOT_FOUND)
				{
					break;
				}
				if (FAILED(enumStatus) || !output)
				{
					continue;
				}
				DXGI_OUTPUT_DESC outputDesc = {};
				output->GetDesc(&outputDesc);
				if (outputDesc.Monitor == windowMonitor)
				{
					IDXGIOutput6 *output6 = nullptr;
					if (SUCCEEDED(output->QueryInterface(IID_PPV_ARGS(&output6))))
					{
						DXGI_OUTPUT_DESC1 outputDesc1 = {};
						if (SUCCEEDED(output6->GetDesc1(&outputDesc1)))
						{
							hdrActive = outputDesc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
							if (hdrActive)
							{
								float reportedPeak = outputDesc1.MaxLuminance > 100.0f
									? outputDesc1.MaxLuminance
									: DefaultPeakNits;
								float targetPeak = std::clamp(reportedPeak, DefaultSdrWhiteNits, MaximumPeakNits);
								peakScale = targetPeak / 80.0f;
							}
						}
					}
					Release(output6);
					Release(output);
					Release(adapter);
					return;
				}
				Release(output);
			}
			Release(adapter);
		}
	}

	bool Initialize(SDL_Window *window)
	{
		wchar_t disableHdr[2] = {};
		if (GetEnvironmentVariableW(L"TPT_DISABLE_HDR", disableHdr, 2))
		{
			return false;
		}

		SDL_SysWMinfo windowInfo = {};
		SDL_VERSION(&windowInfo.version);
		if (!SDL_GetWindowWMInfo(window, &windowInfo) || windowInfo.subsystem != SDL_SYSWM_WINDOWS)
		{
			return false;
		}
		hwnd = windowInfo.info.win.window;

		D3D_FEATURE_LEVEL requestedLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		D3D_FEATURE_LEVEL obtainedLevel = D3D_FEATURE_LEVEL_10_0;
		auto status = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			requestedLevels,
			UINT(std::size(requestedLevels)),
			D3D11_SDK_VERSION,
			&device,
			&obtainedLevel,
			&context
		);
		if (status == E_INVALIDARG)
		{
			// Windows 7's D3D11 runtime does not know the 11.1 enum value.
			status = D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				D3D11_CREATE_DEVICE_BGRA_SUPPORT,
				requestedLevels + 1,
				UINT(std::size(requestedLevels) - 1),
				D3D11_SDK_VERSION,
				&device,
				&obtainedLevel,
				&context
			);
		}
		if (FAILED(status))
		{
			status = D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_WARP,
				nullptr,
				D3D11_CREATE_DEVICE_BGRA_SUPPORT,
				requestedLevels + 1,
				UINT(std::size(requestedLevels) - 1),
				D3D11_SDK_VERSION,
				&device,
				&obtainedLevel,
				&context
			);
		}
		if (FAILED(status))
		{
			return false;
		}

		IDXGIDevice *dxgiDevice = nullptr;
		IDXGIAdapter *adapter = nullptr;
		status = device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
		if (SUCCEEDED(status))
		{
			status = dxgiDevice->GetAdapter(&adapter);
		}
		if (SUCCEEDED(status))
		{
			status = adapter->GetParent(IID_PPV_ARGS(&swapChainFactory));
		}
		Release(adapter);
		Release(dxgiDevice);
		if (FAILED(status) || !swapChainFactory)
		{
			return false;
		}

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		IDXGISwapChain1 *swapChain1 = nullptr;
		status = swapChainFactory->CreateSwapChainForHwnd(
			device,
			hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1
		);
		if (SUCCEEDED(status))
		{
			status = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain));
		}
		Release(swapChain1);
		if (FAILED(status) || !swapChain)
		{
			return false;
		}
		swapChainFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

		UINT colourSpaceSupport = 0;
		status = swapChain->CheckColorSpaceSupport(
			DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709,
			&colourSpaceSupport
		);
		if (FAILED(status) || !(colourSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
		{
			return false;
		}
		if (FAILED(swapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709)))
		{
			return false;
		}

		RECT clientRect = {};
		GetClientRect(hwnd, &clientRect);
		backBufferWidth = int(clientRect.right - clientRect.left);
		backBufferHeight = int(clientRect.bottom - clientRect.top);
		if (!CreateRenderTarget())
		{
			return false;
		}

		D3D11_TEXTURE2D_DESC sourceDesc = {};
		sourceDesc.Width = UINT(logicalWidth);
		sourceDesc.Height = UINT(logicalHeight);
		sourceDesc.MipLevels = 1;
		sourceDesc.ArraySize = 1;
		sourceDesc.Format = DXGI_FORMAT_B8G8R8A8_TYPELESS;
		sourceDesc.SampleDesc.Count = 1;
		sourceDesc.Usage = D3D11_USAGE_DYNAMIC;
		sourceDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		sourceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		if (FAILED(device->CreateTexture2D(&sourceDesc, nullptr, &sourceTexture)))
		{
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC sourceViewDesc = {};
		sourceViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		sourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		sourceViewDesc.Texture2D.MipLevels = 1;
		if (FAILED(device->CreateShaderResourceView(sourceTexture, &sourceViewDesc, &sourceView)))
		{
			return false;
		}

		ID3DBlob *vertexBlob = nullptr;
		ID3DBlob *pixelBlob = nullptr;
		if (FAILED(CompileShader("VertexMain", "vs_4_0", &vertexBlob)) ||
			FAILED(CompileShader("PixelMain", "ps_4_0", &pixelBlob)))
		{
			Release(vertexBlob);
			Release(pixelBlob);
			return false;
		}
		status = device->CreateVertexShader(
			vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &vertexShader);
		if (SUCCEEDED(status))
		{
			status = device->CreatePixelShader(
				pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &pixelShader);
		}
		Release(vertexBlob);
		Release(pixelBlob);
		if (FAILED(status))
		{
			return false;
		}

		D3D11_BUFFER_DESC constantDesc = {};
		constantDesc.ByteWidth = sizeof(ShaderConstants);
		constantDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		if (FAILED(device->CreateBuffer(&constantDesc, nullptr, &constantBuffer)))
		{
			return false;
		}

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		if (FAILED(device->CreateSamplerState(&samplerDesc, &pointSampler)))
		{
			return false;
		}
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		if (FAILED(device->CreateSamplerState(&samplerDesc, &linearSampler)))
		{
			return false;
		}

		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.DepthClipEnable = TRUE;
		if (FAILED(device->CreateRasterizerState(&rasterizerDesc, &rasterizer)))
		{
			return false;
		}

		RecreateOutputFactory();
		RefreshOutputInfo();
		return true;
	}
};

Win32HDRRenderer *Win32HDRCreate(
	SDL_Window *window,
	int logicalWidth,
	int logicalHeight,
	int simulationWidth,
	int simulationHeight
)
{
	auto *renderer = new Win32HDRRenderer;
	renderer->logicalWidth = logicalWidth;
	renderer->logicalHeight = logicalHeight;
	renderer->simulationWidth = simulationWidth;
	renderer->simulationHeight = simulationHeight;
	if (!renderer->Initialize(window))
	{
		delete renderer;
		return nullptr;
	}
	fprintf(stderr, "Using Windows FP16/scRGB presenter (HDR activates automatically with Windows).\n");
	return renderer;
}

void Win32HDRDestroy(Win32HDRRenderer *renderer)
{
	delete renderer;
}

bool Win32HDRPresent(
	Win32HDRRenderer *renderer,
	const void *pixels,
	int sourcePitch,
	bool linearFiltering,
	bool integerScaling
)
{
	if (!renderer || !pixels)
	{
		return false;
	}
	if (renderer->outputRefreshCountdown > 0)
	{
		--renderer->outputRefreshCountdown;
	}
	if (renderer->outputDirty || renderer->outputRefreshCountdown == 0 ||
		!renderer->outputFactory || !renderer->outputFactory->IsCurrent())
	{
		renderer->RefreshOutputInfo();
	}
	if (!renderer->ResizeIfNeeded())
	{
		// A zero-sized client area is normal while minimized.
		return IsIconic(renderer->hwnd) != FALSE;
	}

	D3D11_MAPPED_SUBRESOURCE mappedSource = {};
	if (FAILED(renderer->context->Map(
		renderer->sourceTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSource)))
	{
		return false;
	}
	for (int y = 0; y < renderer->logicalHeight; ++y)
	{
		memcpy(
			static_cast<unsigned char *>(mappedSource.pData) + size_t(y) * mappedSource.RowPitch,
			static_cast<const unsigned char *>(pixels) + size_t(y) * sourcePitch,
			size_t(renderer->logicalWidth) * 4
		);
	}
	renderer->context->Unmap(renderer->sourceTexture, 0);

	D3D11_MAPPED_SUBRESOURCE mappedConstants = {};
	if (FAILED(renderer->context->Map(
		renderer->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedConstants)))
	{
		return false;
	}
	auto *constants = static_cast<ShaderConstants *>(mappedConstants.pData);
	constants->logicalSize[0] = float(renderer->logicalWidth);
	constants->logicalSize[1] = float(renderer->logicalHeight);
	constants->simulationSize[0] = float(renderer->simulationWidth);
	constants->simulationSize[1] = float(renderer->simulationHeight);
	constants->hdrActive = renderer->hdrActive ? 1.0f : 0.0f;
	constants->sdrWhiteScale = renderer->sdrWhiteScale;
	constants->peakScale = renderer->peakScale;
	constants->saturation = HdrSaturation;
	renderer->context->Unmap(renderer->constantBuffer, 0);

	const float clearColour[4] = {};
	renderer->context->ClearRenderTargetView(renderer->renderTarget, clearColour);
	renderer->viewport = renderer->CalculateViewport(integerScaling);
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = renderer->viewport.x;
	viewport.TopLeftY = renderer->viewport.y;
	viewport.Width = renderer->viewport.width;
	viewport.Height = renderer->viewport.height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	renderer->context->RSSetViewports(1, &viewport);
	renderer->context->RSSetState(renderer->rasterizer);
	renderer->context->OMSetRenderTargets(1, &renderer->renderTarget, nullptr);
	renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderer->context->VSSetShader(renderer->vertexShader, nullptr, 0);
	renderer->context->PSSetShader(renderer->pixelShader, nullptr, 0);
	renderer->context->PSSetShaderResources(0, 1, &renderer->sourceView);
	renderer->context->PSSetConstantBuffers(0, 1, &renderer->constantBuffer);
	ID3D11SamplerState *sampler = linearFiltering ? renderer->linearSampler : renderer->pointSampler;
	renderer->context->PSSetSamplers(0, 1, &sampler);
	renderer->context->Draw(3, 0);

	ID3D11ShaderResourceView *nullView = nullptr;
	renderer->context->PSSetShaderResources(0, 1, &nullView);
	auto presentStatus = renderer->swapChain->Present(0, 0);
	return SUCCEEDED(presentStatus);
}

void Win32HDRInvalidateOutput(Win32HDRRenderer *renderer)
{
	if (renderer)
	{
		renderer->outputDirty = true;
	}
}

void Win32HDRWindowToLogical(
	Win32HDRRenderer *renderer,
	int windowX,
	int windowY,
	int *logicalX,
	int *logicalY,
	bool integerScaling
)
{
	if (!renderer)
	{
		return;
	}
	auto viewport = renderer->CalculateViewport(integerScaling);
	if (logicalX)
	{
		*logicalX = int(std::floor((float(windowX) - viewport.x) *
			float(renderer->logicalWidth) / viewport.width));
	}
	if (logicalY)
	{
		*logicalY = int(std::floor((float(windowY) - viewport.y) *
			float(renderer->logicalHeight) / viewport.height));
	}
}

void Win32HDRLogicalToWindow(
	Win32HDRRenderer *renderer,
	float logicalX,
	float logicalY,
	int *windowX,
	int *windowY,
	bool integerScaling
)
{
	if (!renderer)
	{
		return;
	}
	auto viewport = renderer->CalculateViewport(integerScaling);
	if (windowX)
	{
		*windowX = int(std::lround(viewport.x + logicalX * viewport.width /
			float(renderer->logicalWidth)));
	}
	if (windowY)
	{
		*windowY = int(std::lround(viewport.y + logicalY * viewport.height /
			float(renderer->logicalHeight)));
	}
}

void Win32HDRGetMousePosition(
	Win32HDRRenderer *renderer,
	int *logicalX,
	int *logicalY,
	bool integerScaling
)
{
	if (!renderer)
	{
		return;
	}
	POINT cursor = {};
	GetCursorPos(&cursor);
	ScreenToClient(renderer->hwnd, &cursor);
	Win32HDRWindowToLogical(
		renderer, int(cursor.x), int(cursor.y), logicalX, logicalY, integerScaling);
}
