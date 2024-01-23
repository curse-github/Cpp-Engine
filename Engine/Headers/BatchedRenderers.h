#pragma once
#ifndef _BATCHED_RENDERERS_H
#define _BATCHED_RENDERERS_H

#include "Engine.h"
#include "Renderers.h"

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
	BatchedVertex(const Vector3& pos, const Vector2& uv, const Vector4& modulate, const float& texIndex) :
		X(pos.x), Y(pos.y), Z(pos.z), U(uv.x), V(uv.y), R(modulate.x), G(modulate.y), B(modulate.z), A(modulate.w), I(texIndex) {};
	BatchedVertex(const Vector2& pos, const float& zIndex, const Vector2& uv, const Vector4& modulate, const float& texIndex) :
		X(pos.x), Y(pos.y), Z(zIndex), U(uv.x), V(uv.y), R(modulate.x), G(modulate.y), B(modulate.z), A(modulate.w), I(texIndex) {};
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
	unsigned int numTextures=0u;
	std::vector<Texture*> textures;
	void bufferQuad(BatchedQuadData* data);
	void renderBatch();
	public:
	using Renderer::shader;
	const unsigned short int maxQuadCount=10000u;
	const unsigned short int maxVertices=maxQuadCount*4u;
	const unsigned short int maxIndices=maxQuadCount*6u;
	unsigned short int numQuads=0u;
	BatchedSpriteRenderer(OrthoCam* cam);
	virtual ~BatchedSpriteRenderer();
	BatchedQuadData* addSprite(const Vector4& modulate, Texture* tex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedQuadData* addQuad(const Vector4& modulate, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void draw() override;
};
class StaticBatchedSpriteRenderer : protected Renderer2D {
	protected:
	std::vector<BatchedQuadData*> quads;
	std::vector<BatchedVertex> quadVerticesBuffer;
	unsigned int numTextures=0;
	std::vector<Texture*> textures;
	void bufferQuad(BatchedQuadData* data);
	void renderBatch();
	public:
	using Renderer::shader;
	const unsigned short int maxQuadCount=10000u;
	const unsigned short int maxVertices=maxQuadCount*4u;
	const unsigned short int maxIndices=maxQuadCount*6u;
	unsigned short int numQuads=0u;
	StaticBatchedSpriteRenderer(OrthoCam* cam);
	virtual ~StaticBatchedSpriteRenderer();
	BatchedQuadData* addSprite(const Vector4& modulate, Texture* tex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedQuadData* addQuad(const Vector4& modulate, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
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
	std::array<std::vector<BatchedVertex>, 3u> characterBuffers;
	std::array<std::array<Texture*, 32u>, 3u> textureArrays={ {
		{ nullptr },
		{ nullptr },
		{ nullptr }
		} };
	void bufferCharacter(const unsigned short int& shaderIndex, const float& texIndex, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale);
	void renderBatch(const unsigned short int& shaderIndex);
	public:
	using Renderer::shader;
	const unsigned short int maxCharacterCount=3333u;
	const unsigned short int maxVertices=maxCharacterCount*4u;
	const unsigned short int maxIndices=maxCharacterCount*6u;
	std::array<unsigned short int, 3u> numChars={ 0u, 0u, 0u };
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
	void bufferLine(BatchedLineData* data);
	void bufferRect(BatchedRectData* data);
	void renderBatch();
	public:
	float width;
	using Renderer::shader;
	const unsigned short int maxVertices=40000u;
	unsigned short int numVertices=0u;
	BatchedLineRenderer(OrthoCam* cam, const float& _width=1.0f);
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
	void bufferLine(BatchedLineData* data);
	void bufferRect(BatchedRectData* data);
	void renderBatch();
	public:
	float width;
	using Renderer::shader;
	const unsigned short int maxVertices=40000u;
	unsigned short int numVertices=0u;
	StaticBatchedLineRenderer(OrthoCam* cam, const float& _width=1.0f);
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
	unsigned int numTextures=0u;
	std::vector<Texture*> textures;
	void bufferDot(BatchedDotData* data);
	void renderBatch();
	public:
	using Renderer::shader;
	const unsigned short int maxDotCount=10000u;
	const unsigned short int maxVertices=maxDotCount*3u;
	unsigned short int numDots=0u;
	BatchedDotRenderer(OrthoCam* cam);
	virtual ~BatchedDotRenderer();
	BatchedDotData* addTexturedDot(const Vector4& modulate, Texture* tex, const float& radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedDotData* addDot(const Vector4& modulate, const float& radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void draw() override;
};
class StaticBatchedDotRenderer : protected Renderer2D {
	protected:
	std::vector<BatchedDotData*> dots;
	std::vector<BatchedVertex> dotVerticesBuffer;
	unsigned int numTextures=0u;
	std::vector<Texture*> textures;
	void bufferDot(BatchedDotData* data);
	void renderBatch();
	public:
	using Renderer::shader;
	const unsigned short int maxDotCount=10000u;
	const unsigned short int maxVertices=maxDotCount*3u;
	unsigned short int numDots=0u;
	StaticBatchedDotRenderer(OrthoCam* cam);
	virtual ~StaticBatchedDotRenderer();
	BatchedDotData* addTexturedDot(const Vector4& modulate, Texture* tex, const float& radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedDotData* addDot(const Vector4& modulate, const float& radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void bind();
	void draw() override;
};

struct BatchedAtlasedSpriteData : public Transform2D {
	public:
	Vector4 modulate;
	Texture* tex;
	Vector2i atlasSize;
	Vector2i texPos;
	Vector2i texSize;
	float texRot;
	BatchedAtlasedSpriteData(const Vector4& _modulate, Texture* _tex, const Vector2i& _atlasSize, const Vector2i& _texPos=Vector2i::ZERO, const Vector2i& _texSize=Vector2i::ONE, const float& _texRot=0.0f, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center) :
		Transform2D(_position, _zIndex, _scale, _anchor), modulate(_modulate), tex(_tex), atlasSize(_atlasSize), texPos(_texPos), texSize(_texSize), texRot(_texRot) {};
};
class BatchedSpritesheetRenderer : protected Renderer2D {
	protected:
	std::vector<BatchedAtlasedSpriteData*> quads;
	std::vector<BatchedVertex> quadVerticesBuffer;
	unsigned int numTextures=0u;
	std::vector<Texture*> textures;
	void bufferQuad(BatchedAtlasedSpriteData* data);
	void renderBatch();
	public:
	using Renderer::shader;
	const unsigned short int maxQuadCount=10000u;
	const unsigned short int maxVertices=maxQuadCount*4u;
	const unsigned short int maxIndices=maxQuadCount*6u;
	unsigned short int numQuads=0u;
	BatchedSpritesheetRenderer(OrthoCam* cam);
	virtual ~BatchedSpritesheetRenderer();
	BatchedAtlasedSpriteData* addSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const Vector2i& texPos=Vector2i::ZERO, const Vector2i& texSize=Vector2i::ONE, const float& texRot=0.0f, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedAtlasedSpriteData* addTruesizeSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const Vector2i& texPos=Vector2i::ZERO, const Vector2i& texSize=Vector2i::ONE, const float& texRot=0.0f, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const float& _scale=1.0f, const Vector2& _anchor=Vector2::Center);
	void draw() override;
};
class StaticBatchedSpritesheetRenderer : protected Renderer2D {
	protected:
	std::vector<BatchedAtlasedSpriteData*> quads;
	std::vector<BatchedVertex> quadVerticesBuffer;
	unsigned int numTextures=0u;
	std::vector<Texture*> textures;
	void bufferQuad(BatchedAtlasedSpriteData* data);
	void renderBatch();
	public:
	using Renderer::shader;
	const unsigned short int maxQuadCount=10000u;
	const unsigned short int maxVertices=maxQuadCount*4u;
	const unsigned short int maxIndices=maxQuadCount*6u;
	unsigned short int numQuads=0u;
	StaticBatchedSpritesheetRenderer(OrthoCam* cam);
	virtual ~StaticBatchedSpritesheetRenderer();
	BatchedAtlasedSpriteData* addSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const Vector2i& texPos=Vector2i::ZERO, const Vector2i& texSize=Vector2i::ONE, const float& texRot=0.0f, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedAtlasedSpriteData* addTruesizeSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const Vector2i& texPos=Vector2i::ZERO, const Vector2i& texSize=Vector2i::ONE, const float& texRot=0.0f, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const float& _scale=1.0f, const Vector2& _anchor=Vector2::Center);
	void bind();
	void draw() override;
};

class BatchedSpritesheetAnimationRenderer;
struct BatchedAtlasedAnimationData : public BatchedAtlasedSpriteData {
	private:
	Vector2 lastUvShift=Vector2i(0, 0);
	double timeSinceLastFrame=0.0;
	unsigned short int frameIndex=0u;
	bool playing=true;
	friend BatchedSpritesheetAnimationRenderer;
	public:
	Vector2i animationDir=Vector2i(1, 0);
	unsigned short int numFrames;
	double frameDelay;
	bool repeat=true;
	BatchedAtlasedAnimationData(const Vector4& _modulate, Texture* _tex, const Vector2i& _atlasSize, const unsigned short int& _numFrames, const double& _frameDelay, const Vector2i& _texPos=Vector2i::ZERO, const Vector2i& _texSize=Vector2i::ONE, const float& _texRot=0.0f, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center) :
		BatchedAtlasedSpriteData(_modulate, _tex, _atlasSize, _texPos, _texSize, _texRot, _position, _zIndex, _scale, _anchor), numFrames(_numFrames), frameDelay(_frameDelay) {};
	void on_loop(const double& delta) {
		if(playing) {
			timeSinceLastFrame+=delta;
			while(timeSinceLastFrame>=frameDelay) {
				timeSinceLastFrame-=frameDelay;
				frameIndex+=1u;
				if(repeat) frameIndex%=numFrames;
				else { playing=false;break; }
			}
		} else { timeSinceLastFrame=0.0;frameIndex=0u; }
	}
};
class BatchedSpritesheetAnimationRenderer : protected Renderer2D {
	protected:
	std::vector<BatchedAtlasedAnimationData*> quads;
	std::vector<BatchedVertex> quadVerticesBuffer;
	unsigned int numTextures=0u;
	std::vector<Texture*> textures;
	void bufferQuad(BatchedAtlasedAnimationData* data);
	void renderBatch();
	public:
	using Renderer::shader;
	const unsigned short int maxQuadCount=10000u;
	const unsigned short int maxVertices=maxQuadCount*4u;
	const unsigned short int maxIndices=maxQuadCount*6u;
	unsigned short int numQuads=0u;
	BatchedSpritesheetAnimationRenderer(OrthoCam* cam);
	virtual ~BatchedSpritesheetAnimationRenderer();
	BatchedAtlasedAnimationData* addSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const unsigned short int& numFrames, const double& frameDelay, const Vector2i& texPos=Vector2i::ZERO, const Vector2i& texSize=Vector2i::ONE, const float& texRot=0.0f, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedAtlasedAnimationData* addTruesizeSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const unsigned short int& numFrames, const double& frameDelay, const Vector2i& texPos=Vector2i::ZERO, const Vector2i& texSize=Vector2i::ONE, const float& texRot=0.0f, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const float& _scale=1.0f, const Vector2& _anchor=Vector2::Center);
	void draw() override;
	void on_loop(const double& delta) override;
};
#endif// _BATCHED_RENDERERS_H