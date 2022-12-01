#include "pch.h"
#include "Graphics2D.h"

using namespace Scalpio::Graphics;

struct ComException
{
	HRESULT result;
	ComException(HRESULT const value) :
		result(value)
	{}
};
void HR(HRESULT const result)
{
	if (S_OK != result)
	{
		throw ComException(result);
	}
}

namespace Scalpio::Graphics
{


	Graphics2D::Graphics2D()
	{
		bitmap = nullptr;
		d2Factory = nullptr;
		d2Device = nullptr;
		dc = nullptr;
		surface = nullptr;
		visual = nullptr;

		target = nullptr;
		dcompDevice = nullptr;
		direct3dDevice = nullptr;
		dxgiDevice = nullptr;
		dxFactory = nullptr;
		swapChain = nullptr;
		m_pDWriteFactory = nullptr;

		resources = new std::map<std::string, void*>();
	}

	Graphics2D::~Graphics2D()
	{
		bitmap->Release();
		d2Factory->Release();
		d2Device->Release();
		dc->Release();
		surface->Release();
		visual->Release();

		target->Release();
		dcompDevice->Release();
		direct3dDevice->Release();
		dxgiDevice->Release();
		dxFactory->Release();
		swapChain->Release();
		
		for (const auto& [key, value] : *resources)
			(static_cast<ID2D1Brush*>(value))->Release();
	}

	void Graphics2D::InitFontSet() {
		HR(pFontSetBuilder->CreateFontSet(&pFontSet));

		HR(m_pDWriteFactory->CreateFontCollectionFromFontSet(pFontSet, &fontCollection));
	}

