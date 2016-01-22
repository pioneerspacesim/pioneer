// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef OGL_VERTEXBUFFER_H
#define OGL_VERTEXBUFFER_H
#include "OpenGLLibs.h"
#include "graphics/VertexBuffer.h"

namespace Graphics { namespace OGL {

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

	virtual void Unmap() override;

	// copies the contents of the VertexArray into the buffer
	virtual bool Populate(const VertexArray &) override;
	
	virtual void Bind() override;
	virtual void Release() override;

protected:
	virtual Uint8 *MapInternal(BufferMapMode) override;

private:
	GLuint m_vao;
	Uint8 *m_data;
};

class IndexBuffer : public Graphics::IndexBuffer, public GLBufferBase {
public:
	IndexBuffer(Uint32 size, BufferUsage);
	~IndexBuffer();

	virtual Uint32 *Map(BufferMapMode) override;
	virtual void Unmap() override;
	
	virtual void Bind() override;
	virtual void Release() override;

private:
	Uint32 *m_data;
};

// Instance buffer
class InstanceBuffer : public Graphics::InstanceBuffer, public GLBufferBase {
public:
	InstanceBuffer(Uint32 size, BufferUsage);
	virtual ~InstanceBuffer() override;
	virtual matrix4x4f* Map(BufferMapMode) override;
	virtual void Unmap() override;

	virtual void Bind() override;
	virtual void Release() override;

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

#endif // OGL_VERTEXBUFFER_H
