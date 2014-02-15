// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GL2_VERTEXBUFFER_H
#define GL2_VERTEXBUFFER_H
#include "graphics/VertexBuffer.h"

namespace Graphics { namespace GL2 {

class VertexBuffer : public Graphics::VertexBuffer {
public:
	VertexBuffer(const VertexBufferDesc&);
	~VertexBuffer();

	virtual void Unmap() override;
	void SetAttribPointers();
	void UnsetAttribPointers();

	GLuint GetBuffer() const { return m_buffer; }

protected:
	virtual Uint8 *MapInternal(BufferMapMode) override;

private:
	GLuint m_buffer;
	Uint8 *m_data;
};

class IndexBuffer : public Graphics::IndexBuffer {
public:
	IndexBuffer(Uint32 size, BufferUsage);
	~IndexBuffer();

	virtual Uint16 *Map(BufferMapMode) override;
	virtual void Unmap() override;

	GLuint GetBuffer() const { return m_buffer; }

private:
	GLuint m_buffer;
	Uint16 *m_data;
};

} }

#endif // GL2_VERTEXBUFFER_H
