#include "Renderers.h"

#include <freetype/freetype.h>
#include <map>

#pragma region Renderer
Renderer::Renderer(Engine* _engine, Shader* _shader) : Object(_engine), shader(_shader), VAO(0), VBO(0), EBO(0) {
	if(!initialized||!shader->initialized) initialized=false;
}
Renderer::~Renderer() {
	if(!initialized) return;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
void Renderer::setShader(Shader* _shader) {
	if(engine->ended||!initialized) return;
	shader=_shader;
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
CubeRenderer::CubeRenderer(Engine* _engine, Shader* _shader, const Vector3& _position, const Vector3& _scale, const Vector3& _rotAxis, const float& _rotAngle) :
	Renderer(_engine, _shader), Transform(_position, _scale, _rotAxis, _rotAngle) {
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
CubeRenderer::CubeRenderer(Engine* _engine, Shader* _shader, const Vector3& _position, const Vector3& _scale) {}
void CubeRenderer::draw() {
	if(engine->ended||!initialized) return;
	Mat4x4 model=axisRotMat(rotAxis, deg_to_rad(rotAngle))*translate(position);
	shader->bindTextures();
	shader->setMat4x4("model", model);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}
#pragma endregion// CubeRenderer
#pragma region Renderer2D
bool Renderer2D::AABBOverlap(const Vector2& aPos, const Vector2& aSize, const Vector2& bPos, const Vector2& bSize) {
	if(aPos==bPos) return true;// guaranteed collition
	// collision x-axis?
	float collisionX1=((aPos.x+aSize.x/2)-(bPos.x-bSize.x/2));
	float collisionX2=((bPos.x+bSize.x/2)-(aPos.x-aSize.x/2));
	// collision y-axis?
	float collisionY1=((aPos.y+aSize.y/2)-(bPos.y-bSize.y/2));
	float collisionY2=((bPos.y+bSize.y/2)-(aPos.y-aSize.y/2));
	// collision only if on both axes
	return collisionX1>0&&collisionX2>0&&collisionY1>0&&collisionY2>0;
}
Renderer2D::Renderer2D(Engine* _engine, Shader* _shader, Vector2 _position, float _zIndex, Vector2 _scale, Vector2 _anchor, float _rotAngle) :
	Renderer(_engine, _shader), Transform2D(_position, _zIndex, _scale, _anchor, _rotAngle) {
	if(!initialized) return;
};
bool Renderer2D::shouldDraw(const Vector2& viewer, const Vector2& viewRange) {
	if(engine->ended||!initialized) return false;
	return AABBOverlap(position-Vector2(anchor.x*scale.x, anchor.y*scale.y), scale, viewer, viewRange);
}
bool Renderer2D::shouldDraw(const Vector2& viewer, const float& viewRange) {
	return shouldDraw(viewer, Vector2(viewRange, viewRange));
}
bool Renderer2D::shouldDraw(OrthoCam* viewer) {
	return shouldDraw(viewer->position, viewer->scale);
}
#pragma endregion// Renderers2D
#pragma region SpriteRenderer
const float SpriteRenderer::quadvertices[20] {
	-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -1.0f, 0.0f, 1.0f,
	0.5f, 0.5f, -1.0f, 1.0f, 0.0f,
	0.5f, -0.5f, -1.0f, 1.0f, 1.0f
};
const int SpriteRenderer::quadindices[6] {
	1, 3, 2, 2, 0, 1
};
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor, const float& _rotAngle) :
	Renderer2D(_engine, _shader, _position, _zIndex, _scale, _anchor, _rotAngle) {
	if(!initialized) return;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadvertices), quadvertices, GL_STATIC_DRAW);// fill VBO buffer with vertex data
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);// bind buffer so that following code will assign the EBO buffer
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadindices), quadindices, GL_STATIC_DRAW);// fill EBO buffer with index data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) : SpriteRenderer(_engine, _shader, _position, _zIndex, _scale, _anchor, 0.0f) {}
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, const Vector2& _position, const float& _zIndex, const Vector2& _scale) : SpriteRenderer(_engine, _shader, _position, _zIndex, _scale, Vector2::Center, 0.0f) {}
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, const Vector2& _position, const float& _zIndex) : SpriteRenderer(_engine, _shader, _position, _zIndex, Vector2::ONE, Vector2::Center, 0.0f) {}
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, const Vector2& _position) : SpriteRenderer(_engine, _shader, _position, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {}
void SpriteRenderer::draw() {
	if(engine->ended||!initialized) return;
	shader->bindTextures();
	shader->setMat4x4("model", getModelMat());
	glBindVertexArray(VAO);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
#pragma endregion// SpriteRenderer
#pragma region TextRenderer
struct Character {
	unsigned int TextureID=0;// ID handle of the glyph texture
	Vector2   Size;		     // Size of glyph
	Vector2   Bearing;       // Offset from baseline to left/top of glyph
	int Advance=0;  // Offset to advance to next glyph
};
std::map<char, Character> Characters;
bool characterMapInitialized=false;
int initCharacterMap() {
	FT_Library ft;
	if(FT_Init_FreeType(&ft)) return 0;
	FT_Face face;
	if(FT_New_Face(ft, "Fonts/MonocraftBetterBrackets.ttf", 0, &face)) return 0;
	FT_Set_Pixel_Sizes(face, 0, 8);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
	for(unsigned char c=0; c<128; c++) {
		// load character glyph
		if(FT_Load_Char(face, c, FT_LOAD_RENDER)) return 0;
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
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
		Vector2 Size((float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows);
		Vector2 Bearing((float)face->glyph->bitmap_left, (float)face->glyph->bitmap_top);
		Character character={
			texture,
			Size,
			Bearing,
			((int)face->glyph->advance.x)>>6// bitshift by 6 to get value in pixels (2^6 = 64)
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	characterMapInitialized=true;
	return 1;
}
TextRenderer::TextRenderer(Engine* _engine, Shader* _shader, const std::string& _text, const Vector3& _color, const Vector2& _position, const float& _scale, const float& _zIndex, const Vector2& _anchor) :
	Renderer2D(_engine, _shader, _position, _zIndex, Vector2::ONE, _anchor, 0.0f), text(_text), color(_color), scale(_scale) {
	if(!initialized) return;
	if(!characterMapInitialized) {
		if(!initCharacterMap()) {
			initialized=false;
			Log("Error initializing font \"Fonts/MonocraftBetterBrackets.ttf\"");//error
			engine->Delete();
			return;
		}
	}
	//setup buffers
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(SpriteRenderer::quadvertices), &SpriteRenderer::quadvertices[0], GL_STATIC_DRAW);// fill VBO buffer with vertex data
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);// bind buffer so that following code will assign the EBO buffer
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadindices), quadindices, GL_STATIC_DRAW);// fill EBO buffer with index data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	shader->setFloat("text", 0);
}
TextRenderer::TextRenderer(Engine* _engine, Shader* _shader, const std::string& _text, const Vector3& _color, const Vector2& _position, const float& _scale, const float& _zIndex) : TextRenderer(_engine, _shader, _text, _color, _position, _scale, _zIndex, Vector2::Center) {}
TextRenderer::TextRenderer(Engine* _engine, Shader* _shader, const std::string& _text, const Vector3& _color, const Vector2& _position, const float& _scale) : TextRenderer(_engine, _shader, _text, _color, _position, _scale, 0.0f, Vector2::Center) {}
void TextRenderer::draw() {
	if(engine->ended||!initialized) return;
	shader->setFloat3("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);
	// iterate through all characters to find final scale
	int x=0;
	int maxX=0;
	int y=0;
	std::string::const_iterator c2;
	for(c2=text.begin(); c2!=text.end(); c2++) {
		if(*c2=='\n') {
			if(x>maxX)maxX=x;
			x=0;
			y-=9;
			continue;
		}
		Character ch=Characters[*c2];
		if(*c2==' ') { x+=1+ch.Advance; continue; }// skip one space and continue
		else if(*c2=='\t') { x+=1+ch.Advance*4; continue; }//4 character spaces
		x+=ch.Advance; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	if(x>maxX)maxX=x;
	Transform2D::scale=Vector2((float)maxX, (float)y)*scale;
	Mat4x4 casheMat=translate(Vector3(-anchor, 0.0f))*scaleMat(Vector3(scale, scale, 1.0f));
	Vector2 offset=position+Vector2((Transform2D::scale.x*(-anchor.x-0.5f)), (Transform2D::scale.y*(anchor.y-0.5f)));
	// iterate through all characters and render each
	x=0;
	y=0;
	std::string::const_iterator c;
	for(c=text.begin(); c!=text.end(); c++) {
		if(*c=='\n') { x=0;y-=9; continue; } else if(*c=='\r') { x=0; continue; }
		Character ch=Characters[*c];
		if(*c==' ') { x+=1+ch.Advance; continue; }// skip one space and continue
		else if(*c=='\t') { x+=1+ch.Advance*4; continue; }//4 character spaces
		Mat4x4 model=casheMat*scaleMat(Vector3(ch.Size, 1.0f))*translate(Vector3(offset+Vector2(((float)x)+ch.Bearing.x, ((float)y)-(ch.Size.y-ch.Bearing.y))*scale, zIndex-100.0f));
		shader->setMat4x4("model", model);
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// render quad
		//glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		// now advance cursors for next glyph
		x+=1+ch.Advance;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
#pragma endregion// TextRenderer
#pragma region LineRenderer
LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, const std::vector<Vector2>& _positions, const float& _width, const Vector2& _position, const bool& _loop) :
	Renderer2D(_engine, _shader, _position, 100.0f, Vector2::ONE, Vector2::Center, 0.0f), positions(_positions), width(_width), loop(_loop) {
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
	scale=Vector2(farthestX*2.0f, farthestY*2.0f);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), &verts[0], GL_STATIC_DRAW);// fill VBO buffer with vertex data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, const std::vector<Vector2>& _positions, const float& _width, const Vector2& _position) : LineRenderer(_engine, _shader, _positions, _width, _position, false) {}
LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, const std::vector<Vector2>& _positions, const float& _width, const bool& _loop) : LineRenderer(_engine, _shader, _positions, _width, Vector2::ZERO, _loop) {}
LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, const std::vector<Vector2>& _positions, const float& _width) : LineRenderer(_engine, _shader, _positions, _width, Vector2::ZERO, false) {}
void LineRenderer::draw() {
	if(engine->ended||!initialized) return;
	shader->bindTextures();
	shader->setMat4x4("model", createModelMat(position, zIndex, Vector2::ONE, Vector2::Center, 0.0f));
	glBindVertexArray(VAO);
	glLineWidth(width);
	glDrawArrays(loop ? GL_LINE_LOOP : GL_LINE_STRIP, 0, (GLsizei)positions.size());
}
#pragma endregion// LineRenderer