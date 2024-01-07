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
	Texture* tex;
	BatchedQuadData(const Vector4& _modulate, Texture* _tex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center) :
		Transform2D(_position, _zIndex, _scale, _anchor), modulate(_modulate), tex(_tex) {};
};
class BatchedSpriteRenderer : protected Renderer2D {
	protected:
	std::vector<BatchedQuadData*> quads;
	std::vector<BatchedVertex> quadVerticesBuffer;
	unsigned int numTextures=0;
	std::vector<Texture*> textures;
	void bufferQuad(const Vector4& _modulate, Texture* tex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void renderBatch();
	public:
	OrthoCam* cam;
	using Renderer::shader;
	using Object::initialized;
	const unsigned short int maxQuadCount=10000;
	const unsigned short int maxVertices=maxQuadCount*4;
	const unsigned short int maxIndices=maxQuadCount*6;
	unsigned short int numQuads=0;
	BatchedSpriteRenderer() : Renderer2D(), cam(nullptr) { initialized=false; };
	BatchedSpriteRenderer(OrthoCam* _cam);
	virtual ~BatchedSpriteRenderer();
	BatchedQuadData* addSprite(const Vector4& _modulate, Texture* tex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedQuadData* addQuad(const Vector4& _modulate, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void draw() override;
};
class StaticBatchedSpriteRenderer : protected Renderer2D {
	protected:
	std::vector<BatchedQuadData*> quads;
	std::vector<BatchedVertex> quadVerticesBuffer;
	unsigned int numTextures=0;
	std::vector<Texture*> textures;
	void bufferQuad(const Vector4& _modulate, Texture* tex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void renderBatch();
	public:
	OrthoCam* cam;
	using Renderer::shader;
	using Object::initialized;
	const unsigned short int maxQuadCount=10000;
	const unsigned short int maxVertices=maxQuadCount*4;
	const unsigned short int maxIndices=maxQuadCount*6;
	unsigned short int numQuads=0;
	StaticBatchedSpriteRenderer() : Renderer2D(), cam(nullptr) { initialized=false; };
	StaticBatchedSpriteRenderer(OrthoCam* _cam);
	virtual ~StaticBatchedSpriteRenderer();
	BatchedQuadData* addSprite(const Vector4& _modulate, Texture* tex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedQuadData* addQuad(const Vector4& _modulate, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
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
	std::vector<BatchedTextData*> text;
	std::array<std::vector<BatchedVertex>, 3> characterBuffers;
	std::array<std::array<Texture*, 32>, 3> textureArrays={ {
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
		} };
	void bufferCharacter(const int& shaderIndex, const float& texIndex, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale);
	void renderBatch(const int& shaderIndex);
	public:
	using Renderer::shader;
	using Object::initialized;
	const unsigned short int maxCharacterCount=3333;
	const unsigned short int maxVertices=maxCharacterCount*4;
	const unsigned short int maxIndices=maxCharacterCount*6;
	std::array<unsigned short int, 3> numChars={ 0, 0, 0 };
	BatchedTextRenderer() : Renderer2D() { initialized=false; };
	BatchedTextRenderer(Camera* cam);
	virtual ~BatchedTextRenderer();
	BatchedTextData* addText(const std::string& _text, const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const float& _scale=1.0f, const Vector2& _anchor=Vector2::Center);
	void draw() override;
};
#endif// _BATCH_RENDERERS_H