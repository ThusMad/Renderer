#pragma once
#include "Graphics2D.h"
#include <windows.h>
#include <vcclr.h>
#using <System.dll>
#using <WindowsBase.dll>
#include <vector>

using namespace System;
using namespace Scalpio::Graphics;

public ref class DXGraphics2D
{

public:
	DXGraphics2D() : m_Impl(new Graphics2D) {}

	~DXGraphics2D()
	{
		delete m_Impl;
	}

protected:
	!DXGraphics2D()
	{
		delete m_Impl;
	}

public:

	bool Init(void* hwnd)
	{
		return m_Impl->Init(hwnd);
	}

	void Resize(int width, int height)
	{
		m_Impl->Resize(width, height);
	}

	void ClearScreen(float r, float g, float b, float a)
	{
		return m_Impl->ClearScreen(r, g, b, a);
	}

	void DrawLine(float x1, float y1, float x2, float y2, char* brushName, float stroke)
	{
		return m_Impl->DrawLine(x1, y1, x2, y2, brushName, stroke);
	}

	void DrawRectangle(float left, float top, float right, float bottom, char* brushName, float stroke)
	{
		return m_Impl->DrawRectangle(left, top, right, bottom, brushName, stroke);
	}

	void FillRectangle(float left, float top, float right, float bottom, char* brushName)
	{
		return m_Impl->FillRectangle(left, top, right, bottom, brushName);
	}

	void CreateSolidColorBrush(char* name, float r, float g, float b, float a) {
		m_Impl->SetResource(name, m_Impl->CreateSolidColorBrush(r, g, b, a));
	}

	void DrawText(char* string, int stringLength, char* format, float left, float top, float right, float bottom, char* brush) {
		m_Impl->DrawText(string, stringLength, format, left, top, right, bottom, brush);
	}

	void LoadFontFace(char* path) {
		m_Impl->LoadFontFace(path);
	}

	void CreateTextFormat(char* name, char* fontName, int fontWeight, int fontStyle, int fontStretch, float fontSize) {
		m_Impl->SetResource(name, m_Impl->CreateTextFormat(fontName, fontWeight, fontStyle, fontStretch, fontSize));
	}

	void BeginDraw()
	{
		return m_Impl->BeginDraw();
	}

	void EndDraw()
	{
		return m_Impl->EndDraw();
	}

	void Flush()
	{
		return m_Impl->Flush();
	}

	void Present()
	{
		return m_Impl->Present();
	}

	Graphics2D* m_Impl;
};