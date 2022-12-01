#pragma once

#define DirectX_NS DirectX
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif



//#include "Utils.h"
#define DXAPI_EXPORTS 1

#ifdef WIN32
#	ifdef DXAPI_EXPORTS
#	define DXAPI __declspec( dllexport )
#else
#	define DXAPI __declspec( dllimport )
#	endif
#else
#	define DXAPI
#endif

namespace Scalpio::Graphics
{

	public class DXAPI Graphics2D
	{
	public:
		Graphics2D();
		~Graphics2D();

		bool Init(void* hwnd);

		void Resize(UINT width, UINT height);

		void BeginDraw() { dc->BeginDraw();}
		void EndDraw() { dc->EndDraw(); }
		void Flush() { dc->Flush(); }
		void Present() { swapChain->Present(0, 0); }

		ID2D1SolidColorBrush* CreateSolidColorBrush(float r, float g, float b, float a);
		IDWriteTextFormat* CreateTextFormat(char* fontName, int fontWeight, int fontStyle, int fontStretch, float fontSize);

		void DrawText(char* string, int stringLength, char* format, float left, float top, float right, float bottom, char* brush);
		void ClearScreen(float r, float g, float b, float a);
		void DrawLine(float x1, float y1, float x2, float y2, char* resourceName, float stroke = 1.0F, ID2D1StrokeStyle* strokeStyle = (ID2D1StrokeStyle*)0);
		void DrawRectangle(float x1, float y1, float x2, float y2, char* resourceName, float stroke = 1.0F, ID2D1StrokeStyle* strokeStyle = (ID2D1StrokeStyle*)0);
		void FillRectangle(float x1, float y1, float x2, float y2, char* resourceName);

		void SetResource(char* name, void* resource);
		void LoadFontFace(char* path);
		void InitFontSet();

	private:
		void InitStaticResources();
		void CreateD3DDevice();
		void CreateDXGIDevice();
		void SetupSwapChain();
		void CreateDevInDependentResources();
		

		std::map<std::string, void*>* resources;

		HWND windowHandle;

		DXGI_PRESENT_PARAMETERS parameters = { 0 };

		IDWriteFactory5* m_pDWriteFactory;
		IDWriteFontSetBuilder1* pFontSetBuilder;
		IDWriteFontSet* pFontSet;
		IDWriteFontCollection1* fontCollection;

		DXGI_SWAP_CHAIN_DESC1 description = {};
		D2D1_BITMAP_PROPERTIES1 properties = {};
		D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

		// Direct2D
		ID2D1Bitmap1* bitmap;
		ID2D1Factory2* d2Factory;
		ID2D1Device1* d2Device;
		ID2D1DeviceContext1* dc;

		// DXGI
		IDXGISurface2* surface;
		IDXGIDevice* dxgiDevice;
		IDXGIFactory2* dxFactory;
		IDXGISwapChain1* swapChain;

		// Composition
		IDCompositionVisual* visual;
		IDCompositionTarget* target;
		IDCompositionDevice* dcompDevice;

		// Direct3D
		ID3D11Device* direct3dDevice;
	};
}