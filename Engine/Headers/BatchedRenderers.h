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
struct BatchedQuadData : public Transform2D {
	public:
	Vector4 modulate;
	float texIndex;
	BatchedQuadData(const Vector4& _modulate, const float& _texIndex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center) :
		Transform2D(_position, _zIndex, _scale, _anchor), modulate(_modulate), texIndex(_texIndex) {};
};
class BatchedSpriteRenderer : protected Renderer2D {
	protected:
	BatchedVertex* quadBuffer;
	BatchedVertex* quadBufferPtr;
	void bufferQuad(const Vector4& modulate, const float& texIndex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor);
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
	BatchedQuadData* addQuad(const Vector4& _modulate, const float& _texIndex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void draw() override;
};
class StaticBatchedSpriteRenderer : protected Renderer2D {
	protected:
	BatchedVertex* quadBuffer;
	BatchedVertex* quadBufferPtr;
	void bufferQuad(const Vector4& _modulate, const float& _texIndex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
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
	BatchedQuadData* addQuad(const Vector4& _modulate, const float& _texIndex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void bind();
	void draw() override;
};

struct BatchedTextData : public Transform2D {
	public:
	std::string text;
	Vector4 color;
	float scale;
	BatchedTextData(const std::string& _text, const Vector4& _color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const float& _scale=1.0f, const Vector2& _anchor=Vector2::Center) :
		Transform2D(_position, _zIndex, Vector2(_scale), _anchor), text(_text), color(_color), scale(_scale) {};
};
class BatchedTextRenderer : protected Renderer2D {
	protected:
	std::array<BatchedVertex*, 3> characterBuffers;
	std::array<BatchedVertex*, 3> characterBufferPtrs;
	void bufferCharacter(const int& shaderIndex, const float& texIndex, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale);
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
	BatchedTextData* addText(const std::string& _text, const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const float& _scale=1.0f, const Vector2& _anchor=Vector2::Center);
	void draw() override;
};
#endif// _BATCH_RENDERERS_H