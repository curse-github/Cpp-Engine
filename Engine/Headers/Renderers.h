#pragma once
#ifndef _RENDERERS_H
#define _RENDERERS_H

#include "Engine.h"
class OrthoCam;

#include <array>

class Renderer : public Object {
	protected:
	Shader* shader;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	public:
	Renderer() : Object(), shader(nullptr), VAO(0), VBO(0), EBO(0) { initialized=false; }
	Renderer(Shader* _shader);
	virtual ~Renderer();
	void setShader(Shader* _shader);
	virtual void draw()=0;
};
class CubeRenderer : public Renderer, public Transform {
	public:
	static const float cubevertices[100];
	static const int cubeindices[36];
	CubeRenderer() : Renderer(), Transform() { initialized=false; }
	CubeRenderer(Shader* _shader, const Vector3& _position, const Vector3& _scale, const Vector3& _rotAxis, const float& _rotAngle);
	CubeRenderer(Shader* _shader, const Vector3& _position, const Vector3& _scale);
	void draw() override;
};
class Renderer2D : public Renderer, public hasTransform2D {
	public:
	Renderer2D() : Renderer(), hasTransform2D() { initialized=false; };
	Renderer2D(Shader* _shader, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center, const float& _rotAngle=0.0f);
};
class SpriteRenderer : public Renderer2D {
	public:
	static const float quadvertices[20];
	static const int quadindices[6];
	SpriteRenderer() : Renderer2D() { initialized=false; }
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
	TextRenderer() : Renderer2D(), text(""), color(Vector4()), scale(0.0f) { initialized=false; }
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
	LineRenderer() : Renderer2D(), positions {}, width(1.0f), loop(false) { initialized=false; }
	LineRenderer(Shader* _shader, const std::vector<Vector2>& _positions, const float& _width, const bool& _loop, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f);
	void draw() override;
};

#endif// _RENDERERS_H