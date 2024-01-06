#include "Renderers.h"

#include <freetype/freetype.h>

#pragma region Renderer
Renderer::Renderer(Shader* _shader) : Object(), shader(_shader), VAO(0), VBO(0), EBO(0) {
	if(!initialized||(shader==nullptr)||!shader->initialized) initialized=false;
}
Renderer::~Renderer() {
	if(!initialized) return;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
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
const int CubeRenderer::cubeindices[36]={
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4,
	8, 9, 10, 10, 11, 8,
	12, 13, 14, 14, 15, 12,
	8, 13, 16, 16, 17, 8,
	18, 19, 14, 14, 11, 18
};
CubeRenderer::CubeRenderer(Shader* _shader, const Vector3& _position, const Vector3& _scale, const Vector3& _rotAxis, const float& _rotAngle) :
	Renderer(_shader), Transform(_position, _scale, _rotAxis, _rotAngle) {
	if(!initialized) return;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubevertices), cubevertices, GL_STATIC_DRAW);// fill VBO buffer with vertex data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);// bind buffer so that following code will assign the EBO buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeindices), cubeindices, GL_STATIC_DRAW);// fill EBO buffer with index data
	// start, length, type, ?, total size, offset
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
}
CubeRenderer::CubeRenderer(Shader* _shader, const Vector3& _position, const Vector3& _scale) : CubeRenderer(_shader, _position, _scale, Vector3::UP, 0.0f) {}
void CubeRenderer::draw() {
	if(Engine::instance->ended||!initialized) return;
	Mat4x4 model=axisRotMat(rotAxis, deg_to_rad(rotAngle))*translate(position);
	shader->bindTextures();
	shader->setMat4x4("model", model);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	Engine::instance->curDrawCalls++;
}
#pragma endregion// CubeRenderer
#pragma region Renderer2D
Renderer2D::Renderer2D(Shader* _shader, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor, const float& _rotAngle) :
	Renderer(_shader), hasTransform2D(_position, _zIndex, _scale, _anchor, _rotAngle) {
	if(!initialized) return;
};
#pragma endregion// Renderers2D
#pragma region SpriteRenderer
const float SpriteRenderer::quadvertices[20] {
	-0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
	0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
	0.5f, -0.5f, 0.0f, 1.0f, 1.0f
};
const int SpriteRenderer::quadindices[6] {
	1, 3, 2,
	2, 0, 1
};
SpriteRenderer::SpriteRenderer(Shader* _shader, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor, const float& _rotAngle) :
	Renderer2D(_shader, _position, _zIndex, _scale, _anchor, _rotAngle) {
	if(!initialized) return;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadvertices), quadvertices, GL_STATIC_DRAW);// fill VBO buffer with vertex data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void SpriteRenderer::draw() {
	if(Engine::instance->ended||!initialized) return;
	shader->bindTextures();
	shader->setMat4x4("model", getModelMat());
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	Engine::instance->curDrawCalls++;
}
#pragma endregion// SpriteRenderer
#pragma region TextRenderer
struct TextRenderer::Character {
	Texture* tex=new Texture();// ID handle of the glyph texture
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
	if(FT_New_Face(ft, "Fonts/MonocraftBetterBrackets.ttf", 0, &face)) return 0;
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
	if(!initialized) return;
	if(!characterMapInitialized) {
		if(!initCharacterMap()) {
			initialized=false;
			Log("Error initializing font \"Fonts/MonocraftBetterBrackets.ttf\"");//error
			Engine::instance->Delete();
			return;
		}
	}
	//setup buffers
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(SpriteRenderer::quadvertices), &SpriteRenderer::quadvertices[0], GL_STATIC_DRAW);// fill VBO buffer with vertex data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	shader->setFloat("text", 0);
}
void TextRenderer::draw() {
	Log("Draw");
	if(Engine::instance->ended||!initialized) return;
	shader->setFloat4("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);
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
		Log(std::string(1, charC)+": "+pos.to_string());
		shader->setMat4x4("model", scaleMat(Vector3(ch.Size*scale, 1.0f))*translate(Vector3(pos, getZIndex()-100.0f)));
		ch.tex->Bind(0);
		// render quad
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		Engine::instance->curDrawCalls++;
		// now advance cursors for next glyph
		x+=1.0f+ch.Advance;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
#pragma endregion// TextRenderer
#pragma region LineRenderer
LineRenderer::LineRenderer(Shader* _shader, const std::vector<Vector2>& _positions, const float& _width, const bool& _loop, const Vector2& _position, const float& _zIndex) :
	Renderer2D(_shader, _position, _zIndex), positions(_positions), width(_width), loop(_loop) {
	if(!initialized) return;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), &verts[0], GL_STATIC_DRAW);// fill VBO buffer with vertex data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void LineRenderer::draw() {
	if(Engine::instance->ended||!initialized) return;
	shader->bindTextures();
	shader->setMat4x4("model", scaleMat(Vector3((transform.parent!=nullptr ? transform.parent->getWorldScale() : Vector2(1.0f, 1.0f)), 0.0f))*translate(Vector3(getWorldPos(), 100.0f-getZIndex())));
	glBindVertexArray(VAO);
	glLineWidth(width);
	glDrawArrays(loop ? GL_LINE_LOOP : GL_LINE_STRIP, 0, (GLsizei)positions.size());
	Engine::instance->curDrawCalls++;
}
#pragma endregion// LineRenderer
#pragma region DotRenderer
const float DotRenderer::vertices[15] {
	cosf(90.0f/180.0f*PI), sinf(90.0f/180.0f*PI), 0.0f, 0.5f+cosf(90.0f/180.0f*PI), 0.5f+sinf(90.0f/180.0f*PI),
	cosf(210.0f/180.0f*PI), sinf(210.0f/180.0f*PI), 0.0f, 0.5f+cosf(210.0f/180.0f*PI), 0.5f+sinf(210.0f/180.0f*PI),
	cosf(330.0f/180.0f*PI), sinf(330.0f/180.0f*PI), 0.0f, 0.5f+cosf(330.0f/180.0f*PI), 0.5f+sinf(330.0f/180.0f*PI),
};
DotRenderer::DotRenderer(Shader* _shader, const float& _radius, const Vector2& _position, const float& _zIndex, const Vector2& _anchor) :
	Renderer2D(_shader, _position, _zIndex, Vector2(_radius), _anchor, 0.0f), radius(_radius) {
	if(!initialized) return;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);// fill VBO buffer with vertex data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void DotRenderer::draw() {
	if(Engine::instance->ended||!initialized) return;
	shader->bindTextures();
	shader->setMat4x4("model", getModelMat());
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	Engine::instance->curDrawCalls++;
}
#pragma endregion// DotRenderer