	void Graphics2D::CreateDevInDependentResources() {
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory5),
			reinterpret_cast<IUnknown**>(&m_pDWriteFactory));

		HR(m_pDWriteFactory->CreateFontSetBuilder(&pFontSetBuilder));
	}

	void Graphics2D::LoadFontFace(char* path) {
		wchar_t* vOut = new wchar_t[strlen(path) + 1];
		mbstowcs_s(NULL, vOut, strlen(path) + 1, path, strlen(path));
			
		IDWriteFontFile* pFontFile;
		HR(m_pDWriteFactory->CreateFontFileReference(vOut, /* lastWriteTime*/ nullptr, &pFontFile));
		HR(pFontSetBuilder->AddFontFile(pFontFile));
	}

	void Graphics2D::InitStaticResources()
	{
		parameters.DirtyRectsCount = 0; 
		parameters.pDirtyRects = nullptr; 
		parameters.pScrollRect = nullptr; 
		parameters.pScrollOffset = nullptr;

		// DXGI_SWAP_CHAIN_DESC
		description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		description.BufferCount = 2;
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

		// D2D1_BITMAP_PROPERTIES
		properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
		properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

		d2dFactoryOptions = { D2D1_DEBUG_LEVEL_INFORMATION };
	};

	void Graphics2D::CreateD3DDevice()
	{
		HR(D3D11CreateDevice(nullptr,    // адаптер
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,    // модуль
			D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			nullptr,
			0, // высший доступный уровень возможностей
			D3D11_SDK_VERSION,
			&direct3dDevice,
			nullptr,    // реальный уровень возможностей
			nullptr));  // контекст устройства
	};

	void Graphics2D::CreateDXGIDevice()
	{
		if (direct3dDevice != nullptr) {
			HR(direct3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

			HR(CreateDXGIFactory2(
				DXGI_CREATE_FACTORY_DEBUG,
				__uuidof(dxFactory),
				reinterpret_cast<void**>(&dxFactory)));
		}
	};

	void Graphics2D::SetupSwapChain()
	{
		if (dxgiDevice != nullptr) {
			HR(dxFactory->CreateSwapChainForComposition(dxgiDevice,
				&description,
				nullptr, // не ограничиваем
				&swapChain));
		}

		HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,
			d2dFactoryOptions,
			&d2Factory));

		if (dxgiDevice != nullptr) {
			HR(d2Factory->CreateDevice(dxgiDevice,
				&d2Device));
		}

		HR(d2Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
			&dc));

		dc->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

		HR(swapChain->GetBuffer(
			0, // индекс
			__uuidof(surface),
			reinterpret_cast<void**>(&surface)));
		
	};

	void Graphics2D::Resize(UINT width, UINT height) {
		RECT rect;
		GetClientRect(windowHandle, &rect);

		description.Width = width;
		description.Height = height;

		swapChain->Release();
		surface->Release();
		bitmap->Release();
		dcompDevice->Release();
		target->Release();
		visual->Release();

		swapChain = nullptr;
		surface = nullptr;
		bitmap = nullptr;
		dcompDevice = nullptr;
		target = nullptr;
		visual = nullptr;

		HR(dxFactory->CreateSwapChainForComposition(dxgiDevice,
			&description,
			nullptr, // не ограничиваем
			&swapChain));
		
		HR(swapChain->GetBuffer(
			0, // индекс
			__uuidof(surface),
			reinterpret_cast<void**>(&surface)));
		
		if (surface != nullptr) {
			HR(dc->CreateBitmapFromDxgiSurface(surface,
				properties,
				&bitmap));
		}
		
		dc->SetTarget(bitmap);
		
		HR(DCompositionCreateDevice(
			dxgiDevice,
			__uuidof(dcompDevice),
			reinterpret_cast<void**>(&dcompDevice)));

		HR(dcompDevice->CreateTargetForHwnd(windowHandle,
			true, // самое верхнее
			&target));
	
		HR(dcompDevice->CreateVisual(&visual));
		HR(visual->SetContent(swapChain));
		HR(target->SetRoot(visual));
		HR(dcompDevice->Commit());
	};

	bool Graphics2D::Init(void* hwnd)
	{
		windowHandle = (HWND)hwnd;

		RECT rect;
		GetClientRect(windowHandle, &rect);

		InitStaticResources();
		CreateD3DDevice();

		description.Width = rect.right - rect.left;
		description.Height = rect.bottom - rect.top;

		CreateDXGIDevice();
		SetupSwapChain();

		if (surface != nullptr) {
			HR(dc->CreateBitmapFromDxgiSurface(surface,
				properties,
				&bitmap));
		}

		dc->SetTarget(bitmap);
		
		HR(DCompositionCreateDevice(
			dxgiDevice,
			__uuidof(dcompDevice),
			reinterpret_cast<void**>(&dcompDevice)));

		HR(dcompDevice->CreateTargetForHwnd(windowHandle,
			true, // самое верхнее
			&target));

		HR(dcompDevice->CreateVisual(&visual));
		HR(visual->SetContent(swapChain));
		HR(target->SetRoot(visual));
		HR(dcompDevice->Commit());

		CreateDevInDependentResources();

		return true;
	}

	void Graphics2D::DrawText(char *string, int stringLength, char* format, float left, float top, float right, float bottom, char* brush) {

		std::string brushStr(brush);
		std::string formatStr(format);

		auto brushIt = resources->find(brushStr);
		auto formatIt = resources->find(formatStr);

		void* brushResource = brushIt->second;
		void* formatResource = formatIt->second;

		wchar_t* stringOut = new wchar_t[strlen(string) + 1];
		mbstowcs_s(NULL, stringOut, strlen(string) + 1, string, strlen(string));

		auto brushCast = static_cast<ID2D1Brush*>(brushResource);
		auto formatCast = static_cast<IDWriteTextFormat*>(formatResource);

		dc->DrawText(stringOut, stringLength, formatCast, D2D1::RectF(left, top, right, bottom), brushCast);
	}

	void Graphics2D::ClearScreen(float r, float g, float b, float a) {
		
		dc->Clear(D2D1::ColorF(r, g, b, a));
	}

	void Graphics2D::DrawLine(float x1, float y1, float x2, float y2, char* resourceName, float stroke, ID2D1StrokeStyle* strokeStyle) {

		std::string str(resourceName);
		auto it = resources->find(str);

		void* resource = it->second;

		if (resource != nullptr) {

			auto cast = static_cast<ID2D1Brush*>(resource);

			dc->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2), cast, 1.0f);
		}
	}

	void Graphics2D::DrawRectangle(float left, float top, float right, float bottom, char* resourceName, float stroke, ID2D1StrokeStyle* strokeStyle) {

		std::string str(resourceName);
		auto it = resources->find(str);

		void* resource = it->second;

		if (resource != nullptr) {

			auto cast = static_cast<ID2D1Brush*>(resource);

			dc->DrawRectangle(D2D1::RectF(left, top, right, bottom), cast);
		}
	}
	void Graphics2D::FillRectangle(float x1, float y1, float x2, float y2, char* resourceName)
	{
		std::string str(resourceName);
		auto it = resources->find(str);

		void* resource = it->second;

		if (resource != nullptr) {

			auto cast = static_cast<ID2D1Brush*>(resource);

			dc->FillRectangle(D2D1::RectF(x1, y1, x2, y2), cast);
		}
	}
	
	ID2D1SolidColorBrush* Graphics2D::CreateSolidColorBrush(float r, float g, float b, float a) {
		ID2D1SolidColorBrush* brush;

		HR(dc->CreateSolidColorBrush(D2D1::ColorF(r, g, b, a), &brush));

		return brush;
	}

	IDWriteTextFormat* Graphics2D::CreateTextFormat(char* fontName, int fontWeight, int fontStyle, int fontStretch, float fontSize) {
		IDWriteTextFormat* pTextFormat_;

		wchar_t* vOut = new wchar_t[strlen(fontName) + 1];
		mbstowcs_s(NULL, vOut, strlen(fontName) + 1, fontName, strlen(fontName));

		HR(m_pDWriteFactory->CreateTextFormat(
			vOut,
			NULL,
			static_cast<DWRITE_FONT_WEIGHT>(fontWeight),
			static_cast<DWRITE_FONT_STYLE>(fontStyle),
			static_cast<DWRITE_FONT_STRETCH>(fontStretch),
			fontSize,
			L"en-us",
			&pTextFormat_
		));

		return pTextFormat_;
	}

	void Graphics2D::SetResource(char* name, void* resource) {
		std::string str(name);

		if (resources != nullptr) {
			if (!resources->count(str)) {
				resources->insert(std::make_pair(str, resource));
			}
		}
	}
}

