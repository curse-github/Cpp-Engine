#pragma once
#ifndef _BATCH_RENDERERS_H
#define _BATCH_RENDERERS_H

#include "Engine.h"

const int maxTextures=32;
struct BatchedVertex {
	float X;
	float Y;
	float Z;

	float U;
	float V;

	float R;
	float G;
	float B;
	float A;

	float I;
};
struct QuadData {
	Vector2 position;
	float zIndex;
	Vector2 scale;
	float texIndex;
	Vector4 modulate;
};
class BatchedSpriteRenderer : protected Renderer2D {
	protected:
	BatchedVertex* dataBuffer;
	BatchedVertex* dataBufferPtr;
	void bufferQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex);
	void renderBatch();
	public:
	std::vector<QuadData> quads;
	const int maxQuadCount=10000;
	int drawCalls=0;
	int numQuads=0;
	BatchedSpriteRenderer() : Renderer2D(), dataBuffer(nullptr), dataBufferPtr(nullptr) {};
	BatchedSpriteRenderer(Engine* _engine, Shader* _shader);
	virtual ~BatchedSpriteRenderer();
	void addQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex);
	void draw() override;
};
class StaticBatchedSpriteRenderer : protected Renderer2D {
	protected:
	BatchedVertex* dataBuffer;
	BatchedVertex* dataBufferPtr;
	void bufferQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex);
	void renderBatch();
	public:
	std::vector<QuadData> quads;
	int maxQuadCount=10000;
	int numQuads=0;
	StaticBatchedSpriteRenderer() : Renderer2D(), dataBuffer(nullptr), dataBufferPtr(nullptr) {};
	StaticBatchedSpriteRenderer(Engine* _engine, Shader* _shader);
	virtual ~StaticBatchedSpriteRenderer();
	void addQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex);
	void bind();
	void draw() override;
};

#endif// _BATCH_RENDERERS_H