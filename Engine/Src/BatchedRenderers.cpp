#include "BatchedRenderers.h"

#pragma region BatchedSpriteRenderer
void BatchedSpriteRenderer::bufferQuad(const Vector4& modulate, Texture* tex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	float texIndex=-1.0f;
	if(tex!=nullptr) {
		for(unsigned int i=0; i<numTextures; i++) {
			if(textures[i]->ID==tex->ID) { texIndex=static_cast<float>(i);break; }
		}
		if(texIndex==-1.0f) {
			if(numTextures>=32) renderBatch();// if out of room for textures in this batch, render and reset
			texIndex=static_cast<float>(numTextures);textures.push_back(tex);numTextures++;
		}
	}
	for(unsigned int i=0; i<static_cast<unsigned int>(4*5); i+=5) {
		quadVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex {
			_position.x+(SpriteRenderer::quadvertices[i+0]-_anchor.x)*_scale.x, _position.y+(SpriteRenderer::quadvertices[i+1]-_anchor.y)*_scale.y,
				_zIndex-100.0f,
				SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4],
				modulate.x, modulate.y, modulate.z, modulate.w,
				texIndex
		});
	}
	numQuads++;
	if(numQuads>=maxQuadCount) renderBatch();// if out of room for quads in this batch, render and reset
}
void BatchedSpriteRenderer::renderBatch() {
	if(numQuads==0) return;
	shader->use();
	for(unsigned int i=0; i<numTextures; i++) textures[i]->Bind(i);
	VBO->dynamicSub((float*)(&quadVerticesBuffer[0]), 10*4*numQuads);//10 floats per vertex, 4 vertices per quad
	VAO->drawTrisIndexed(numQuads*6);
	quadVerticesBuffer.clear();
	numQuads=0;
	textures.clear();
	numTextures=0;
}
BatchedSpriteRenderer::BatchedSpriteRenderer(OrthoCam* _cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), cam(_cam) {
	engine_assert(cam!=nullptr, "[BatchedSpriteRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(10*maxVertices);
	IBO->staticFillRepeated(SpriteRenderer::quadindices, 6, maxQuadCount, 4);
	VBO->applyAttributes({ 3, 2, 4, 1 });
}
BatchedSpriteRenderer::~BatchedSpriteRenderer() {
	for(BatchedQuadData* quad:quads) delete quad;
}
BatchedQuadData* BatchedSpriteRenderer::addSprite(const Vector4& modulate, Texture* tex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	BatchedQuadData* ptr=new BatchedQuadData(modulate, tex, _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
BatchedQuadData* BatchedSpriteRenderer::addQuad(const Vector4& modulate, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	BatchedQuadData* ptr=new BatchedQuadData(modulate, nullptr, _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
void BatchedSpriteRenderer::draw() {
	// reserve space for vertices equal to the amount of quads x 4
	if(quads.size()<=maxQuadCount) quadVerticesBuffer.reserve(quads.size()*4); else quadVerticesBuffer.reserve(maxVertices);
	for(BatchedQuadData* quad:quads) bufferQuad(quad->modulate, quad->tex, quad->getWorldPos(), quad->zIndex, quad->getWorldScale(), quad->anchor);
	renderBatch();
}
#pragma endregion// BatchedSpriteRenderer

#pragma region StaticBatchedSpriteRenderer
void StaticBatchedSpriteRenderer::bufferQuad(const Vector4& modulate, Texture* tex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(numQuads>=maxQuadCount) return;//dont try to add more than the max, this should not occur though
	float texIndex=-1.0f;
	for(unsigned int i=0; i<numTextures; i++) {
		if(textures[i]->ID==tex->ID) { texIndex=static_cast<float>(i);break; }
	}
	if(texIndex==-1.0f) {
		if(numTextures>=32) return;// if out of room for textures in this batch, render and reset
		texIndex=static_cast<float>(numTextures);textures.push_back(tex);numTextures++;
	}
	for(unsigned int i=0; i<static_cast<unsigned int>(4*5); i+=5) {
		quadVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex {
			_position.x+(SpriteRenderer::quadvertices[i+0]-_anchor.x)*_scale.x, _position.y+(SpriteRenderer::quadvertices[i+1]-_anchor.y)*_scale.y,
				_zIndex-100.0f,
				SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4],
				modulate.x, modulate.y, modulate.z, modulate.w,
				texIndex
		});
	}
	numQuads++;
}
void StaticBatchedSpriteRenderer::renderBatch() {
	if(numQuads==0) return;
	shader->use();
	for(unsigned int i=0; i<numTextures; i++) textures[i]->Bind(i);
	VAO->drawTrisIndexed(numQuads*6);
}
StaticBatchedSpriteRenderer::StaticBatchedSpriteRenderer(OrthoCam* _cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), cam(_cam) {
	engine_assert(cam!=nullptr, "[StaticBatchedSpriteRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(10*maxVertices);
	IBO->staticFillRepeated(SpriteRenderer::quadindices, 6, maxQuadCount, 4);
	VBO->applyAttributes({ 3, 2, 4, 1 });
}
StaticBatchedSpriteRenderer::~StaticBatchedSpriteRenderer() {
	for(BatchedQuadData* quad:quads) delete quad;
}
BatchedQuadData* StaticBatchedSpriteRenderer::addSprite(const Vector4& modulate, Texture* tex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(quads.size()>maxQuadCount) return nullptr;
	BatchedQuadData* ptr=new BatchedQuadData(modulate, tex, _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
BatchedQuadData* StaticBatchedSpriteRenderer::addQuad(const Vector4& modulate, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(quads.size()>maxQuadCount) return nullptr;
	BatchedQuadData* ptr=new BatchedQuadData(modulate, nullptr, _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
void StaticBatchedSpriteRenderer::bind() {
	numQuads=0;
	textures.clear();
	numTextures=0;
	quadVerticesBuffer.reserve(std::min(static_cast<unsigned short int>(quads.size()), maxQuadCount)*4);// we know there will be 4 vertices per quad so reserve the memory for that many vertices
	for(BatchedQuadData* quad:quads) bufferQuad(quad->modulate, quad->tex, quad->getWorldPos(), quad->zIndex, quad->getWorldScale(), quad->anchor);
	VBO->dynamicSub((float*)(&quadVerticesBuffer[0]), 10*4*numQuads);//10 floats per vertex, 4 vertices per quad
	quadVerticesBuffer.clear();// clear out the memory for it, it is no longer being used
}
void StaticBatchedSpriteRenderer::draw() {
	renderBatch();
}
#pragma endregion// StaticBatchedSpriteRenderer

#pragma region BatchedTextRenderer
struct TextRenderer::Character {
	Texture* tex=nullptr;// ID handle of the glyph texture
	Vector2   Size;		       // Size of glyph
	Vector2   Bearing;         // Offset from baseline to left/top of glyph
	float Advance=0.0f;        // Offset to advance to next glyph
};
void BatchedTextRenderer::bufferCharacter(const int& shaderIndex, const float& texIndex, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale) {
	for(unsigned int i=0; i<static_cast<unsigned int>(4*5); i+=5) {
		characterBuffers[shaderIndex].push_back((BatchedVertex&&)BatchedVertex {
			_position.x+SpriteRenderer::quadvertices[i+0]*_scale.x, _position.y+SpriteRenderer::quadvertices[i+1]*_scale.y,// x and y
				_zIndex-100.0f,// z
				SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4],// u and v
				color.x, color.y, color.z, color.w,// r, g, b, and a
				texIndex
		});
	}
	numChars[shaderIndex]++;
	if(numChars[shaderIndex]>=maxCharacterCount) renderBatch(shaderIndex);// if out of room render and reset
}
void BatchedTextRenderer::renderBatch(const int& shaderIndex) {
	if(numChars[shaderIndex]==0) return;
	shader->use();
	for(int i=0; i<32; i++) {
		// equivilent to '''textureArrays[shaderIndex][i]->Bind(i);'''
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, textureArrays[shaderIndex][i]->ID);
	}
	VBO->dynamicSub((float*)(&characterBuffers[shaderIndex][0]), 10*4*numChars[shaderIndex]);//10 floats per vertex, 4 vertices per quad
	VAO->drawTrisIndexed(numChars[shaderIndex]*6);
	characterBuffers[shaderIndex].clear();
	numChars[shaderIndex]=0;
}
BatchedTextRenderer::BatchedTextRenderer(Camera* cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/textBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {
	engine_assert(cam!=nullptr, "[BatchedTextRenderer]: cam is nullptr");
	if(!TextRenderer::characterMapInitialized) {
		engine_assert(TextRenderer::initCharacterMap(), "[BatchedTextRenderer]: Error initializing font \"Fonts/MonocraftBetterBrackets.ttf\"");
	}
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();
	for(unsigned int shaderIndex=0; shaderIndex<3; shaderIndex++) {
		// set textures from TextRenderer class into textureArrays
		for(int i=0; i<32; i++) {
			textureArrays[shaderIndex][i]=TextRenderer::Characters[static_cast<char>((shaderIndex+1)*32+i)].tex;
		}
	}

	VBO->dynamicDefine(10*maxVertices);
	IBO->staticFillRepeated(SpriteRenderer::quadindices, 6, maxCharacterCount, 4);
	VBO->applyAttributes({ 3, 2, 4, 1 });
}
BatchedTextRenderer::~BatchedTextRenderer() {
	for(BatchedTextData* _text:text) delete _text;
}
BatchedTextData* BatchedTextRenderer::addText(const std::string& _text, const Vector4& color, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor) {
	BatchedTextData* ptr=new BatchedTextData(_text, color, _position, _zIndex, _scale, _anchor);
	text.push_back(ptr);
	return ptr;
}
void BatchedTextRenderer::draw() {
	for(BatchedTextData* _text:text) {
		// iterate through all characters to find scale of full text block
		Vector2 tmpScale=Vector2(0.0f, 9.0f);
		float curX=0.0f;
		const char* cstr=_text->text.c_str();
		int len=static_cast<int>(_text->text.length());
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
		Vector2 offset=_text->getWorldPos()-Vector2(tmpScale.x*(_text->anchor.x+0.5f), tmpScale.y*(_text->anchor.y-0.5f))*_text->scale;
		// iterate through all characters and add to buffer
		float x=0.0f;
		float y=0.0f;
		for(int c=0;c!=len;c++) {
			char charC=cstr[c];
			int intC=static_cast<int>(charC);
			if(charC=='\n') { x=0.0f;y-=9.0f; continue; } else if(charC=='\r') { x=0.0f; continue; }
			TextRenderer::Character ch=TextRenderer::Characters[intC];
			if(charC==' ') { x+=1.0f+ch.Advance; continue; }// skip one space and continue
			else if(charC=='\t') { x+=1.0f+ch.Advance*4.0f; continue; }//4 character spaces
			bufferCharacter(
				((intC)/32)-1,
				static_cast<float>((intC)%32),
				_text->color,
				offset+(Vector2(x, y)+ch.Bearing-Vector2(_text->anchor.x*ch.Size.x, (ch.Size.y)/2.0f+9.0f))*_text->scale,
				_text->zIndex,
				ch.Size*_text->scale
			);
			// now advance cursors for next glyph
			x+=1.0f+ch.Advance;
		}
	}
	for(int shaderIndex=0; shaderIndex<3; shaderIndex++) {
		renderBatch(shaderIndex);
	}
}
#pragma endregion// BatchedTextRenderer

#pragma region BatchedLineRenderer
void BatchedLineRenderer::bufferVertex(const Vector2& vertexPos, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale) {
	verticesBuffer.push_back((BatchedVertex&&)BatchedVertex {
		_position.x+vertexPos.x*_scale.x, _position.y+vertexPos.y*_scale.y,
			_zIndex-100.0f,
			0.0f, 0.0f,// no uvs
			color.x, color.y, color.z, color.w,
			-1.0f// no texture
	});
	numVertices++;
}
void BatchedLineRenderer::bufferLine(const std::vector<Vector2>& positions, const bool& loop, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale) {
	unsigned int len=static_cast<unsigned int>(positions.size());
	for(unsigned int i=0u; i<len-1u; i++) {
		bufferVertex(positions[i], color, _position, _zIndex, _scale);
		bufferVertex(positions[i+1u], color, _position, _zIndex, _scale);
		if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	}
	if(loop) {
		bufferVertex(positions[0u], color, _position, _zIndex, _scale);
		bufferVertex(positions[len-1u], color, _position, _zIndex, _scale);
		if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	}
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
}
void BatchedLineRenderer::bufferRect(const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale) {
	//{ Vector2(-0.5f, 0.5f), Vector2(0.5f, 0.5f), Vector2(0.5f, -0.5f), Vector2(-0.5f, -0.5f) }
	bufferVertex(Vector2(-0.5f, 0.5f), color, _position, _zIndex, _scale);
	bufferVertex(Vector2(0.5f, 0.5f), color, _position, _zIndex, _scale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(0.5f, 0.5f), color, _position, _zIndex, _scale);
	bufferVertex(Vector2(0.5f, -0.5f), color, _position, _zIndex, _scale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(0.5f, -0.5f), color, _position, _zIndex, _scale);
	bufferVertex(Vector2(-0.5f, -0.5f), color, _position, _zIndex, _scale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(-0.5f, -0.5f), color, _position, _zIndex, _scale);
	bufferVertex(Vector2(-0.5f, 0.5f), color, _position, _zIndex, _scale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
}
void BatchedLineRenderer::renderBatch() {
	if(numVertices==0) return;
	shader->use();
	VBO->dynamicSub((float*)(&verticesBuffer[0]), 10*numVertices);//10 floats per vertex
	VAO->drawLines(numVertices, width, false);
	verticesBuffer.clear();
	numVertices=0;
}
BatchedLineRenderer::BatchedLineRenderer(OrthoCam* _cam, const float& _width) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), cam(_cam), width(_width) {
	engine_assert(cam!=nullptr, "[BatchedLineRenderer]: cam is nullptr");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(10*maxVertices);
	VBO->applyAttributes({ 3, 2, 4, 1 });
}
BatchedLineRenderer::~BatchedLineRenderer() {
	for(BatchedLineData* line:lines) delete line;
}
BatchedLineData* BatchedLineRenderer::addLine(std::vector<Vector2>&& positions, const bool& loop, const Vector4& color, const Vector2& _position, const float& _zIndex) {
	BatchedLineData* ptr=new BatchedLineData((std::vector<Vector2>&&)positions, loop, color, _position, _zIndex);
	lines.push_back(ptr);
	return ptr;
}
BatchedRectData* BatchedLineRenderer::addRect(const Vector4& color, const Vector2& _position, const float& _zIndex) {
	BatchedRectData* ptr=new BatchedRectData(color, _position, _zIndex);
	rects.push_back(ptr);
	return ptr;
}
void BatchedLineRenderer::draw() {
	for(BatchedLineData* line:lines) bufferLine(line->positions, line->loop, line->color, line->getWorldPos(), line->zIndex, line->getWorldScale());
	for(BatchedRectData* rect:rects) bufferRect(rect->color, rect->getWorldPos(), rect->zIndex, rect->getWorldScale());
	renderBatch();
}
#pragma endregion// BatchedLineRenderer

#pragma region StaticBatchedLineRenderer
void StaticBatchedLineRenderer::bufferVertex(const Vector2& vertexPos, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale) {
	verticesBuffer.push_back((BatchedVertex&&)BatchedVertex {
		_position.x+vertexPos.x*_scale.x, _position.y+vertexPos.y*_scale.y,
			_zIndex-100.0f,
			0.0f, 0.0f,// no uvs
			color.x, color.y, color.z, color.w,
			-1.0f// no texture
	});
	numVertices++;
}
void StaticBatchedLineRenderer::bufferLine(const std::vector<Vector2>& positions, const bool& loop, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale) {
	unsigned int len=static_cast<unsigned int>(positions.size());
	for(unsigned int i=0u; i<len-1u; i++) {
		bufferVertex(positions[i], color, _position, _zIndex, _scale);
		bufferVertex(positions[i+1u], color, _position, _zIndex, _scale);
		if(numVertices>=maxVertices) return;// if out of room for lines in this batch, render and reset
	}
	if(loop) {
		bufferVertex(positions[0u], color, _position, _zIndex, _scale);
		bufferVertex(positions[len-1u], color, _position, _zIndex, _scale);
		if(numVertices>=maxVertices) return;// if out of room for lines in this batch, render and reset
	}
	if(numVertices>=maxVertices) return;// if out of room for lines in this batch, render and reset
}
void StaticBatchedLineRenderer::bufferRect(const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale) {
	//{ Vector2(-0.5f, 0.5f), Vector2(0.5f, 0.5f), Vector2(0.5f, -0.5f), Vector2(-0.5f, -0.5f) }
	bufferVertex(Vector2(-0.5f, 0.5f), color, _position, _zIndex, _scale);
	bufferVertex(Vector2(0.5f, 0.5f), color, _position, _zIndex, _scale);
	if(numVertices>=maxVertices) return;// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(0.5f, 0.5f), color, _position, _zIndex, _scale);
	bufferVertex(Vector2(0.5f, -0.5f), color, _position, _zIndex, _scale);
	if(numVertices>=maxVertices) return;// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(0.5f, -0.5f), color, _position, _zIndex, _scale);
	bufferVertex(Vector2(-0.5f, -0.5f), color, _position, _zIndex, _scale);
	if(numVertices>=maxVertices) return;// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(-0.5f, -0.5f), color, _position, _zIndex, _scale);
	bufferVertex(Vector2(-0.5f, 0.5f), color, _position, _zIndex, _scale);
	if(numVertices>=maxVertices) return;// if out of room for lines in this batch, render and reset
}
void StaticBatchedLineRenderer::renderBatch() {
	if(numVertices==0) return;
	shader->use();
	VAO->drawLines(numVertices, width, false);
}
StaticBatchedLineRenderer::StaticBatchedLineRenderer(OrthoCam* _cam, const float& _width) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), cam(_cam), width(_width) {
	engine_assert(cam!=nullptr, "[StaticBatchedLineRenderer]: cam is nullptr");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(10*maxVertices);
	VBO->applyAttributes({ 3, 2, 4, 1 });
}
StaticBatchedLineRenderer::~StaticBatchedLineRenderer() {
	for(BatchedLineData* line:lines) delete line;
}
BatchedLineData* StaticBatchedLineRenderer::addLine(std::vector<Vector2>&& positions, const bool& loop, const Vector4& color, const Vector2& _position, const float& _zIndex) {
	if(numVertices+(positions.size()-1u)*2u>maxVertices) return nullptr;
	BatchedLineData* ptr=new BatchedLineData((std::vector<Vector2>&&)positions, loop, color, _position, _zIndex);
	lines.push_back(ptr);
	return ptr;
}
BatchedRectData* StaticBatchedLineRenderer::addRect(const Vector4& color, const Vector2& _position, const float& _zIndex) {
	if(numVertices+8>maxVertices) return nullptr;
	BatchedRectData* ptr=new BatchedRectData(color, _position, _zIndex);
	rects.push_back(ptr);
	return ptr;
}
void StaticBatchedLineRenderer::bind() {
	numVertices=0;
	for(BatchedLineData* line:lines) bufferLine(line->positions, line->loop, line->color, line->getWorldPos(), line->zIndex, line->getWorldScale());
	for(BatchedRectData* rect:rects) bufferRect(rect->color, rect->getWorldPos(), rect->zIndex, rect->getWorldScale());
	VBO->dynamicSub((float*)(&verticesBuffer[0]), 10*numVertices);//10 floats per vertex
	verticesBuffer.clear();// clear out the memory for it, it is no longer being used
}
void StaticBatchedLineRenderer::draw() {
	renderBatch();
}
#pragma endregion// StaticBatchedLineRenderer

#pragma region BatchedDotRenderer
void BatchedDotRenderer::bufferDot(const Vector4& modulate, Texture* tex, const float& _radius, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	float texIndex=-1.0f;
	if(tex!=nullptr) {
		for(unsigned int i=0u; i<numTextures; i++) {
			if(textures[i]->ID==tex->ID) { texIndex=static_cast<float>(i);break; }
		}
		if(texIndex==-1.0f) {
			if(numTextures>=32u) renderBatch();// if out of room for textures in this batch, render and reset
			texIndex=static_cast<float>(numTextures);textures.push_back(tex);numTextures++;
		}
	}
	for(unsigned int i=0u; i<3u*5u; i+=5u) {
		dotVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex {
			_position.x+(DotRenderer::vertices[i]-_anchor.x)*_radius*_scale.x, _position.y+(DotRenderer::vertices[i+1u]-_anchor.y)*_radius*_scale.y,
				_zIndex-100.0f,
				DotRenderer::vertices[i+3u], DotRenderer::vertices[i+4u],
				modulate.x, modulate.y, modulate.z, modulate.w,
				texIndex
		});
	}
	numDots++;
	if(numDots>=maxDotCount) renderBatch();// if out of room for quads in this batch, render and reset
}
void BatchedDotRenderer::renderBatch() {
	if(numDots==0u) return;
	shader->use();
	for(unsigned int i=0u; i<numTextures; i++) textures[i]->Bind(i);
	VBO->dynamicSub((float*)(&dotVerticesBuffer[0u]), 10u*3u*numDots);//10 floats per vertex, 4 vertices per quad
	VAO->drawTris(numDots*3u);
	dotVerticesBuffer.clear();
	numDots=0u;
	textures.clear();
	numTextures=0u;
}
BatchedDotRenderer::BatchedDotRenderer(OrthoCam* _cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/dotTexBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), cam(_cam) {
	engine_assert(cam!=nullptr, "[BatchedDotRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(10u*maxVertices);
	VBO->applyAttributes({ 3u, 2u, 4u, 1u });
}
BatchedDotRenderer::~BatchedDotRenderer() {
	for(BatchedDotData* dot:dots) delete dot;
}
BatchedDotData* BatchedDotRenderer::addTexturedDot(const Vector4& modulate, Texture* tex, const float& radius, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	BatchedDotData* ptr=new BatchedDotData(modulate, tex, radius, _position, _zIndex, _scale, _anchor);
	dots.push_back(ptr);
	return ptr;
}
BatchedDotData* BatchedDotRenderer::addDot(const Vector4& modulate, const float& radius, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	BatchedDotData* ptr=new BatchedDotData(modulate, nullptr, radius, _position, _zIndex, _scale, _anchor);
	dots.push_back(ptr);
	return ptr;
}
void BatchedDotRenderer::draw() {
	// reserve space for vertices equal to the amount of dots x 4
	if(dots.size()<=maxDotCount) dotVerticesBuffer.reserve(dots.size()*3); else dotVerticesBuffer.reserve(maxVertices);
	for(BatchedDotData* dot:dots) bufferDot(dot->modulate, dot->tex, dot->radius, dot->getWorldPos(), dot->zIndex, dot->getWorldScale(), dot->anchor);
	renderBatch();
}
#pragma endregion// BatchedDotRenderer

#pragma region StaticBatchedDotRenderer
void StaticBatchedDotRenderer::bufferDot(const Vector4& modulate, Texture* tex, const float& _radius, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	float texIndex=-1.0f;
	if(tex!=nullptr) {
		for(unsigned int i=0u; i<numTextures; i++) {
			if(textures[i]->ID==tex->ID) { texIndex=static_cast<float>(i);break; }
		}
		if(texIndex==-1.0f) {
			if(numTextures>=32u) return;
			texIndex=static_cast<float>(numTextures);textures.push_back(tex);numTextures++;
		}
	}
	for(unsigned int i=0u; i<3u*5u; i+=5u) {
		dotVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex {
			_position.x+(DotRenderer::vertices[i]-_anchor.x)*_radius*_scale.x, _position.y+(DotRenderer::vertices[i+1u]-_anchor.y)*_radius*_scale.y,
				_zIndex-100.0f,
				DotRenderer::vertices[i+3u], DotRenderer::vertices[i+4u],
				modulate.x, modulate.y, modulate.z, modulate.w,
				texIndex
		});
	}
	numDots++;
	if(numDots>=maxDotCount) return;// if out of room for quads in this batch, render and reset
}
void StaticBatchedDotRenderer::renderBatch() {
	if(numDots==0u) return;
	shader->use();
	for(unsigned int i=0u; i<numTextures; i++) textures[i]->Bind(i);
	VAO->drawTris(numDots*3u);
}
StaticBatchedDotRenderer::StaticBatchedDotRenderer(OrthoCam* _cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/dotTexBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), cam(_cam) {
	engine_assert(cam!=nullptr, "[StaticBatchedDotRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(10u*maxVertices);
	VBO->applyAttributes({ 3u, 2u, 4u, 1u });
}
StaticBatchedDotRenderer::~StaticBatchedDotRenderer() {
	for(BatchedDotData* dot:dots) delete dot;
}
BatchedDotData* StaticBatchedDotRenderer::addTexturedDot(const Vector4& modulate, Texture* tex, const float& radius, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	BatchedDotData* ptr=new BatchedDotData(modulate, tex, radius, _position, _zIndex, _scale, _anchor);
	dots.push_back(ptr);
	return ptr;
}
BatchedDotData* StaticBatchedDotRenderer::addDot(const Vector4& modulate, const float& radius, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	BatchedDotData* ptr=new BatchedDotData(modulate, nullptr, radius, _position, _zIndex, _scale, _anchor);
	dots.push_back(ptr);
	return ptr;
}
void StaticBatchedDotRenderer::bind() {
	numDots=0u;
	textures.clear();
	numTextures=0u;
	// reserve space for vertices equal to the amount of dots x 4
	if(dots.size()<=maxDotCount) dotVerticesBuffer.reserve(dots.size()*3); else dotVerticesBuffer.reserve(maxVertices);
	for(BatchedDotData* dot:dots) bufferDot(dot->modulate, dot->tex, dot->radius, dot->getWorldPos(), dot->zIndex, dot->getWorldScale(), dot->anchor);
	VBO->dynamicSub((float*)(&dotVerticesBuffer[0u]), 10u*3u*numDots);//10 floats per vertex, 4 vertices per quad
	dotVerticesBuffer.clear();// clear out the memory for it, it is no longer being used
}
void StaticBatchedDotRenderer::draw() {
	renderBatch();
}
#pragma endregion// StaticBatchedDotRenderer

#pragma region BatchedSpritesheetRenderer
Vector2 translateRotateScaleUvs(const Vector2& uv, const Vector2& atlasSize, const Vector2& texPos, const Vector2& texSize, const float& texRot) {
	if(texRot!=0.0f) {
		return Vector2(texPos.x/atlasSize.x, texPos.y/atlasSize.y)+Vector2(
			(cosf(texRot)*(uv.x-0.5f)+sinf(texRot)*(uv.y-0.5f)+0.5f)*(texSize.x/atlasSize.x),
			(cosf(texRot)*(uv.y-0.5f)-sinf(texRot)*(uv.x-0.5f)+0.5f)*(texSize.y/atlasSize.y)
		);
	} else {
		return Vector2(texPos.x/atlasSize.x, texPos.y/atlasSize.y)+Vector2(uv.x*(texSize.x/atlasSize.x), uv.y*(texSize.y/atlasSize.y));
	}
}
void BatchedSpritesheetRenderer::bufferQuad(const Vector4& modulate, Texture* tex, const Vector2& atlasSize, const Vector2& texPos, const Vector2& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	float texIndex=-1.0f;
	for(unsigned int i=0; i<numTextures; i++) {
		if(textures[i]->ID==tex->ID) { texIndex=static_cast<float>(i);break; }
	}
	if(texIndex==-1.0f) {
		if(numTextures>=32) renderBatch();// if out of room for textures in this batch, render and reset
		texIndex=static_cast<float>(numTextures);textures.push_back(tex);numTextures++;
	}
	for(unsigned int i=0; i<static_cast<unsigned int>(4*5); i+=5) {
		Vector2 uv=translateRotateScaleUvs(Vector2(SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4]), atlasSize, texPos, texSize, texRot);
		quadVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex {
			_position.x+(SpriteRenderer::quadvertices[i+0]-_anchor.x)*_scale.x, _position.y+(SpriteRenderer::quadvertices[i+1]-_anchor.y)*_scale.y,
				_zIndex-100.0f,
				uv.x, uv.y,
				modulate.x, modulate.y, modulate.z, modulate.w,
				texIndex
		});
	}
	numQuads++;
	if(numQuads>=maxQuadCount) renderBatch();// if out of room for quads in this batch, render and reset
}
void BatchedSpritesheetRenderer::renderBatch() {
	if(numQuads==0) return;
	shader->use();
	for(unsigned int i=0; i<numTextures; i++) textures[i]->Bind(i);
	VBO->dynamicSub((float*)(&quadVerticesBuffer[0]), 10*4*numQuads);//10 floats per vertex, 4 vertices per quad
	VAO->drawTrisIndexed(numQuads*6);
	quadVerticesBuffer.clear();
	numQuads=0;
	textures.clear();
	numTextures=0;
}
BatchedSpritesheetRenderer::BatchedSpritesheetRenderer(OrthoCam* _cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), cam(_cam) {
	engine_assert(cam!=nullptr, "[BatchedSpritesheetRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(10*maxVertices);
	IBO->staticFillRepeated(SpriteRenderer::quadindices, 6, maxQuadCount, 4);
	VBO->applyAttributes({ 3, 2, 4, 1 });
}
BatchedSpritesheetRenderer::~BatchedSpritesheetRenderer() {
	for(BatchedAtlasedSpriteData* quad:quads) delete quad;
}
BatchedAtlasedSpriteData* BatchedSpritesheetRenderer::addSprite(const Vector4& modulate, Texture* tex, const Vector2& atlasSize, const Vector2& texPos, const Vector2& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	BatchedAtlasedSpriteData* ptr=new BatchedAtlasedSpriteData(modulate, tex, atlasSize, texPos, texSize, deg_to_rad(texRot), _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
BatchedAtlasedSpriteData* BatchedSpritesheetRenderer::addTruesizeSprite(const Vector4& modulate, Texture* tex, const Vector2& atlasSize, const Vector2& texPos, const Vector2& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor) {
	BatchedAtlasedSpriteData* ptr=new BatchedAtlasedSpriteData(modulate, tex, atlasSize, texPos, texSize, deg_to_rad(texRot), _position, _zIndex, texSize*_scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
void BatchedSpritesheetRenderer::draw() {
	// reserve space for vertices equal to the amount of quads x 4
	if(quads.size()<=maxQuadCount) quadVerticesBuffer.reserve(quads.size()*4); else quadVerticesBuffer.reserve(maxVertices);
	for(BatchedAtlasedSpriteData* quad:quads) bufferQuad(quad->modulate, quad->tex, quad->atlasSize, quad->texPos, quad->texSize, quad->texRot, quad->getWorldPos(), quad->zIndex, quad->getWorldScale(), quad->anchor);
	renderBatch();
}
#pragma endregion// BatchedSpriteRenderer

#pragma region StaticBatchedSpritesheetRenderer
void StaticBatchedSpritesheetRenderer::bufferQuad(const Vector4& modulate, Texture* tex, const Vector2& atlasSize, const Vector2& texPos, const Vector2& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	float texIndex=-1.0f;
	for(unsigned int i=0; i<numTextures; i++) {
		if(textures[i]->ID==tex->ID) { texIndex=static_cast<float>(i);break; }
	}
	if(texIndex==-1.0f) {
		if(numTextures>=32) renderBatch();// if out of room for textures in this batch, render and reset
		texIndex=static_cast<float>(numTextures);textures.push_back(tex);numTextures++;
	}
	for(unsigned int i=0; i<static_cast<unsigned int>(4*5); i+=5) {
		Vector2 uv=translateRotateScaleUvs(Vector2(SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4]), atlasSize, texPos, texSize, texRot);
		quadVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex {
			_position.x+(SpriteRenderer::quadvertices[i+0]-_anchor.x)*_scale.x, _position.y+(SpriteRenderer::quadvertices[i+1]-_anchor.y)*_scale.y,
				_zIndex-100.0f,
				uv.x, uv.y,
				modulate.x, modulate.y, modulate.z, modulate.w,
				texIndex
		});
	}
	numQuads++;
	if(numQuads>=maxQuadCount) renderBatch();// if out of room for quads in this batch, render and reset
}
void StaticBatchedSpritesheetRenderer::renderBatch() {
	if(numQuads==0) return;
	shader->use();
	for(unsigned int i=0; i<numTextures; i++) textures[i]->Bind(i);
	VAO->drawTrisIndexed(numQuads*6);
}
StaticBatchedSpritesheetRenderer::StaticBatchedSpritesheetRenderer(OrthoCam* _cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), cam(_cam) {
	engine_assert(cam!=nullptr, "[StaticBatchedSpritesheetRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(10*maxVertices);
	IBO->staticFillRepeated(SpriteRenderer::quadindices, 6, maxQuadCount, 4);
	VBO->applyAttributes({ 3, 2, 4, 1 });
}
StaticBatchedSpritesheetRenderer::~StaticBatchedSpritesheetRenderer() {
	for(BatchedAtlasedSpriteData* quad:quads) delete quad;
}
BatchedAtlasedSpriteData* StaticBatchedSpritesheetRenderer::addSprite(const Vector4& modulate, Texture* tex, const Vector2& atlasSize, const Vector2& texPos, const Vector2& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	BatchedAtlasedSpriteData* ptr=new BatchedAtlasedSpriteData(modulate, tex, atlasSize, texPos, texSize, deg_to_rad(texRot), _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
BatchedAtlasedSpriteData* StaticBatchedSpritesheetRenderer::addTruesizeSprite(const Vector4& modulate, Texture* tex, const Vector2& atlasSize, const Vector2& texPos, const Vector2& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor) {
	BatchedAtlasedSpriteData* ptr=new BatchedAtlasedSpriteData(modulate, tex, atlasSize, texPos, texSize, deg_to_rad(texRot), _position, _zIndex, texSize*_scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
void StaticBatchedSpritesheetRenderer::bind() {
	numQuads=0;
	textures.clear();
	numTextures=0;
	quadVerticesBuffer.reserve(std::min(static_cast<unsigned short int>(quads.size()), maxQuadCount)*4);// we know there will be 4 vertices per quad so reserve the memory for that many vertices
	for(BatchedAtlasedSpriteData* quad:quads) bufferQuad(quad->modulate, quad->tex, quad->atlasSize, quad->texPos, quad->texSize, quad->texRot, quad->getWorldPos(), quad->zIndex, quad->getWorldScale(), quad->anchor);
	VBO->dynamicSub((float*)(&quadVerticesBuffer[0]), 10*4*numQuads);//10 floats per vertex, 4 vertices per quad
	quadVerticesBuffer.clear();// clear out the memory for it, it is no longer being used
}
void StaticBatchedSpritesheetRenderer::draw() {
	renderBatch();
}
#pragma endregion// BatchedSpriteRenderer