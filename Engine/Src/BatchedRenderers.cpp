#include "BatchedRenderers.h"

#pragma region BatchedSpriteRenderer
void BatchedSpriteRenderer::bufferQuad(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector4& modulate, const float& texIndex) {
	if(engine->ended||!initialized) return;
	if(numQuads>=maxQuadCount) renderBatch();// if out of room render and reset
	for(unsigned int i=0; i<((unsigned int)4*5); i+=5) {
		*quadBufferPtr=BatchedVertex {
			_position.x+SpriteRenderer::quadvertices[i+0]*_scale.x, _position.y+SpriteRenderer::quadvertices[i+1]*_scale.y,
			_zIndex-100.0f,
			SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4],
			modulate.x, modulate.y, modulate.z, modulate.w,
			texIndex
		};
		quadBufferPtr++;
	}
	numQuads++;
}
void BatchedSpriteRenderer::renderBatch() {
	if(engine->ended||!initialized) return;
	if(numQuads==0) return;
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(BatchedVertex)*numQuads, quadBuffer);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numQuads*6, GL_UNSIGNED_INT, nullptr);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, numQuads*4);
	drawCalls++;
	quadBufferPtr=quadBuffer;
	numQuads=0;
}
BatchedSpriteRenderer::BatchedSpriteRenderer(Engine* _engine, Shader* _shader) :
	Renderer2D(_engine, _shader, Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), quadBuffer(nullptr), quadBufferPtr(nullptr) {
	if(!initialized) return;

	quadBuffer=new BatchedVertex[maxQuadCount*4];
	quadBufferPtr=quadBuffer;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, maxQuadCount*4*sizeof(BatchedVertex), nullptr, GL_DYNAMIC_DRAW);
	unsigned int* indices=new unsigned int[maxQuadCount*6];
	for(unsigned int i=0; i<maxQuadCount; i++) {
		indices[i*6+0]=(i*4+SpriteRenderer::quadindices[0]);
		indices[i*6+1]=(i*4+SpriteRenderer::quadindices[1]);
		indices[i*6+2]=(i*4+SpriteRenderer::quadindices[2]);
		indices[i*6+3]=(i*4+SpriteRenderer::quadindices[3]);
		indices[i*6+4]=(i*4+SpriteRenderer::quadindices[4]);
		indices[i*6+5]=(i*4+SpriteRenderer::quadindices[5]);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*maxQuadCount*6, indices, GL_STATIC_DRAW);
	delete[] indices;

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
	delete[] quadBuffer;
}
BatchedQuadData* BatchedSpriteRenderer::addQuad(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector4& modulate, const float& texIndex) {
	if(engine->ended||!initialized) return nullptr;
	BatchedQuadData* ptr=new BatchedQuadData { _position, _zIndex, _scale, modulate, texIndex };
	quads.push_back(ptr);
	return ptr;
}
void BatchedSpriteRenderer::draw() {
	if(engine->ended||!initialized) return;
	drawCalls=0;
	//equivilent to shader->bindTextures();
	shader->use();
	for(unsigned int i=0; i<shader->numTextures; i++) {
		// equivilent to '''textures[i]->Bind(textureIndexes[i]);'''
		glActiveTexture(GL_TEXTURE0+shader->textureIndexes[i]);
		glBindTexture(GL_TEXTURE_2D, shader->textures[i]->ID);
	}
	for(BatchedQuadData* quad:quads) bufferQuad(quad->position, quad->zIndex, quad->scale, quad->modulate, quad->texIndex);
	renderBatch();
}
#pragma endregion// BatchedSpriteRenderer
#pragma region StaticBatchedSpriteRenderer
void StaticBatchedSpriteRenderer::bufferQuad(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector4& modulate, const float& texIndex) {
	if(engine->ended||!initialized) return;
	if(numQuads>=maxQuadCount) return;//dont try to add more than the max, this should not occur though
	for(unsigned int i=0; i<((unsigned int)4*5); i+=5) {
		*quadBufferPtr=BatchedVertex {
			_position.x+SpriteRenderer::quadvertices[i+0]*_scale.x, _position.y+SpriteRenderer::quadvertices[i+1]*_scale.y,
			_zIndex-100.0f,
			SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4],
			modulate.x, modulate.y, modulate.z, modulate.w,
			texIndex
		};
		quadBufferPtr++;
	}
	numQuads++;
}
void StaticBatchedSpriteRenderer::renderBatch() {
	if(engine->ended||!initialized) return;
	if(numQuads==0) return;
	drawCalls++;
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numQuads*6, GL_UNSIGNED_INT, nullptr);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, numQuads*4);
}
StaticBatchedSpriteRenderer::StaticBatchedSpriteRenderer(Engine* _engine, Shader* _shader) :
	Renderer2D(_engine, _shader, Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), quadBuffer(nullptr), quadBufferPtr(nullptr) {
	if(!initialized) return;
	quadBuffer=new BatchedVertex[maxQuadCount*4];
	quadBufferPtr=quadBuffer;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, maxQuadCount*4*sizeof(BatchedVertex), nullptr, GL_DYNAMIC_DRAW);
	unsigned int* indices=new unsigned int[maxQuadCount*6];
	for(unsigned int i=0; i<maxQuadCount; i++) {
		indices[i*6+0]=i*4+1;
		indices[i*6+1]=i*4+3;
		indices[i*6+2]=i*4+2;
		indices[i*6+3]=i*4+2;
		indices[i*6+4]=i*4+0;
		indices[i*6+5]=i*4+1;
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*maxQuadCount*6, indices, GL_STATIC_DRAW);
	delete[] indices;

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
	delete[] quadBuffer;
	for(BatchedQuadData* data:quads) delete data;
}
BatchedQuadData* StaticBatchedSpriteRenderer::addQuad(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector4& modulate, const float& texIndex) {
	if(engine->ended||!initialized) return nullptr;
	BatchedQuadData* ptr=new BatchedQuadData { _position, _zIndex, _scale, modulate, texIndex };
	quads.push_back(ptr);
	return ptr;
}
void StaticBatchedSpriteRenderer::bind() {
	if(engine->ended||!initialized) return;
	quadBufferPtr=quadBuffer;
	numQuads=0;
	for(BatchedQuadData* quad:quads) bufferQuad(quad->position, quad->zIndex, quad->scale, quad->modulate, quad->texIndex);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, maxQuadCount*4*sizeof(BatchedVertex), quadBuffer);
}
void StaticBatchedSpriteRenderer::draw() {
	if(engine->ended||!initialized) return;
	drawCalls+=0;
	//equivilent to shader->bindTextures();
	shader->use();
	for(unsigned int i=0; i<shader->numTextures; i++) {
		// equivilent to '''textures[i]->Bind(textureIndexes[i]);'''
		glActiveTexture(GL_TEXTURE0+shader->textureIndexes[i]);
		glBindTexture(GL_TEXTURE_2D, shader->textures[i]->ID);
	}
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
void BatchedTextRenderer::bufferCharacter(const int& shaderIndex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector4& _color, const float& texIndex) {
	if(engine->ended||!initialized) return;
	if(numChars[shaderIndex]>=maxCharacterCount) renderBatch(shaderIndex);// if out of room render and reset
	for(unsigned int i=0; i<((unsigned int)4*5); i+=5) {
		*characterBufferPtrs[shaderIndex]=BatchedVertex {
			_position.x+SpriteRenderer::quadvertices[i+0]*_scale.x, _position.y+SpriteRenderer::quadvertices[i+1]*_scale.y,// x and y
			_zIndex-100.0f,// z
			SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4],// u and v
			_color.x, _color.y, _color.z, _color.w,
			texIndex
		};
		characterBufferPtrs[shaderIndex]++;
	}
	numChars[shaderIndex]++;
}
void BatchedTextRenderer::renderBatch(const int& shaderIndex) {
	if(engine->ended||!initialized) return;
	if(numChars[shaderIndex]==0) return;
	shader->use();
	for(int i=0; i<32; i++) {
		// equivilent to '''textureArrays[shaderIndex][i]->Bind(i);'''
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, textureArrays[shaderIndex][i]->ID);
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(BatchedVertex)*numChars[shaderIndex], characterBuffers[shaderIndex]);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numChars[shaderIndex]*6, GL_UNSIGNED_INT, nullptr);
	drawCalls++;
	characterBufferPtrs[shaderIndex]=characterBuffers[shaderIndex];
	numChars[shaderIndex]=0;
}
BatchedTextRenderer::BatchedTextRenderer(Engine* _engine, Camera* cam) :
	Renderer2D(_engine, nullptr, Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f),
	characterBuffers({ nullptr, nullptr, nullptr }), characterBufferPtrs({ nullptr, nullptr, nullptr }) {

	initialized=true;
	if(!TextRenderer::characterMapInitialized) {
		if(!TextRenderer::initCharacterMap(engine)) {
			initialized=false;
			Log("Error initializing font \"Fonts/MonocraftBetterBrackets.ttf\"");//error
			engine->Delete();
			return;
		}
	}
	shader=new Shader(engine, "Shaders/batch.vert", "Shaders/textBatch.frag");
	if(!shader->initialized) {
		initialized=false;
		Log("Error initializing BatchedTextRenderer shader");//error
		engine->Delete();
		return;
	}
	shader->setTextureArray("_textures");
	cam->bindShader(shader);
	cam->use();
	for(unsigned int shaderIndex=0; shaderIndex<3; shaderIndex++) {
		// allocate memory for all 4 buffers
		characterBuffers[shaderIndex]=new BatchedVertex[maxCharacterCount*3];
		characterBufferPtrs[shaderIndex]=characterBuffers[shaderIndex];
		// set textures from TextRenderer class into textureArrays
		for(int i=0; i<32; i++) {
			textureArrays[shaderIndex][i]=TextRenderer::Characters[(char)((shaderIndex+1)*32+i)].tex;
		}
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, maxCharacterCount*4*sizeof(BatchedVertex), nullptr, GL_DYNAMIC_DRAW);
	unsigned int* indices=new unsigned int[maxCharacterCount*6];
	for(unsigned int i=0; i<maxCharacterCount; i++) {
		indices[i*6+0]=i*4+SpriteRenderer::quadindices[0];
		indices[i*6+1]=i*4+SpriteRenderer::quadindices[1];
		indices[i*6+2]=i*4+SpriteRenderer::quadindices[2];
		indices[i*6+3]=i*4+SpriteRenderer::quadindices[3];
		indices[i*6+4]=i*4+SpriteRenderer::quadindices[4];
		indices[i*6+5]=i*4+SpriteRenderer::quadindices[5];
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*maxCharacterCount*6, indices, GL_STATIC_DRAW);
	delete[] indices;

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
	for(short unsigned int shaderIndex=0; shaderIndex<3; shaderIndex++) {
		delete[] characterBuffers[shaderIndex];
	}
	for(BatchedTextData* _text:text) delete _text;
}
BatchedTextData* BatchedTextRenderer::addText(const std::string& _text, const Vector4& _color, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor) {
	if(engine->ended||!initialized) return nullptr;
	BatchedTextData* ptr=new BatchedTextData { _text, _color, _position, _zIndex, _scale, _anchor };
	text.push_back(ptr);
	return ptr;
}
void BatchedTextRenderer::draw() {
	if(engine->ended||!initialized) return;
	drawCalls=0;

	for(BatchedTextData* _text:text) {
		// iterate through all characters to find scale of full text block
		Vector2 tmpScale=Vector2(0.0f, 9.0f);
		float curX=0.0f;
		const char* cstr=_text->text.c_str();
		int len=(int)_text->text.length();
		for(int c=0;c!=len;c++) {
			char charC=cstr[c];
			if(charC=='\n') {
				if(curX>tmpScale.x)tmpScale.x=curX;
				curX=0.0f; tmpScale.y+=9.0f;
				continue;
			}
			TextRenderer::Character ch=TextRenderer::Characters[(int)charC];
			if(charC==' ') { curX+=1.0f+ch.Advance; continue; }// skip one space and continue
			else if(charC=='\t') { curX+=1.0f+ch.Advance*4.0f; continue; }//4 character spaces
			curX+=1.0f+ch.Advance;
		}
		if(curX>tmpScale.x)tmpScale.x=curX;
		Vector2 offset=_text->position-Vector2(tmpScale.x*(_text->anchor.x+0.5f), tmpScale.y*(_text->anchor.y-0.5f));
		// iterate through all characters and add to buffer
		float x=0.0f;
		float y=0.0f;
		for(int c=0;c!=len;c++) {
			char charC=cstr[c];
			int intC=(int)charC;
			if(charC=='\n') { x=0.0f;y-=9.0f; continue; } else if(charC=='\r') { x=0.0f; continue; }
			TextRenderer::Character ch=TextRenderer::Characters[intC];
			if(charC==' ') { x+=1.0f+ch.Advance; continue; }// skip one space and continue
			else if(charC=='\t') { x+=1.0f+ch.Advance*4.0f; continue; }//4 character spaces
			bufferCharacter(
				((intC)/32)-1,
				(offset+Vector2(x, y)+ch.Bearing-Vector2(_text->anchor.x*ch.Size.x, (ch.Size.y)/2.0f+9.0f))*_text->scale,
				_text->zIndex,
				ch.Size*_text->scale,
				_text->color,
				(float)((intC)%32)
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