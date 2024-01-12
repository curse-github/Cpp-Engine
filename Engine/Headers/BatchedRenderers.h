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

struct BatchedLineData : public Transform2D {
	public:
	std::vector<Vector2> positions;
	bool loop;
	Vector4 color;
	BatchedLineData(std::vector<Vector2>&& _positions, const bool& _loop, const Vector4& _color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f) :
		Transform2D(_position, _zIndex, Vector2::ONE, Vector2::Center), positions(_positions), loop(_loop), color(_color) {};
};
struct BatchedRectData : public Transform2D {
	public:
	Vector4 color;
	BatchedRectData(const Vector4& _color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f) :
		Transform2D(_position, _zIndex, Vector2::ONE, Vector2::Center), color(_color) {};
};
class BatchedLineRenderer : protected Renderer2D {
	protected:
	std::vector<BatchedLineData*> lines;
	std::vector<BatchedRectData*> rects;
	std::vector<BatchedVertex> verticesBuffer;
	void bufferVertex(const Vector2& vertexPos, const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE);
	void bufferLine(const std::vector<Vector2>& positions, const bool& loop, const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE);
	void bufferRect(const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE);
	void renderBatch();
	public:
	OrthoCam* cam;
	float width;
	using Renderer::shader;
	using Object::initialized;
	const unsigned short int maxVertices=40000;
	unsigned short int numVertices=0;
	BatchedLineRenderer() : Renderer2D(), cam(nullptr), width(1.0f) { initialized=false; };
	BatchedLineRenderer(OrthoCam* _cam, const float& width=1.0f);
	virtual ~BatchedLineRenderer();
	BatchedLineData* addLine(std::vector<Vector2>&& positions, const bool& loop, const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f);
	BatchedRectData* addRect(const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f);
	void draw() override;
};
class StaticBatchedLineRenderer : protected Renderer2D {
	protected:
	std::vector<BatchedLineData*> lines;
	std::vector<BatchedRectData*> rects;
	std::vector<BatchedVertex> verticesBuffer;
	void bufferVertex(const Vector2& vertexPos, const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE);
	void bufferLine(const std::vector<Vector2>& positions, const bool& loop, const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE);
	void bufferRect(const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE);
	void renderBatch();
	public:
	OrthoCam* cam;
	float width;
	using Renderer::shader;
	using Object::initialized;
	const unsigned short int maxVertices=40000;
	unsigned short int numVertices=0;
	StaticBatchedLineRenderer() : Renderer2D(), cam(nullptr), width(1.0f) { initialized=false; };
	StaticBatchedLineRenderer(OrthoCam* _cam, const float& width=1.0f);
	virtual ~StaticBatchedLineRenderer();
	BatchedLineData* addLine(std::vector<Vector2>&& positions, const bool& loop, const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f);
	BatchedRectData* addRect(const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f);
	void bind();
	void draw() override;
};

struct BatchedDotData : public Transform2D {
	public:
	Vector4 modulate;
	Texture* tex;
	float radius;
	BatchedDotData(const Vector4& _modulate, Texture* _tex, const float& _radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center) :
		Transform2D(_position, _zIndex, _scale, _anchor), modulate(_modulate), tex(_tex), radius(_radius) {};
};
class BatchedDotRenderer : protected Renderer2D {
	protected:
	std::vector<BatchedDotData*> dots;
	std::vector<BatchedVertex> dotVerticesBuffer;
	unsigned int numTextures=0;
	std::vector<Texture*> textures;
	void bufferDot(const Vector4& _modulate, Texture* tex, const float& radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void renderBatch();
	public:
	OrthoCam* cam;
	using Renderer::shader;
	using Object::initialized;
	const unsigned short int maxDotCount=10000;
	const unsigned short int maxVertices=maxDotCount*3;
	unsigned short int numDots=0;
	BatchedDotRenderer() : Renderer2D(), cam(nullptr) { initialized=false; };
	BatchedDotRenderer(OrthoCam* _cam);
	virtual ~BatchedDotRenderer();
	BatchedDotData* addTexturedDot(const Vector4& _modulate, Texture* tex, const float& _radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedDotData* addDot(const Vector4& _modulate, const float& radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void draw() override;
};
class StaticBatchedDotRenderer : protected Renderer2D {
	protected:
	std::vector<BatchedDotData*> dots;
	std::vector<BatchedVertex> dotVerticesBuffer;
	unsigned int numTextures=0;
	std::vector<Texture*> textures;
	void bufferDot(const Vector4& _modulate, Texture* tex, const float& radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void renderBatch();
	public:
	OrthoCam* cam;
	using Renderer::shader;
	using Object::initialized;
	const unsigned short int maxDotCount=10000;
	const unsigned short int maxVertices=maxDotCount*3;
	unsigned short int numDots=0;
	StaticBatchedDotRenderer() : Renderer2D(), cam(nullptr) { initialized=false; };
	StaticBatchedDotRenderer(OrthoCam* _cam);
	virtual ~StaticBatchedDotRenderer();
	BatchedDotData* addTexturedDot(const Vector4& _modulate, Texture* tex, const float& _radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedDotData* addDot(const Vector4& _modulate, const float& radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void bind();
	void draw() override;
};
#endif// _BATCH_RENDERERS_H