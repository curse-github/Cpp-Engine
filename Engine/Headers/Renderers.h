#pragma once
#ifndef _RENDERERS_H
#define _RENDERERS_H

#include "Engine.h"
class OrthoCam;

#include <array>

class Renderer : public Object {
	protected:
	VertexArrayObject* VAO;
	VertexBufferObject* VBO;
	IndexBufferObject* IBO;
	public:
	Shader* shader;
	Renderer(Shader* _shader);
	virtual void draw()=0;
};
class CubeRenderer : public Renderer, public Transform {
	public:
	static const float cubevertices[100];
	static const unsigned int cubeindices[36];
	CubeRenderer(Shader* _shader, const Vector3& _position, const Vector3& _scale, const Vector3& _rotAxis, const float& _rotAngle);
	CubeRenderer(Shader* _shader, const Vector3& _position, const Vector3& _scale);
	void draw() override;
};
class Renderer2D : public Renderer, public hasTransform2D {
	public:
	Renderer2D(Shader* _shader, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center, const float& _rotAngle=0.0f);
};
class SpriteRenderer : public Renderer2D {
	public:
	static const float quadvertices[20];
	static const unsigned int quadindices[6];
	SpriteRenderer(Shader* _shader, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center, const float& _rotAngle=0.0f);
	void draw() override;
};
extern bool characterMapInitialized;
class TextRenderer : public Renderer2D {
	public:
	struct Character;
	static std::array<Character, 128> Characters;
	static bool characterMapInitialized;
	static int initCharacterMap();
	using Renderer::shader;

	std::string text;
	Vector4 color;
	float scale;
	TextRenderer(Shader* _shader, const std::string& _text, const Vector4& _color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const float& _scale=1.0f, const Vector2& _anchor=Vector2::Center);
	void draw() override;
	virtual Vector2 getWorldScale() const { return Vector2(scale); };// makes text scale independent of parent transform2d
};
class LineRenderer : public Renderer2D {
	protected:
	std::vector<Vector2> positions;
	public:
	float width;
	bool loop;
	LineRenderer(Shader* _shader, const std::vector<Vector2>& _positions, const bool& _loop, const float& _width, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f);
	void draw() override;
};
class DotRenderer : public Renderer2D {
	public:
	static const float vertices[15];
	float radius;
	DotRenderer(Shader* _shader, const float& _radius, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _anchor=Vector2::Center);
	void draw() override;
};

#endif// _RENDERERS_H