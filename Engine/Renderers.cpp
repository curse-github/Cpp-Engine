#include "Renderers.h"

#include <ft2build.h>
#include <freetype/freetype.h>

#pragma region Renderer
Renderer::Renderer(Shader* _shader) : Object(), shader(_shader), VAO(nullptr), VBO(nullptr), IBO(nullptr) {
	engine_assert(shader!=nullptr, "[Renderer]: shader is nullptr");
	VAO=new VertexArrayObject(true);
	VBO=new VertexBufferObject(VAO);
	IBO=new IndexBufferObject(VAO);
}
#pragma endregion// Renderer
#pragma region CubeRenderer
const float CubeRenderer::cubevertices[100]={
	0.5f, -0.5f, -0.5f, 0.0f, 0.0f,//0
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f,//1
	-0.5f, 0.5f, -0.5f, 1.0f, 1.0f,//2
	0.5f, 0.5f, -0.5f, 0.0f, 1.0f,//3
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,//4
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f,//5
	0.5f, 0.5f, 0.5f, 1.0f, 1.0f,//6
	-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,//7
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,//8
	-0.5f, -0.5f, 0.5f, 1.0f, 0.0f,//9
	-0.5f, 0.5f, 0.5f, 1.0f, 1.0f,//10
	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,//11
	0.5f, -0.5f, 0.5f, 0.0f, 0.0f,//12
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f,//13
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f,//14
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f,//15
	0.5f, -0.5f, 0.5f, 1.0f, 1.0f,//16
	-0.5f, -0.5f, 0.5f, 0.0f, 1.0f,//17
	-0.5f, 0.5f, 0.5f, 0.0f, 0.0f,//18
	0.5f, 0.5f, 0.5f, 1.0f, 0.0f,//19
};
const unsigned int CubeRenderer::cubeindices[36]={
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4,
	8, 9, 10, 10, 11, 8,
	12, 13, 14, 14, 15, 12,
	8, 13, 16, 16, 17, 8,
	18, 19, 14, 14, 11, 18
};
CubeRenderer::CubeRenderer(Shader* _shader, const Vector3& _position, const Vector3& _scale, const Vector3& _rotAxis, const float& _rotAngle) :
	Renderer(_shader), Transform(_position, _scale, _rotAxis, _rotAngle) {
	VBO->staticFill(cubevertices, 100);
	IBO->staticFill(cubeindices, 36);
	VBO->applyAttributes({ 3, 2 });
}
CubeRenderer::CubeRenderer(Shader* _shader, const Vector3& _position, const Vector3& _scale) : CubeRenderer(_shader, _position, _scale, Vector3::UP, 0.0f) {}
void CubeRenderer::draw() {
	Mat4x4 model=axisRotMat(rotAxis, deg_to_rad(rotAngle))*translate(position);
	shader->bindTextures();
	shader->setMat4x4("model", model);
	VAO->drawTrisIndexed(36);
}
#pragma endregion// CubeRenderer
#pragma region Renderer2D
Renderer2D::Renderer2D(Shader* _shader, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor, const float& _rotAngle) :
	Renderer(_shader), hasTransform2D(_position, _zIndex, _scale, _anchor, _rotAngle) {};
