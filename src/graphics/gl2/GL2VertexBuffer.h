// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GL2_VERTEXBUFFER_H
#define GL2_VERTEXBUFFER_H
#include "OpenGLLibs.h"
#include "graphics/VertexBuffer.h"

namespace Graphics { namespace GL2 {

class GLBufferBase {
public:
	GLuint GetBuffer() const { return m_buffer; }

protected:
	GLuint m_buffer;
};

class VertexBuffer : public Graphics::VertexBuffer, public GLBufferBase {
public:
	VertexBuffer(const VertexBufferDesc&);
	~VertexBuffer();

	virtual void Unmap() override final;

	// copies the contents of the VertexArray into the buffer
	virtual bool Populate(const VertexArray &) override final;

	// change the buffer data without mapping
	virtual void BufferData(const size_t, void*) override final;

	virtual void Bind() override final;
	virtual void Release() override final;

protected:
	virtual Uint8 *MapInternal(BufferMapMode) override final;

private:
	GLuint m_vao;
	Uint8 *m_data;
};

class IndexBuffer : public Graphics::IndexBuffer, public GLBufferBase {
public:
	IndexBuffer(Uint32 size, BufferUsage);
	~IndexBuffer();

	virtual Uint32 *Map(BufferMapMode) override final;
	virtual void Unmap() override final;

	// change the buffer data without mapping
	virtual void BufferData(const size_t, void*) override final;

	virtual void Bind() override final;
	virtual void Release() override final;

private:
	Uint32 *m_data;
};

// Instance buffer
class InstanceBuffer : public Graphics::InstanceBuffer, public GLBufferBase {
public:
	InstanceBuffer(Uint32 size, BufferUsage);
	virtual ~InstanceBuffer() override final;
	virtual matrix4x4f* Map(BufferMapMode) override final;
	virtual void Unmap() override final;

	virtual void Bind() override final;
	virtual void Release() override final;

protected:
	enum InstOffs {
		INSTOFFS_MAT0 = 5, // these value must match those of a_transform within data/shaders/opengl/attributes.glsl
		INSTOFFS_MAT1 = 6,
		INSTOFFS_MAT2 = 7,
		INSTOFFS_MAT3 = 8
	};
	std::unique_ptr<matrix4x4f> m_data;
};

} }

#endif // GL2_VERTEXBUFFER_H
