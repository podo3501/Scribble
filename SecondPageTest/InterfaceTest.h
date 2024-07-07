#pragma once

#include "../Include/Interface.h"

interface ITestRenderer : public IRenderer
{
	virtual ~ITestRenderer() {};

	virtual bool IsInitialize() { return false; };
	virtual bool OnResize(int wndWidth, int wndHeight) { return true; };
	virtual bool LoadMesh(Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem) { return true; };
	virtual bool LoadTexture(const TextureList& textureList) { return true; };
	virtual bool SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize) { return true; };
	virtual bool PrepareFrame() { return true; };
	virtual bool Draw(AllRenderItems& renderItem) { return true; };

	virtual void Set4xMsaaState(HWND hwnd, int widht, int height, bool value) {};
};