#pragma endregion// Renderers2D
#pragma region SpriteRenderer
const float SpriteRenderer::quadvertices[20] {
	-0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
	0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
	0.5f, -0.5f, 0.0f, 1.0f, 1.0f
};
const unsigned int SpriteRenderer::quadindices[6] {
	1, 3, 2,
	2, 0, 1
};
SpriteRenderer::SpriteRenderer(Shader* _shader, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor, const float& _rotAngle) :
	Renderer2D(_shader, _position, _zIndex, _scale, _anchor, _rotAngle) {
	VBO->staticFill(quadvertices, 20);
	VBO->applyAttributes({ 3, 2 });
}
void SpriteRenderer::draw() {
	if (!this->isActive()) return;
	shader->bindTextures();
	shader->setMat4x4("model", getModelMat());
	VAO->drawTriStrip(4);
}
#pragma endregion// SpriteRenderer
#pragma region TextRenderer
struct TextRenderer::Character {
	Texture* tex=nullptr;// ID handle of the glyph texture
	Vector2   Size;		       // Size of glyph
	Vector2   Bearing;         // Offset from baseline to left/top of glyph
	float Advance=0.0f;        // Offset to advance to next glyph
};
std::array<TextRenderer::Character, 128> TextRenderer::Characters;
bool TextRenderer::characterMapInitialized=false;
int TextRenderer::initCharacterMap() {
	FT_Library ft;
	if(FT_Init_FreeType(&ft)) return 0;
	FT_Face face;
	if(FT_New_Face(ft, "Resources/MonocraftBetterBrackets.ttf", 0, &face)) return 0;
	FT_Set_Pixel_Sizes(face, 0, 8);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
	for(unsigned char c=0; c<128; c++) {// characters 0 through 127
		// load character glyph
		if(FT_Load_Char(face, c, FT_LOAD_RENDER)) return 0;
		// generate texture
		unsigned int texID;
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexImage2D(
			GL_TEXTURE_2D,
			0, GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0, GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// now store character for later use
		Vector2 Size(static_cast<float>(face->glyph->bitmap.width), static_cast<float>(face->glyph->bitmap.rows));
		Vector2 Bearing(static_cast<float>(face->glyph->bitmap_left), static_cast<float>(face->glyph->bitmap_top));

		Characters[static_cast<unsigned int>(c)]=Character {
			new Texture(texID),
			Size,
			Bearing,
			static_cast<float>(static_cast<int>(face->glyph->advance.x)>>6)// bitshift by 6 to get value in pixels (2^6 = 64)
		};
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	characterMapInitialized=true;
	return 1;
}
TextRenderer::TextRenderer(Shader* _shader, const std::string& _text, const Vector4& _color, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor) :
	Renderer2D(_shader, _position, _zIndex, Vector2::ONE, _anchor, 0.0f), text(_text), color(_color), scale(_scale) {
	if(!TextRenderer::characterMapInitialized) {
		engine_assert(TextRenderer::initCharacterMap(), "[TextRenderer]: Error initializing font \"Resources/MonocraftBetterBrackets.ttf\"");
	}
	VBO->staticFill(SpriteRenderer::quadvertices, 20);
	VBO->applyAttributes({ 3, 2 });
	shader->setFloat("text", 0);
}
void TextRenderer::draw() {
	if (!this->isActive()) return;
	shader->setFloat4("textColor", color);
	// iterate through all characters to find full text block
	Vector2 tmpScale=Vector2(0.0f, 9.0f);
	float curX=0.0f;
	const char* cstr=text.c_str();
	int len=static_cast<int>(text.length());
	for(int c=0;c!=len;c++) {
		char charC=cstr[c];
		if(charC=='\n') {
			if(curX>tmpScale.x)tmpScale.x=curX;
			curX=0.0f; tmpScale.y+=9.0f;
			continue;
		}
		TextRenderer::Character ch=TextRenderer::Characters[static_cast<int>(charC)];
		if(charC==' ') { curX+=1.0f+ch.Advance; continue; }// skip one space and continue
		else if(charC=='\t') { curX+=1.0f+ch.Advance*4.0f; continue; }//4 character spaces
		curX+=1.0f+ch.Advance;
	}
	if(curX>tmpScale.x)tmpScale.x=curX;
	Vector2 anchor=getAnchor();
	Vector2 offset=getWorldPos()-Vector2(tmpScale.x*(anchor.x+0.5f), tmpScale.y*(anchor.y-0.5f))*scale;
	// iterate through all characters and render each
	float x=0.0f;
	float y=0.0f;
	for(int c=0;c!=len;c++) {
		char charC=cstr[c];
		int intC=static_cast<int>(charC);
		if(charC=='\n') { x=0.0f;y-=9.0f; continue; } else if(charC=='\r') { x=0.0f; continue; }
		TextRenderer::Character ch=TextRenderer::Characters[intC];
		if(charC==' ') { x+=1.0f+ch.Advance; continue; }// skip one space and continue
		else if(charC=='\t') { x+=1.0f+ch.Advance*4.0f; continue; }//4 character spaces
		Vector2 pos=offset+(Vector2(x, y)+ch.Bearing-Vector2(anchor.x*ch.Size.x, (ch.Size.y)/2.0f+9.0f))*scale;
		shader->setMat4x4("model", scaleMat(Vector3(ch.Size*scale, 1.0f))*translate(Vector3(pos, getZIndex()-100.0f)));
		ch.tex->Bind(0);
		// render quad
		VAO->drawTriStrip(4);
		// now advance cursors for next glyph
		x+=1.0f+ch.Advance;
	}
}
#pragma endregion// TextRenderer
#pragma region LineRenderer
LineRenderer::LineRenderer(Shader* _shader, const std::vector<Vector2>& _positions, const bool& _loop, const float& _width, const Vector2& _position, const float& _zIndex) :
	Renderer2D(_shader, _position, _zIndex), positions(_positions), width(_width), loop(_loop) {
	float farthestX=0.0f;
	float farthestY=0.0f;
	std::vector<float> verts;
	for(Vector2 pos : positions) {
		Vector2 absPos=pos.abs();
		if(absPos.x>farthestX) farthestX=absPos.x;
		if(absPos.y>farthestY) farthestY=absPos.y;
		verts.push_back(pos.x);
		verts.push_back(pos.y);
		verts.push_back(0.0f);
		verts.push_back(0.0f);
		verts.push_back(0.0f);
	}
	transform.scale=Vector2(farthestX*2.0f, farthestY*2.0f);
	VBO->staticFill(&verts[0], verts.size());
	VBO->applyAttributes({ 3, 2 });
}
void LineRenderer::draw() {
	if (!this->isActive()) return;
	shader->bindTextures();
	shader->setMat4x4("model", scaleMat(Vector3((transform.parent!=nullptr ? transform.parent->getWorldScale() : Vector2(1.0f, 1.0f)), 0.0f))*translate(Vector3(getWorldPos(), 100.0f-getZIndex())));
	VAO->drawLine(static_cast<unsigned int>(positions.size()), width, loop, true);
}
#pragma endregion// LineRenderer
#pragma region DotRenderer
const float DotRenderer::vertices[15] {
	2.0f*cosf(90.0f/180.0f*PI), 2.0f*sinf(90.0f/180.0f*PI), 0.0f, 0.5f+cosf(90.0f/180.0f*PI), 0.5f+sinf(90.0f/180.0f*PI),
	2.0f*cosf(210.0f/180.0f*PI), 2.0f*sinf(210.0f/180.0f*PI), 0.0f, 0.5f+cosf(210.0f/180.0f*PI), 0.5f+sinf(210.0f/180.0f*PI),
	2.0f*cosf(330.0f/180.0f*PI), 2.0f*sinf(330.0f/180.0f*PI), 0.0f, 0.5f+cosf(330.0f/180.0f*PI), 0.5f+sinf(330.0f/180.0f*PI),
};
DotRenderer::DotRenderer(Shader* _shader, const float& _radius, const Vector2& _position, const float& _zIndex, const Vector2& _anchor) :
	Renderer2D(_shader, _position, _zIndex, Vector2(_radius), _anchor, 0.0f), radius(_radius) {
	VBO->staticFill(vertices, 15);
	VBO->applyAttributes({ 3, 2 });
}
void DotRenderer::draw() {
	if (!this->isActive()) return;
	shader->bindTextures();
	shader->setMat4x4("model", getModelMat());
	VAO->drawTris(3);
}
#pragma endregion// DotRenderer
#pragma region SpritesheetRenderer
SpritesheetRenderer::SpritesheetRenderer(Shader* _shader, const Vector2i& _atlasSize, const Vector2i& _texPos, const Vector2i& _texSize, const float& _texRot, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor, const float& _rotAngle) :
	Renderer2D(_shader, _position, _zIndex, _scale, _anchor, _rotAngle), atlasSize(_atlasSize), texPos(_texPos), texSize(_texSize), texRot(_texRot) {
	VBO->staticFill(SpriteRenderer::quadvertices, 20);
	VBO->applyAttributes({ 3, 2 });
	update();
}
void SpritesheetRenderer::update() {
	shader->setFloat2("uvScale", Vector2(texSize.x/(float)atlasSize.x, texSize.y/(float)atlasSize.y));
	shader->setFloat2("uvShift", Vector2(texPos.x/(float)atlasSize.x, texPos.y/(float)atlasSize.y));
	shader->setFloat("uvRot", deg_to_rad(texRot));
}
void SpritesheetRenderer::draw() {
	if (!this->isActive()) return;
	shader->bindTextures();
	shader->setMat4x4("model", getModelMat());
	VAO->drawTriStrip(4);
}
#pragma endregion// SpritesheetRenderer
#pragma region SpritesheetAnimationRenderer
SpritesheetAnimationRenderer::SpritesheetAnimationRenderer(Shader* _shader, const Vector2i& _atlasSize, const unsigned short int& _numFrames, const double& _frameDelay, const Vector2i& _texPos, const Vector2i& _texSize, const float& _texRot, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor, const float& _rotAngle) :
	SpritesheetRenderer(_shader, _atlasSize, _texPos, _texSize, _texRot, _position, _zIndex, _scale, _anchor, _rotAngle), numFrames(_numFrames), frameDelay(_frameDelay) {
	update();
	Engine::instance->sub_loop(this);
}
void SpritesheetAnimationRenderer::update() {
	shader->setFloat2("uvScale", Vector2(texSize.x/(float)atlasSize.x, texSize.y/(float)atlasSize.y));
	lastUvShift=Vector2(texPos.x/(float)atlasSize.x, texPos.y/(float)atlasSize.y);
	Vector2 frameShift=frameIndex*Vector2(animationDir.x/(float)atlasSize.x, animationDir.y/(float)atlasSize.y);
	shader->setFloat2("uvShift", lastUvShift+(Vector2)frameShift);
	shader->setFloat("uvRot", deg_to_rad(texRot));
}
void SpritesheetAnimationRenderer::on_loop(const double& delta) {
	if(playing) {
		timeSinceLastFrame+=delta;
		while(timeSinceLastFrame>=frameDelay) {
			timeSinceLastFrame-=frameDelay;
			frameIndex+=1;
			if(repeat) frameIndex%=numFrames;
			else { playing=false;break; }
		}
		Vector2 frameShift=frameIndex*Vector2(animationDir.x/(float)atlasSize.x, animationDir.y/(float)atlasSize.y);
		shader->setFloat2("uvShift", lastUvShift+(Vector2)frameShift);
	} else { timeSinceLastFrame=0.0;frameIndex=0; }
}
#pragma endregion// SpritesheetAnimationRenderer
