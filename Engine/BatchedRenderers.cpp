#include "BatchedRenderers.h"

#pragma region BatchedSpriteRenderer
void BatchedSpriteRenderer::bufferQuad(BatchedQuadData* data) {
	if (!data->isActive()) return;
	float texIndex=-1.0f;
	if(data->tex!=nullptr) {
		for(unsigned int i=0u; i<numTextures; i++) {
			if(textures[i]->ID==data->tex->ID) { texIndex=static_cast<float>(i);break; }
		}
		if(texIndex==-1.0f) {
			if(numTextures>=maxTextures) renderBatch();// if out of room for textures in this batch, render and reset
			texIndex=static_cast<float>(numTextures);textures.push_back(data->tex);numTextures++;
		}
	}
	Vector2 worldPos=data->getWorldPos();
	Vector2 worldScale=data->getWorldScale();
	for(unsigned int i=0u; i<4u*5u; i+=5u) {
		Vector2 tmp=Vector2(SpriteRenderer::quadvertices[i+0u], SpriteRenderer::quadvertices[i+1u])-data->anchor;
		quadVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex(
			worldPos+Vector2(tmp.x*worldScale.x, tmp.y*worldScale.y),
			data->zIndex-100.0f,
			Vector2(SpriteRenderer::quadvertices[i+3u], SpriteRenderer::quadvertices[i+4u]),
			data->modulate,
			texIndex
		));
	}
	numQuads++;
	if(numQuads>=maxQuadCount) renderBatch();// if out of room for quads in this batch, render and reset
}
void BatchedSpriteRenderer::renderBatch() {
	if(numQuads==0u) return;
	shader->use();
	for(unsigned int i=0u; i<numTextures; i++) textures[i]->Bind(i);
	VBO->dynamicSub((float*)(&quadVerticesBuffer[0u]), static_cast<size_t>(10u*4u*numQuads));//10 floats per vertex, 4 vertices per quad
	VAO->drawTrisIndexed(numQuads*6u);
	quadVerticesBuffer.clear();
	numQuads=0u;
	textures.clear();
	numTextures=0u;
}
BatchedSpriteRenderer::BatchedSpriteRenderer(OrthoCam* cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {
	engine_assert(cam!=nullptr, "[BatchedSpriteRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(10u*maxVertices);
	IBO->staticFillRepeated(SpriteRenderer::quadindices, static_cast<size_t>(6), static_cast<size_t>(maxQuadCount), 4u);
	VBO->applyAttributes({ 3u, 2u, 4u, 1u });
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
	if(quads.size()<=maxQuadCount) quadVerticesBuffer.reserve(quads.size()*4u); else quadVerticesBuffer.reserve(maxVertices);
	for(BatchedQuadData* quad:quads) bufferQuad(quad);
	renderBatch();
}
#pragma endregion// BatchedSpriteRenderer

#pragma region StaticBatchedSpriteRenderer
void StaticBatchedSpriteRenderer::bufferQuad(BatchedQuadData* data) {
	if (!data->isActive()) return;
	if(numQuads>=maxQuadCount) return;//dont try to add more than the max, this should not occur though
	float texIndex=-1.0f;
	if(data->tex!=nullptr) {
		for(unsigned int i=0u; i<numTextures; i++) {
			if(textures[i]->ID==data->tex->ID) { texIndex=static_cast<float>(i);break; }
		}
		if(texIndex==-1.0f) {
			if(numTextures>=maxTextures) renderBatch();// if out of room for textures in this batch, render and reset
			texIndex=static_cast<float>(numTextures);textures.push_back(data->tex);numTextures++;
		}
	}
	Vector2 worldPos=data->getWorldPos();
	Vector2 worldScale=data->getWorldScale();
	for(unsigned int i=0u; i<4u*5u; i+=5u) {
		Vector2 tmp=Vector2(SpriteRenderer::quadvertices[i+0u], SpriteRenderer::quadvertices[i+1u])-data->anchor;
		quadVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex(
			worldPos+Vector2(tmp.x*worldScale.x, tmp.y*worldScale.y),
			data->zIndex-100.0f,
			Vector2(SpriteRenderer::quadvertices[i+3u], SpriteRenderer::quadvertices[i+4u]),
			data->modulate,
			texIndex
		));
	}
	numQuads++;
}
void StaticBatchedSpriteRenderer::renderBatch() {
	if(numQuads==0u) return;
	shader->use();
	for(unsigned int i=0u; i<numTextures; i++) textures[i]->Bind(i);
	VAO->drawTrisIndexed(numQuads*6u);
}
StaticBatchedSpriteRenderer::StaticBatchedSpriteRenderer(OrthoCam* cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {
	engine_assert(cam!=nullptr, "[StaticBatchedSpriteRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(10u*maxVertices);
	IBO->staticFillRepeated(SpriteRenderer::quadindices, static_cast<size_t>(6), static_cast<size_t>(maxQuadCount), 4u);
	VBO->applyAttributes({ 3u, 2u, 4u, 1u });
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
	numQuads=0u;
	textures.clear();
	numTextures=0u;
	quadVerticesBuffer.reserve(std::min(quads.size(), static_cast<size_t>(maxQuadCount))*4u);// we know there will be 4 vertices per quad so reserve the memory for that many vertices
	for(BatchedQuadData* quad:quads) bufferQuad(quad);
	if (numQuads!=0u) VBO->dynamicSub((float*)(&quadVerticesBuffer[0u]), static_cast<size_t>(10u*4u*numQuads));//draw 10 floats per vertex, 4 vertices per quad
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
void BatchedTextRenderer::bufferCharacter(const unsigned short int& shaderIndex, const float& texIndex, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale) {
	for(unsigned int i=0u; i<4u*5u; i+=5u) {
		characterBuffers[shaderIndex].push_back((BatchedVertex&&)BatchedVertex(
			_position+Vector2(SpriteRenderer::quadvertices[i+0u]*_scale.x, SpriteRenderer::quadvertices[i+1u]*_scale.y),
			_zIndex-100.0f,
			Vector2(SpriteRenderer::quadvertices[i+3u], SpriteRenderer::quadvertices[i+4u]),
			color,
			texIndex
		));
	}
	numChars[shaderIndex]++;
	if(numChars[shaderIndex]>=maxCharacterCount) renderBatch(shaderIndex);// if out of room render and reset
}
void BatchedTextRenderer::renderBatch(const unsigned short int& shaderIndex) {
	if(numChars[shaderIndex]==0u) return;
	shader->use();
	for(int i=0u; i<maxTextures; i++) {
		textureArrays[shaderIndex][i]->Bind(i);
	}
	VBO->dynamicSub((float*)(&characterBuffers[shaderIndex][0u]), static_cast<size_t>(10u*4u*numChars[shaderIndex]));//10 floats per vertex, 4 vertices per quad
	VAO->drawTrisIndexed(numChars[shaderIndex]*6u);
	characterBuffers[shaderIndex].clear();
	numChars[shaderIndex]=0u;
}
BatchedTextRenderer::BatchedTextRenderer(Camera* cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/textBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {
	engine_assert(cam!=nullptr, "[BatchedTextRenderer]: cam is nullptr");
	if(!TextRenderer::characterMapInitialized) {
		engine_assert(TextRenderer::initCharacterMap(), "[BatchedTextRenderer]: Error initializing font \"Resources/MonocraftBetterBrackets.ttf\"");
	}
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();
	for(unsigned short int shaderIndex=0u; shaderIndex<3u; shaderIndex++) {
		// set textures from TextRenderer class into textureArrays
		for(unsigned short int i=0u; i<32u; i++) {
			textureArrays[shaderIndex][i]=TextRenderer::Characters[static_cast<char>((shaderIndex+1u)*32u+i)].tex;
		}
	}

	VBO->dynamicDefine(10u*maxVertices);
	IBO->staticFillRepeated(SpriteRenderer::quadindices, static_cast<size_t>(6u), static_cast<size_t>(maxCharacterCount), 4u);
	VBO->applyAttributes({ 3u, 2u, 4u, 1u });
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
		if (!_text->isActive()) continue;
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
				((intC)/32u)-1,
				static_cast<float>((intC)%32u),
				_text->color,
				offset+(Vector2(x, y)+ch.Bearing-Vector2(_text->anchor.x*ch.Size.x, (ch.Size.y)/2.0f+9.0f))*_text->scale,
				_text->zIndex,
				ch.Size*_text->scale
			);
			// now advance cursors for next glyph
			x+=1.0f+ch.Advance;
		}
	}
	for(unsigned short int shaderIndex=0u; shaderIndex<3u; shaderIndex++) {
		renderBatch(shaderIndex);
	}
}
#pragma endregion// BatchedTextRenderer

#pragma region BatchedLineRenderer
void BatchedLineRenderer::bufferVertex(const Vector2& vertexPos, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale) {
	verticesBuffer.push_back((BatchedVertex&&)BatchedVertex(
		_position+Vector2(vertexPos.x*_scale.x, vertexPos.y*_scale.y),
		_zIndex-100.0f,
		Vector2::ZERO,// no uvs
		color,
		-1.0f// no texture
	));
	numVertices++;
}
void BatchedLineRenderer::bufferLine(BatchedLineData* data) {
	unsigned int len=static_cast<unsigned int>(data->positions.size());
	Vector2 worldPos=data->getWorldPos();
	Vector2 worldScale=data->getWorldScale();
	for(unsigned int i=0u; i<len-1u; i++) {
		bufferVertex(data->positions[i], data->color, worldPos, data->zIndex, worldScale);
		bufferVertex(data->positions[i+1u], data->color, worldPos, data->zIndex, worldScale);
		if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	}
	if(data->loop) {
		bufferVertex(data->positions[0u], data->color, worldPos, data->zIndex, worldScale);
		bufferVertex(data->positions[len-1u], data->color, worldPos, data->zIndex, worldScale);
		if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	}
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
}
void BatchedLineRenderer::bufferRect(BatchedRectData* data) {
	Vector2 worldPos=data->getWorldPos();
	Vector2 worldScale=data->getWorldScale();
	//{ Vector2(-0.5f, 0.5f), Vector2(0.5f, 0.5f), Vector2(0.5f, -0.5f), Vector2(-0.5f, -0.5f) }
	bufferVertex(Vector2(-0.5f, 0.5f), data->color, worldPos, data->zIndex, worldScale);
	bufferVertex(Vector2(0.5f, 0.5f), data->color, worldPos, data->zIndex, worldScale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(0.5f, 0.5f), data->color, worldPos, data->zIndex, worldScale);
	bufferVertex(Vector2(0.5f, -0.5f), data->color, worldPos, data->zIndex, worldScale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(0.5f, -0.5f), data->color, worldPos, data->zIndex, worldScale);
	bufferVertex(Vector2(-0.5f, -0.5f), data->color, worldPos, data->zIndex, worldScale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(-0.5f, -0.5f), data->color, worldPos, data->zIndex, worldScale);
	bufferVertex(Vector2(-0.5f, 0.5f), data->color, worldPos, data->zIndex, worldScale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
}
void BatchedLineRenderer::renderBatch() {
	if(numVertices==0u) return;
	shader->use();
	VBO->dynamicSub((float*)(&verticesBuffer[0u]), static_cast<size_t>(10u*numVertices));//10 floats per vertex
	VAO->drawLines(numVertices, width, false);
	verticesBuffer.clear();
	numVertices=0u;
}
BatchedLineRenderer::BatchedLineRenderer(OrthoCam* cam, const float& _width) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), width(_width) {
	engine_assert(cam!=nullptr, "[BatchedLineRenderer]: cam is nullptr");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(static_cast<size_t>(10u*maxVertices));
	VBO->applyAttributes({ 3u, 2u, 4u, 1u });
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
	for(BatchedLineData* line:lines) bufferLine(line);
	for(BatchedRectData* rect:rects) bufferRect(rect);
	renderBatch();
}
#pragma endregion// BatchedLineRenderer

#pragma region StaticBatchedLineRenderer
void StaticBatchedLineRenderer::bufferVertex(const Vector2& vertexPos, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale) {
	verticesBuffer.push_back((BatchedVertex&&)BatchedVertex(
		_position+Vector2(vertexPos.x*_scale.x, vertexPos.y*_scale.y),
		_zIndex-100.0f,
		Vector2::ZERO,// no uvs
		color,
		-1.0f// no texture
	));
	numVertices++;
}
void StaticBatchedLineRenderer::bufferLine(BatchedLineData* data) {
	unsigned int len=static_cast<unsigned int>(data->positions.size());
	Vector2 worldPos=data->getWorldPos();
	Vector2 worldScale=data->getWorldScale();
	for(unsigned int i=0u; i<len-1u; i++) {
		bufferVertex(data->positions[i], data->color, worldPos, data->zIndex, worldScale);
		bufferVertex(data->positions[i+1u], data->color, worldPos, data->zIndex, worldScale);
		if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	}
	if(data->loop) {
		bufferVertex(data->positions[0u], data->color, worldPos, data->zIndex, worldScale);
		bufferVertex(data->positions[len-1u], data->color, worldPos, data->zIndex, worldScale);
		if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	}
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
}
void StaticBatchedLineRenderer::bufferRect(BatchedRectData* data) {
	Vector2 worldPos=data->getWorldPos();
	Vector2 worldScale=data->getWorldScale();
	//{ Vector2(-0.5f, 0.5f), Vector2(0.5f, 0.5f), Vector2(0.5f, -0.5f), Vector2(-0.5f, -0.5f) }
	bufferVertex(Vector2(-0.5f, 0.5f), data->color, worldPos, data->zIndex, worldScale);
	bufferVertex(Vector2(0.5f, 0.5f), data->color, worldPos, data->zIndex, worldScale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(0.5f, 0.5f), data->color, worldPos, data->zIndex, worldScale);
	bufferVertex(Vector2(0.5f, -0.5f), data->color, worldPos, data->zIndex, worldScale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(0.5f, -0.5f), data->color, worldPos, data->zIndex, worldScale);
	bufferVertex(Vector2(-0.5f, -0.5f), data->color, worldPos, data->zIndex, worldScale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
	bufferVertex(Vector2(-0.5f, -0.5f), data->color, worldPos, data->zIndex, worldScale);
	bufferVertex(Vector2(-0.5f, 0.5f), data->color, worldPos, data->zIndex, worldScale);
	if(numVertices>=maxVertices) renderBatch();// if out of room for lines in this batch, render and reset
}
void StaticBatchedLineRenderer::renderBatch() {
	if(numVertices==0u) return;
	shader->use();
	VAO->drawLines(numVertices, width, false);
}
StaticBatchedLineRenderer::StaticBatchedLineRenderer(OrthoCam* cam, const float& _width) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), width(_width) {
	engine_assert(cam!=nullptr, "[StaticBatchedLineRenderer]: cam is nullptr");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(static_cast<size_t>(10u*maxVertices));
	VBO->applyAttributes({ 3u, 2u, 4u, 1u });
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
	if(numVertices+8u>maxVertices) return nullptr;
	BatchedRectData* ptr=new BatchedRectData(color, _position, _zIndex);
	rects.push_back(ptr);
	return ptr;
}
void StaticBatchedLineRenderer::bind() {
	numVertices=0u;
	for(BatchedLineData* line:lines) bufferLine(line);
	for(BatchedRectData* rect:rects) bufferRect(rect);
	if (numVertices!=0u) VBO->dynamicSub((float*)(&verticesBuffer[0u]), static_cast<size_t>(10u*numVertices));//10 floats per vertex
	verticesBuffer.clear();// clear out the memory for it, it is no longer being used
}
void StaticBatchedLineRenderer::draw() {
	renderBatch();
}
#pragma endregion// StaticBatchedLineRenderer

#pragma region BatchedDotRenderer
void BatchedDotRenderer::bufferDot(BatchedDotData* data) {
	if (!data->isActive()) return;
	float texIndex=-1.0f;
	if(data->tex!=nullptr) {
		for(unsigned int i=0u; i<numTextures; i++) {
			if(textures[i]->ID==data->tex->ID) { texIndex=static_cast<float>(i);break; }
		}
		if(texIndex==-1.0f) {
			if(numTextures>=maxTextures) renderBatch();// if out of room for textures in this batch, render and reset
			texIndex=static_cast<float>(numTextures);textures.push_back(data->tex);numTextures++;
		}
	}
	Vector2 worldPos=data->getWorldPos();
	Vector2 worldScale=data->getWorldScale();
	for(unsigned int i=0u; i<3u*5u; i+=5u) {
		Vector2 tmp=Vector2(DotRenderer::vertices[i+0u], DotRenderer::vertices[i+1u])-data->anchor;
		dotVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex(
			data->position+Vector2(tmp.x*worldScale.x, tmp.y*worldScale.y)*data->radius,
			data->zIndex-100.0f,
			Vector2(DotRenderer::vertices[i+3u], DotRenderer::vertices[i+4u]),
			data->modulate,
			texIndex
		));
	}
	numDots++;
	if(numDots>=maxDotCount) renderBatch();// if out of room for quads in this batch, render and reset
}
void BatchedDotRenderer::renderBatch() {
	if(numDots==0u) return;
	shader->use();
	for(unsigned int i=0u; i<numTextures; i++) textures[i]->Bind(i);
	VBO->dynamicSub((float*)(&dotVerticesBuffer[0u]), static_cast<size_t>(10u*3u*numDots));//10 floats per vertex, 4 vertices per quad
	VAO->drawTris(numDots*3u);
	dotVerticesBuffer.clear();
	numDots=0u;
	textures.clear();
	numTextures=0u;
}
BatchedDotRenderer::BatchedDotRenderer(OrthoCam* cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/dotTexBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {
	engine_assert(cam!=nullptr, "[BatchedDotRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(static_cast<size_t>(10u*maxVertices));
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
	if(dots.size()<=maxDotCount) dotVerticesBuffer.reserve(dots.size()*3u); else dotVerticesBuffer.reserve(static_cast<size_t>(maxVertices));
	for(BatchedDotData* dot:dots) bufferDot(dot);
	renderBatch();
}
#pragma endregion// BatchedDotRenderer

#pragma region StaticBatchedDotRenderer
void StaticBatchedDotRenderer::bufferDot(BatchedDotData* data) {
	if (!data->isActive()) return;
	float texIndex=-1.0f;
	if(data->tex!=nullptr) {
		for(unsigned int i=0u; i<numTextures; i++) {
			if(textures[i]->ID==data->tex->ID) { texIndex=static_cast<float>(i);break; }
		}
		if(texIndex==-1.0f) {
			if(numTextures>=maxTextures) renderBatch();// if out of room for textures in this batch, render and reset
			texIndex=static_cast<float>(numTextures);textures.push_back(data->tex);numTextures++;
		}
	}
	Vector2 worldPos=data->getWorldPos();
	Vector2 worldScale=data->getWorldScale();
	for(unsigned int i=0u; i<3u*5u; i+=5u) {
		Vector2 tmp=Vector2(DotRenderer::vertices[i+0u], DotRenderer::vertices[i+1u])-data->anchor;
		dotVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex(
			data->position+Vector2(tmp.x*worldScale.x, tmp.y*worldScale.y)*data->radius,
			data->zIndex-100.0f,
			Vector2(DotRenderer::vertices[i+3u], DotRenderer::vertices[i+4u]),
			data->modulate,
			texIndex
		));
	}
	numDots++;
	if(numDots>=maxDotCount) renderBatch();// if out of room for quads in this batch, render and reset
}
void StaticBatchedDotRenderer::renderBatch() {
	if(numDots==0u) return;
	shader->use();
	for(unsigned int i=0u; i<numTextures; i++) textures[i]->Bind(i);
	VAO->drawTris(numDots*3u);
}
StaticBatchedDotRenderer::StaticBatchedDotRenderer(OrthoCam* cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/dotTexBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {
	engine_assert(cam!=nullptr, "[StaticBatchedDotRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(static_cast<size_t>(10u*maxVertices));
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
	if(dots.size()<=maxDotCount) dotVerticesBuffer.reserve(dots.size()*3u); else dotVerticesBuffer.reserve(maxVertices);
	for(BatchedDotData* dot:dots) bufferDot(dot);
	if (numDots!=0u) VBO->dynamicSub((float*)(&dotVerticesBuffer[0u]), static_cast<size_t>(10u*3u*numDots));//10 floats per vertex, 4 vertices per quad
	dotVerticesBuffer.clear();// clear out the memory for it, it is no longer being used
}
void StaticBatchedDotRenderer::draw() {
	renderBatch();
}
#pragma endregion// StaticBatchedDotRenderer

#pragma region BatchedSpritesheetRenderer
Vector2 translateRotateScaleUvs(const Vector2& uv, const Vector2i& atlasSize, const Vector2i& texPos, const Vector2i& texSize, const float& texRot) {
	if(texRot!=0.0f) {
		float rad=deg_to_rad(texRot);
		return Vector2(texPos.x/(float)atlasSize.x, texPos.y/(float)atlasSize.y)+Vector2(
			(cosf(rad)*(uv.x-0.5f)+sinf(rad)*(uv.y-0.5f)+0.5f)*(texSize.x/(float)atlasSize.x),
			(cosf(rad)*(uv.y-0.5f)-sinf(rad)*(uv.x-0.5f)+0.5f)*(texSize.y/(float)atlasSize.y)
		);
	} else {
		return Vector2(texPos.x/(float)atlasSize.x, texPos.y/(float)atlasSize.y)+Vector2(uv.x*(texSize.x/(float)atlasSize.x), uv.y*(texSize.y/(float)atlasSize.y));
	}
}
void BatchedSpritesheetRenderer::bufferQuad(BatchedAtlasedSpriteData* data) {
	if (!data->isActive()) return;
	float texIndex=-1.0f;
	for(unsigned int i=0u; i<numTextures; i++) {
		if(textures[i]->ID==data->tex->ID) { texIndex=static_cast<float>(i);break; }
	}
	if(texIndex==-1.0f) {
		if(numTextures>=maxTextures) renderBatch();// if out of room for textures in this batch, render and reset
		texIndex=static_cast<float>(numTextures);textures.push_back(data->tex);numTextures++;
	}
	Vector2 worldPos=data->getWorldPos();
	Vector2 worldScale=data->getWorldScale();
	for(unsigned int i=0u; i<4u*5u; i+=5u) {
		Vector2 newUv=translateRotateScaleUvs(Vector2(SpriteRenderer::quadvertices[i+3u], SpriteRenderer::quadvertices[i+4u]), data->atlasSize, data->texPos, data->texSize, data->texRot);
		Vector2 tmp=Vector2(SpriteRenderer::quadvertices[i+0u], SpriteRenderer::quadvertices[i+1u])-data->anchor;
		quadVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex(
			worldPos+Vector2(tmp.x*worldScale.x, tmp.y*worldScale.y),
			data->zIndex-100.0f,
			newUv,
			data->modulate,
			texIndex
		));
	}
	numQuads++;
	if(numQuads>=maxQuadCount) renderBatch();// if out of room for quads in this batch, render and reset
}
void BatchedSpritesheetRenderer::renderBatch() {
	if(numQuads==0u) return;
	shader->use();
	for(unsigned int i=0u; i<numTextures; i++) textures[i]->Bind(i);
	VBO->dynamicSub((float*)(&quadVerticesBuffer[0u]), static_cast<size_t>(10u*4u*numQuads));//10 floats per vertex, 4 vertices per quad
	VAO->drawTrisIndexed(numQuads*6u);
	quadVerticesBuffer.clear();
	numQuads=0u;
	textures.clear();
	numTextures=0u;
}
BatchedSpritesheetRenderer::BatchedSpritesheetRenderer(OrthoCam* cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {
	engine_assert(cam!=nullptr, "[BatchedSpritesheetRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(static_cast<size_t>(10u*maxVertices));
	IBO->staticFillRepeated(SpriteRenderer::quadindices, 6u, static_cast<size_t>(maxQuadCount), static_cast<size_t>(4u));
	VBO->applyAttributes({ 3u, 2u, 4u, 1u });
}
BatchedSpritesheetRenderer::~BatchedSpritesheetRenderer() {
	for(BatchedAtlasedSpriteData* quad:quads) delete quad;
}
BatchedAtlasedSpriteData* BatchedSpritesheetRenderer::addSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const Vector2i& texPos, const Vector2i& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	BatchedAtlasedSpriteData* ptr=new BatchedAtlasedSpriteData(modulate, tex, atlasSize, texPos, texSize, texRot, _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
BatchedAtlasedSpriteData* BatchedSpritesheetRenderer::addTruesizeSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const Vector2i& texPos, const Vector2i& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor) {
	BatchedAtlasedSpriteData* ptr=new BatchedAtlasedSpriteData(modulate, tex, atlasSize, texPos, texSize, texRot, _position, _zIndex, texSize*_scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
void BatchedSpritesheetRenderer::draw() {
	// reserve space for vertices equal to the amount of quads x 4
	if(quads.size()<=maxQuadCount) quadVerticesBuffer.reserve(quads.size()*4u); else quadVerticesBuffer.reserve(maxVertices);
	for(BatchedAtlasedSpriteData* quad:quads) bufferQuad(quad);
	renderBatch();
}
#pragma endregion// BatchedSpriteRenderer

#pragma region StaticBatchedSpritesheetRenderer
void StaticBatchedSpritesheetRenderer::bufferQuad(BatchedAtlasedSpriteData* data) {
	if (!data->isActive()) return;
	float texIndex=-1.0f;
	for(unsigned int i=0; i<numTextures; i++) {
		if(textures[i]->ID==data->tex->ID) { texIndex=static_cast<float>(i);break; }
	}
	if(texIndex==-1.0f) {
		if(numTextures>=maxTextures) renderBatch();// if out of room for textures in this batch, render and reset
		texIndex=static_cast<float>(numTextures);textures.push_back(data->tex);numTextures++;
	}
	Vector2 worldPos=data->getWorldPos();
	Vector2 worldScale=data->getWorldScale();
	for(unsigned int i=0; i<4u*5u; i+=5u) {
		Vector2 newUv=translateRotateScaleUvs(Vector2(SpriteRenderer::quadvertices[i+3u], SpriteRenderer::quadvertices[i+4u]), data->atlasSize, data->texPos, data->texSize, data->texRot);
		Vector2 tmp=Vector2(SpriteRenderer::quadvertices[i+0u], SpriteRenderer::quadvertices[i+1u])-data->anchor;
		quadVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex(
			worldPos+Vector2(tmp.x*worldScale.x, tmp.y*worldScale.y),
			data->zIndex-100.0f,
			newUv,
			data->modulate,
			texIndex
		));
	}
	numQuads++;
	if(numQuads>=maxQuadCount) renderBatch();// if out of room for quads in this batch, render and reset
}
void StaticBatchedSpritesheetRenderer::renderBatch() {
	if(numQuads==0u) return;
	shader->use();
	for(unsigned int i=0u; i<numTextures; i++) textures[i]->Bind(i);
	VAO->drawTrisIndexed(numQuads*6u);
}
StaticBatchedSpritesheetRenderer::StaticBatchedSpritesheetRenderer(OrthoCam* cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {
	engine_assert(cam!=nullptr, "[StaticBatchedSpritesheetRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(static_cast<size_t>(10u*maxVertices));
	IBO->staticFillRepeated(SpriteRenderer::quadindices, 6u, static_cast<size_t>(maxQuadCount), static_cast<size_t>(4u));
	VBO->applyAttributes({ 3u, 2u, 4u, 1u });
}
StaticBatchedSpritesheetRenderer::~StaticBatchedSpritesheetRenderer() {
	for(BatchedAtlasedSpriteData* quad:quads) delete quad;
}
BatchedAtlasedSpriteData* StaticBatchedSpritesheetRenderer::addSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const Vector2i& texPos, const Vector2i& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	BatchedAtlasedSpriteData* ptr=new BatchedAtlasedSpriteData(modulate, tex, atlasSize, texPos, texSize, texRot, _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
BatchedAtlasedSpriteData* StaticBatchedSpritesheetRenderer::addTruesizeSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const Vector2i& texPos, const Vector2i& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor) {
	BatchedAtlasedSpriteData* ptr=new BatchedAtlasedSpriteData(modulate, tex, atlasSize, texPos, texSize, texRot, _position, _zIndex, texSize*_scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
void StaticBatchedSpritesheetRenderer::bind() {
	numQuads=0u;
	textures.clear();
	numTextures=0u;
	quadVerticesBuffer.reserve(std::min(quads.size(), static_cast<size_t>(maxQuadCount))*4u);// we know there will be 4 vertices per quad so reserve the memory for that many vertices
	for(BatchedAtlasedSpriteData* quad:quads) bufferQuad(quad);
	if (numQuads!=0u) VBO->dynamicSub((float*)(&quadVerticesBuffer[0u]), static_cast<size_t>(10u*4u*numQuads));//10 floats per vertex, 4 vertices per quad
	quadVerticesBuffer.clear();// clear out the memory for it, it is no longer being used
}
void StaticBatchedSpritesheetRenderer::draw() {
	renderBatch();
}
#pragma endregion// BatchedSpriteRenderer

#pragma region BatchedSpritesheetAnimationRenderer
void BatchedSpritesheetAnimationRenderer::bufferQuad(BatchedAtlasedAnimationData* data) {
	if (!data->isActive()) return;
	float texIndex=-1.0f;
	for(unsigned int i=0u; i<numTextures; i++) {
		if(textures[i]->ID==data->tex->ID) { texIndex=static_cast<float>(i);break; }
	}
	if(texIndex==-1.0f) {
		if(numTextures>=maxTextures) renderBatch();// if out of room for textures in this batch, render and reset
		texIndex=static_cast<float>(numTextures);textures.push_back(data->tex);numTextures++;
	}
	Vector2 worldPos=data->getWorldPos();
	Vector2 worldScale=data->getWorldScale();
	for(unsigned int i=0; i<4u*5u; i+=5u) {
		Vector2 newUv=translateRotateScaleUvs(Vector2(SpriteRenderer::quadvertices[i+3u], SpriteRenderer::quadvertices[i+4u]), data->atlasSize, data->texPos+data->frameIndex*data->animationDir, data->texSize, data->texRot);
		Vector2 tmp=Vector2(SpriteRenderer::quadvertices[i+0u], SpriteRenderer::quadvertices[i+1u])-data->anchor;
		quadVerticesBuffer.push_back((BatchedVertex&&)BatchedVertex(
			worldPos+Vector2(tmp.x*worldScale.x, tmp.y*worldScale.y),
			data->zIndex-100.0f,
			newUv,
			data->modulate,
			texIndex
		));
	}
	numQuads++;
	if(numQuads>=maxQuadCount) renderBatch();// if out of room for quads in this batch, render and reset
}
void BatchedSpritesheetAnimationRenderer::renderBatch() {
	if(numQuads==0u) return;
	shader->use();
	for(unsigned int i=0u; i<numTextures; i++) textures[i]->Bind(i);
	VBO->dynamicSub((float*)(&quadVerticesBuffer[0u]), static_cast<size_t>(10u*4u*numQuads));//10 floats per vertex, 4 vertices per quad
	VAO->drawTrisIndexed(numQuads*6u);
	quadVerticesBuffer.clear();
	numQuads=0u;
	textures.clear();
	numTextures=0u;
}
BatchedSpritesheetAnimationRenderer::BatchedSpritesheetAnimationRenderer(OrthoCam* cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {
	engine_assert(cam!=nullptr, "[BatchedSpritesheetAnimationRenderer]: cam is nullptr");
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	VBO->dynamicDefine(static_cast<size_t>(10u*maxVertices));
	IBO->staticFillRepeated(SpriteRenderer::quadindices, 6u, static_cast<size_t>(maxQuadCount), static_cast<size_t>(4u));
	VBO->applyAttributes({ 3u, 2u, 4u, 1u });
	Engine::instance->sub_loop(this);
}
BatchedSpritesheetAnimationRenderer::~BatchedSpritesheetAnimationRenderer() {
	for(BatchedAtlasedAnimationData* quad:quads) delete quad;
}
BatchedAtlasedAnimationData* BatchedSpritesheetAnimationRenderer::addSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const unsigned short int& numFrames, const double& frameDelay, const Vector2i& texPos, const Vector2i& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	BatchedAtlasedAnimationData* ptr=new BatchedAtlasedAnimationData(modulate, tex, atlasSize, numFrames, frameDelay, texPos, texSize, texRot, _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
BatchedAtlasedAnimationData* BatchedSpritesheetAnimationRenderer::addTruesizeSprite(const Vector4& modulate, Texture* tex, const Vector2i& atlasSize, const unsigned short int& numFrames, const double& frameDelay, const Vector2i& texPos, const Vector2i& texSize, const float& texRot, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor) {
	BatchedAtlasedAnimationData* ptr=new BatchedAtlasedAnimationData(modulate, tex, atlasSize, numFrames, frameDelay, texPos, texSize, texRot, _position, _zIndex, texSize*_scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
void BatchedSpritesheetAnimationRenderer::draw() {
	// reserve space for vertices equal to the amount of quads x 4
	if(quads.size()<=maxQuadCount) quadVerticesBuffer.reserve(quads.size()*4u); else quadVerticesBuffer.reserve(maxVertices);
	for(BatchedAtlasedAnimationData* quad:quads) bufferQuad(quad);
	renderBatch();
}
void BatchedSpritesheetAnimationRenderer::on_loop(const double& delta) {
	for(BatchedAtlasedAnimationData* quad:quads) quad->on_loop(delta);
}
#pragma endregion// BatchedSpriteRenderer