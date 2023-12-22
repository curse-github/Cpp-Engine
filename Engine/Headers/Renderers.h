#pragma once
#ifndef _RENDERERS_H
#define _RENDERERS_H

#include "Engine.h"
class OrthoCam;

class Renderer : public Object {
	protected:
	Shader* shader;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	public:
	Renderer() : Object(), shader(nullptr), VAO(0), VBO(0), EBO(0) {}
	Renderer(Engine* _engine, Shader* _shader);
	virtual ~Renderer();
	void setShader(Shader* _shader);
	virtual void draw()=0;
};
class CubeRenderer : public Renderer, public Transform {
	public:
	static const float cubevertices[100];
	static const int cubeindices[36];
	CubeRenderer() : Renderer(), Transform() {}
	CubeRenderer(Engine* _engine, Shader* _shader, const Vector3& _position, const Vector3& _scale, const Vector3& _rotAxis, const float& _rotAngle);
	CubeRenderer(Engine* _engine, Shader* _shader, const Vector3& _position, const Vector3& _scale);
	void draw() override;
};
class Renderer2D : public Renderer, public Transform2D {
	protected:
	static bool AABBOverlap(const Vector2& aPos, const Vector2& aSize, const Vector2& bPos, const Vector2& bSize);
	public:
	Renderer2D() : Renderer(), Transform2D() {};
	Renderer2D(Engine* _engine, Shader* _shader, Vector2 _position, float _zIndex, Vector2 _scale, Vector2 _anchor, float _rotAngle);
	bool shouldDraw(const Vector2& viewer, const Vector2& viewRange);
	bool shouldDraw(const Vector2& viewer, const float& viewRange);
	bool shouldDraw(OrthoCam* viewer);
};
class SpriteRenderer : public Renderer2D {
	public:
	static const float quadvertices[20];
	static const int quadindices[6];
	SpriteRenderer() : Renderer2D() {}
	SpriteRenderer(Engine* _engine, Shader* _shader, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor, const float& _rotAngle);
	SpriteRenderer(Engine* _engine, Shader* _shader, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor);
	SpriteRenderer(Engine* _engine, Shader* _shader, const Vector2& _position, const float& _zIndex, const Vector2& _scale);
	SpriteRenderer(Engine* _engine, Shader* _shader, const Vector2& _position, const float& _zIndex);
	SpriteRenderer(Engine* _engine, Shader* _shader, const Vector2& _position);
	void draw() override;
};
extern bool characterMapInitialized;
class TextRenderer : public Renderer2D {
	public:
	std::string text;
	Vector3 color;
	float scale;
	TextRenderer() : Renderer2D(), text(""), color(Vector3()), scale(0.0f) {}
	TextRenderer(Engine* _engine, Shader* _shader, const std::string& _text, const Vector3& _color, const Vector2& _position, const float& _scale, const float& _zIndex, const Vector2& _anchor);
	TextRenderer(Engine* _engine, Shader* _shader, const std::string& _text, const Vector3& _color, const Vector2& _position, const float& _scale, const float& _zIndex);
	TextRenderer(Engine* _engine, Shader* _shader, const std::string& _text, const Vector3& _color, const Vector2& _position, const float& _scale);
	void draw() override;
	using Renderer2D::shouldDraw;
};
class LineRenderer : public Renderer2D {
	protected:
	std::vector<Vector2> positions;
	public:
	float width;
	bool loop;
	LineRenderer() : Renderer2D(), positions {}, width(1.0f), loop(false) {}
	LineRenderer(Engine* _engine, Shader* _shader, const std::vector<Vector2>& _positions, const float& _width, const Vector2& _position, const bool& _loop);
	LineRenderer(Engine* _engine, Shader* _shader, const std::vector<Vector2>& _positions, const float& _width, const Vector2& _position);
	LineRenderer(Engine* _engine, Shader* _shader, const std::vector<Vector2>& _positions, const float& _width, const bool& _loop);
	LineRenderer(Engine* _engine, Shader* _shader, const std::vector<Vector2>& _positions, const float& _width);
	void draw() override;
};

#endif// _RENDERERS_H