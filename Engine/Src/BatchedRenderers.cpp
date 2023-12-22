#include "BatchedRenderers.h"

#pragma region BatchedSpriteRenderer
void BatchedSpriteRenderer::bufferQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex) {
	if(engine->ended||!initialized) return;
	if(numQuads>=maxQuadCount) renderBatch();// if out of room render and reset
	for(unsigned int i=0; i<((unsigned int)4*5); i+=5) {
		*dataBufferPtr=BatchedVertex {
			position.x+SpriteRenderer::quadvertices[i+0]*scale.x, position.y+SpriteRenderer::quadvertices[i+1]*scale.y,
			zIndex-100.0f,
			SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4],
			modulate.x, modulate.y, modulate.z, modulate.w,
			texIndex
		};
		dataBufferPtr++;
	}
	numQuads++;
}
void BatchedSpriteRenderer::renderBatch() {
	if(engine->ended||!initialized) return;
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(BatchedVertex)*numQuads, dataBuffer);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numQuads*6, GL_UNSIGNED_INT, nullptr);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, numQuads*4);
	drawCalls++;
	dataBufferPtr=dataBuffer;
	numQuads=0;
}
BatchedSpriteRenderer::BatchedSpriteRenderer(Engine* _engine, Shader* _shader) :
	Renderer2D(_engine, _shader, Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), dataBuffer(nullptr), dataBufferPtr(nullptr) {
	if(!initialized) return;

	dataBuffer=new BatchedVertex[maxQuadCount*4];
	dataBufferPtr=dataBuffer;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, maxQuadCount*4*sizeof(BatchedVertex), nullptr, GL_DYNAMIC_DRAW);
	unsigned int* indices=new unsigned int[maxQuadCount*6];
	for(int i=0; i<maxQuadCount; i++) {
		indices[i*6+0]=((unsigned int)i*4+SpriteRenderer::quadindices[0]);
		indices[i*6+1]=((unsigned int)i*4+SpriteRenderer::quadindices[1]);
		indices[i*6+2]=((unsigned int)i*4+SpriteRenderer::quadindices[2]);
		indices[i*6+3]=((unsigned int)i*4+SpriteRenderer::quadindices[3]);
		indices[i*6+4]=((unsigned int)i*4+SpriteRenderer::quadindices[4]);
		indices[i*6+5]=((unsigned int)i*4+SpriteRenderer::quadindices[5]);
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
	delete[] dataBuffer;
}
void BatchedSpriteRenderer::addQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex) {
	if(engine->ended||!initialized) return;
	quads.push_back(QuadData { position, zIndex, scale, texIndex, modulate });
}
void BatchedSpriteRenderer::draw() {
	if(engine->ended||!initialized) return;
	drawCalls=0;
	shader->bindTextures();
	for(const QuadData& quad:quads) bufferQuad(quad.position, quad.zIndex, quad.scale, quad.modulate, quad.texIndex);
	renderBatch();
}
#pragma endregion// BatchedSpriteRenderer
#pragma region StaticBatchedSpriteRenderer
void StaticBatchedSpriteRenderer::bufferQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex) {
	if(engine->ended||!initialized) return;
	if(numQuads>=maxQuadCount) return;//dont try to add more than the max, this should not occur though
	for(unsigned int i=0; i<((unsigned int)4*5); i+=5) {
		*dataBufferPtr=BatchedVertex {
			position.x+SpriteRenderer::quadvertices[i+0]*scale.x, position.y+SpriteRenderer::quadvertices[i+1]*scale.y,
			zIndex-100.0f,
			SpriteRenderer::quadvertices[i+3], SpriteRenderer::quadvertices[i+4],
			modulate.x, modulate.y, modulate.z, modulate.w,
			texIndex
		};
		dataBufferPtr++;
	}
	numQuads++;
}
void StaticBatchedSpriteRenderer::renderBatch() {
	if(engine->ended||!initialized) return;
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numQuads*6, GL_UNSIGNED_INT, nullptr);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, numQuads*4);
}
StaticBatchedSpriteRenderer::StaticBatchedSpriteRenderer(Engine* _engine, Shader* _shader) :
	Renderer2D(_engine, _shader, Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), dataBuffer(nullptr), dataBufferPtr(nullptr) {
	if(!initialized) return;
	dataBuffer=new BatchedVertex[maxQuadCount*4];
	dataBufferPtr=dataBuffer;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, maxQuadCount*4*sizeof(BatchedVertex), nullptr, GL_DYNAMIC_DRAW);
	unsigned int* indices=new unsigned int[maxQuadCount*6];
	for(int i=0; i<maxQuadCount; i++) {
		indices[i*6+0]=((unsigned int)i*4+1);
		indices[i*6+1]=((unsigned int)i*4+3);
		indices[i*6+2]=((unsigned int)i*4+2);
		indices[i*6+3]=((unsigned int)i*4+2);
		indices[i*6+4]=((unsigned int)i*4+0);
		indices[i*6+5]=((unsigned int)i*4+1);
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
StaticBatchedSpriteRenderer::~StaticBatchedSpriteRenderer() { delete[] dataBuffer; }
void StaticBatchedSpriteRenderer::addQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex) {
	if(engine->ended||!initialized) return;
	quads.push_back(QuadData { position, zIndex, scale, texIndex, modulate });
}
void StaticBatchedSpriteRenderer::bind() {
	if(engine->ended||!initialized) return;
	dataBufferPtr=dataBuffer;
	numQuads=0;
	for(const QuadData& quad:quads) bufferQuad(quad.position, quad.zIndex, quad.scale, quad.modulate, quad.texIndex);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, maxQuadCount*4*sizeof(BatchedVertex), dataBuffer);
}
void StaticBatchedSpriteRenderer::draw() {
	if(engine->ended||!initialized) return;
	shader->bindTextures();
	renderBatch();
}
#pragma endregion// StaticBatchedSpriteRenderer