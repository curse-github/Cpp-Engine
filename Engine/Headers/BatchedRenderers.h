#pragma once
#ifndef _BATCH_RENDERERS_H
#define _BATCH_RENDERERS_H

#include "Engine.h"
#include "Renderers.h"

#include <array>

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
struct BatchedQuadData {
	Vector2 position;
	float zIndex;
	Vector2 scale;
	Vector4 modulate;
	float texIndex;
};
class BatchedSpriteRenderer : protected Renderer2D {
	protected:
	BatchedVertex* quadBuffer;
	BatchedVertex* quadBufferPtr;
	void bufferQuad(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector4& modulate, const float& texIndex);
	void renderBatch();
	public:
	using Renderer::shader;
	using Object::initialized;
	std::vector<BatchedQuadData*> quads;
	unsigned int numQuads=0;
	const unsigned int maxQuadCount=1000;
	int drawCalls=0;
	BatchedSpriteRenderer() : Renderer2D(), quadBuffer(nullptr), quadBufferPtr(nullptr) { initialized=false; };
	BatchedSpriteRenderer(Shader* _shader);
	virtual ~BatchedSpriteRenderer();
	BatchedQuadData* addQuad(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector4& modulate, const float& texIndex);
	void draw() override;
};
class StaticBatchedSpriteRenderer : protected Renderer2D {
	protected:
	BatchedVertex* quadBuffer;
	BatchedVertex* quadBufferPtr;
	void bufferQuad(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector4& modulate, const float& texIndex);
	void renderBatch();
	public:
	using Renderer::shader;
	using Object::initialized;
	std::vector<BatchedQuadData*> quads;
	unsigned int numQuads=0;
	const unsigned int maxQuadCount=1000;
	int drawCalls=0;
	StaticBatchedSpriteRenderer() : Renderer2D(), quadBuffer(nullptr), quadBufferPtr(nullptr) { initialized=false; };
	StaticBatchedSpriteRenderer(Shader* _shader);
	virtual ~StaticBatchedSpriteRenderer();
	BatchedQuadData* addQuad(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector4& modulate, const float& texIndex);
	void bind();
	void draw() override;
};

struct BatchedTextData {
	std::string text;
	Vector4 color;
	Vector2 position;
	float zIndex;
	float scale;
	Vector2 anchor;
};
class BatchedTextRenderer : protected Renderer2D {
	protected:
	std::array<BatchedVertex*, 3> characterBuffers;
	std::array<BatchedVertex*, 3> characterBufferPtrs;
	void bufferCharacter(const int& shaderIndex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector4& color, const float& texIndex);
	void renderBatch(const int& shaderIndex);
	std::array<std::array<Texture*, 32>, 3> textureArrays={ {
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
		} };
	public:
	using Object::initialized;
	std::vector<BatchedTextData*> text;
	std::array<unsigned int, 3> numChars={ 0, 0, 0 };
	const unsigned int maxCharacterCount=1000;
	int drawCalls=0;
	BatchedTextRenderer() : Renderer2D(), characterBuffers({ nullptr, nullptr, nullptr }), characterBufferPtrs({ nullptr, nullptr, nullptr }) { initialized=false; };
	BatchedTextRenderer(Camera* cam);
	virtual ~BatchedTextRenderer();
	BatchedTextData* addText(const std::string& _text, const Vector4& _color, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor);
	void draw() override;
};
#endif// _BATCH_RENDERERS_H