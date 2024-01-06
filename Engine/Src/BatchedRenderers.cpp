#include "BatchedRenderers.h"

#pragma region BatchedSpriteRenderer
void BatchedSpriteRenderer::bufferQuad(const Vector4& modulate, Texture* tex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(Engine::instance->ended||!initialized) return;
	if(numQuads>=maxQuadCount) renderBatch();// if out of room for quads in this batch, render and reset
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
		quadVerticesBuffer.push_back(BatchedVertex {
			_position.x+(SpriteRenderer::quadvertices[i+0]-_anchor.x)*_scale.x, _position.y+(SpriteRenderer::quadvertices[i+1]-_anchor.y)*_scale.y,
			_zIndex-100.0f,
			SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4],
			modulate.x, modulate.y, modulate.z, modulate.w,
			texIndex
			});
	}
	numQuads++;
}
void BatchedSpriteRenderer::renderBatch() {
	if(Engine::instance->ended||!initialized) return;
	if(numQuads==0) return;
	shader->use();
	for(unsigned int i=0; i<numTextures; i++) {
		// equivilent to 'textures[i]->Bind(i);'
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, textures[i]->ID);
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numQuads*4*sizeof(BatchedVertex), &quadVerticesBuffer[0]);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numQuads*6, GL_UNSIGNED_INT, nullptr);
	Engine::instance->curDrawCalls++;
	quadVerticesBuffer.clear();
	numQuads=0;
	textures.clear();
	numTextures=0;
}
BatchedSpriteRenderer::BatchedSpriteRenderer(OrthoCam* _cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), cam(_cam) {
	if(!initialized) return;
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, maxQuadCount*4*sizeof(BatchedVertex), nullptr, GL_DYNAMIC_DRAW);
	// Populates index buffer without allocating memory for thousands of ints
	const unsigned int totalIndices=maxQuadCount*6;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*totalIndices, nullptr, GL_STATIC_DRAW);
	unsigned int* indicesData=static_cast<unsigned int*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));// Maps the buffer to update the data directly
	for(int i=0; i<maxQuadCount; i++) {// Populate the indices directly in the mapped buffer
		for(int j=0; j<6; j++) {
			indicesData[i*6+j]=i*4+SpriteRenderer::quadindices[j];
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(9*sizeof(float)));
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
BatchedSpriteRenderer::~BatchedSpriteRenderer() {
	if(!initialized) return;
	for(BatchedQuadData* data:quads) delete data;
}
BatchedQuadData* BatchedSpriteRenderer::addSprite(const Vector4& modulate, Texture* tex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(Engine::instance->ended||!initialized) return nullptr;
	BatchedQuadData* ptr=new BatchedQuadData(modulate, tex, _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
BatchedQuadData* BatchedSpriteRenderer::addQuad(const Vector4& modulate, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(Engine::instance->ended||!initialized) return nullptr;
	BatchedQuadData* ptr=new BatchedQuadData(modulate, nullptr, _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
void BatchedSpriteRenderer::draw() {
	if(Engine::instance->ended||!initialized) return;
	for(BatchedQuadData* quad:quads) bufferQuad(quad->modulate, quad->tex, quad->getWorldPos(), quad->zIndex, quad->getWorldScale(), quad->anchor);
	renderBatch();
}
#pragma endregion// BatchedSpriteRenderer

#pragma region StaticBatchedSpriteRenderer
void StaticBatchedSpriteRenderer::bufferQuad(const Vector4& modulate, Texture* tex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(Engine::instance->ended||!initialized) return;
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
		quadVerticesBuffer.push_back(BatchedVertex {
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
	if(Engine::instance->ended||!initialized) return;
	if(numQuads==0) return;
	shader->use();
	for(unsigned int i=0; i<numTextures; i++) {
		// equivilent to 'textures[i]->Bind(i);'
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, textures[i]->ID);
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numQuads*4*sizeof(BatchedVertex), &quadVerticesBuffer[0]);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numQuads*6, GL_UNSIGNED_INT, nullptr);
	Engine::instance->curDrawCalls++;
}
StaticBatchedSpriteRenderer::StaticBatchedSpriteRenderer(OrthoCam* _cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/texBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), cam(_cam) {
	if(!initialized) return;
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, maxQuadCount*4*sizeof(BatchedVertex), nullptr, GL_DYNAMIC_DRAW);
	// Populates index buffer without allocating memory for thousands of ints
	const unsigned int totalIndices=maxQuadCount*6;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*totalIndices, nullptr, GL_STATIC_DRAW);
	unsigned int* indicesData=static_cast<unsigned int*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));// Maps the buffer to update the data directly
	for(int i=0; i<maxQuadCount; i++) {// Populate the indices directly in the mapped buffer
		for(int j=0; j<6; j++) {
			indicesData[i*6+j]=i*4+SpriteRenderer::quadindices[j];
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(9*sizeof(float)));
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
StaticBatchedSpriteRenderer::~StaticBatchedSpriteRenderer() {
	if(!initialized) return;
	for(BatchedQuadData* data:quads) delete data;
}
BatchedQuadData* StaticBatchedSpriteRenderer::addSprite(const Vector4& modulate, Texture* tex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(Engine::instance->ended||!initialized) return nullptr;
	if(quads.size()>maxQuadCount) return nullptr;
	BatchedQuadData* ptr=new BatchedQuadData(modulate, tex, _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
BatchedQuadData* StaticBatchedSpriteRenderer::addQuad(const Vector4& modulate, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(Engine::instance->ended||!initialized) return nullptr;
	if(quads.size()>maxQuadCount) return nullptr;
	BatchedQuadData* ptr=new BatchedQuadData(modulate, nullptr, _position, _zIndex, _scale, _anchor);
	quads.push_back(ptr);
	return ptr;
}
void StaticBatchedSpriteRenderer::bind() {
	if(Engine::instance->ended||!initialized) return;
	quadVerticesBuffer.clear();
	numQuads=0;
	textures.clear();
	numTextures=0;
	//quadVerticesBuffer.reserve(std::min(static_cast<unsigned int>(quads.size()), maxQuadCount)*4);// we know there will be 4 vertices per quad so reserve the memory for that many vertices
	for(BatchedQuadData* quad:quads) bufferQuad(quad->modulate, quad->tex, quad->getWorldPos(), quad->zIndex, quad->getWorldScale(), quad->anchor);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numQuads*4*sizeof(BatchedVertex), &quadVerticesBuffer[0]);
}
void StaticBatchedSpriteRenderer::draw() {
	if(Engine::instance->ended||!initialized) return;
	renderBatch();
}
#pragma endregion// StaticBatchedSpriteRenderer

#pragma region BatchedTextRenderer
struct TextRenderer::Character {
	Texture* tex=new Texture();// ID handle of the glyph texture
	Vector2   Size;		       // Size of glyph
	Vector2   Bearing;         // Offset from baseline to left/top of glyph
	float Advance=0.0f;        // Offset to advance to next glyph
};
void BatchedTextRenderer::bufferCharacter(const int& shaderIndex, const float& texIndex, const Vector4& color, const Vector2& _position, const float& _zIndex, const Vector2& _scale) {
	if(Engine::instance->ended||!initialized) return;
	if(numChars[shaderIndex]>=maxCharacterCount) renderBatch(shaderIndex);// if out of room render and reset
	for(unsigned int i=0; i<static_cast<unsigned int>(4*5); i+=5) {
		characterBuffers[shaderIndex].push_back(BatchedVertex {
			_position.x+SpriteRenderer::quadvertices[i+0]*_scale.x, _position.y+SpriteRenderer::quadvertices[i+1]*_scale.y,// x and y
			_zIndex-100.0f,// z
			SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4],// u and v
			color.x, color.y, color.z, color.w,// r, g, b, and a
			texIndex
			});
	}
	numChars[shaderIndex]++;
}
void BatchedTextRenderer::renderBatch(const int& shaderIndex) {
	if(Engine::instance->ended||!initialized) return;
	if(numChars[shaderIndex]==0) return;
	shader->use();
	for(int i=0; i<32; i++) {
		// equivilent to '''textureArrays[shaderIndex][i]->Bind(i);'''
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, textureArrays[shaderIndex][i]->ID);
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(BatchedVertex)*numChars[shaderIndex], &characterBuffers[shaderIndex][0]);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numChars[shaderIndex]*6, GL_UNSIGNED_INT, nullptr);
	Engine::instance->curDrawCalls++;
	characterBuffers[shaderIndex].clear();
	numChars[shaderIndex]=0;
}
BatchedTextRenderer::BatchedTextRenderer(Camera* cam) :
	Renderer2D(new Shader("Shaders/batch.vert", "Shaders/textBatch.frag"), Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {
	if(!initialized) return;
	if(!TextRenderer::characterMapInitialized) {
		if(!TextRenderer::initCharacterMap()) {
			initialized=false;
			Log("Error initializing font \"Fonts/MonocraftBetterBrackets.ttf\"");//error
			Engine::instance->Delete();
			return;
		}
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

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, maxCharacterCount*4*sizeof(BatchedVertex), nullptr, GL_DYNAMIC_DRAW);
	// Populates index buffer without allocating memory for thousands of ints
	const unsigned int totalIndices=maxCharacterCount*6;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*totalIndices, nullptr, GL_STATIC_DRAW);
	unsigned int* indicesData=static_cast<unsigned int*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));// Maps the buffer to update the data directly
	for(int i=0; i<maxCharacterCount; i++) {// Populate the indices directly in the mapped buffer
		for(int j=0; j<6; j++) {
			indicesData[i*6+j]=i*4+SpriteRenderer::quadindices[j];
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(9*sizeof(float)));
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
BatchedTextRenderer::~BatchedTextRenderer() {
	if(!initialized) return;
	for(BatchedTextData* _text:text) delete _text;
}
BatchedTextData* BatchedTextRenderer::addText(const std::string& _text, const Vector4& color, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor) {
	if(Engine::instance->ended||!initialized) return nullptr;
	BatchedTextData* ptr=new BatchedTextData(_text, color, _position, _zIndex, _scale, _anchor);
	text.push_back(ptr);
	return ptr;
}
void BatchedTextRenderer::draw() {
	if(Engine::instance->ended||!initialized) return;
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