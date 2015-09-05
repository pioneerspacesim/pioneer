// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef DUMMY_VERTEXBUFFER_H
#define DUMMY_VERTEXBUFFER_H

#include "graphics/VertexBuffer.h"

namespace Graphics {

namespace Dummy {

class VertexBuffer : public Graphics::VertexBuffer {
public:
	VertexBuffer(const VertexBufferDesc &d) : Graphics::VertexBuffer(d),
	m_buffer(new Uint8[m_desc.numVertices * m_desc.stride])
	{}

	// copies the contents of the VertexArray into the buffer
	virtual bool Populate(const VertexArray &) override { return true; }

	virtual void Bind() {}
	virtual void Release() {}

	virtual void Unmap() override {}

protected:
	virtual Uint8 *MapInternal(BufferMapMode) { return m_buffer.get(); }

private:
	std::unique_ptr<Uint8[]> m_buffer;
};

class IndexBuffer : public Graphics::IndexBuffer {
public:
	IndexBuffer(Uint32 size, BufferUsage bu) : Graphics::IndexBuffer(size, bu),
	m_buffer(new Uint16[size])
	{};

	virtual Uint16 *Map(BufferMapMode) override { return m_buffer.get(); }

	virtual void Unmap() override {}

	virtual void Bind() {}
	virtual void Release() {}

private:
    std::unique_ptr<Uint16[]> m_buffer;
};

// Instance buffer
class InstanceBuffer : public Graphics::InstanceBuffer {
public:
	InstanceBuffer(Uint32 size, BufferUsage hint)
		: Graphics::InstanceBuffer(size, hint), m_data(new matrix4x4f[size])
	{}
	virtual ~InstanceBuffer() {};
	virtual matrix4x4f* Map(BufferMapMode) override { return m_data.get(); }
	virtual void Unmap() override {}

	Uint32 GetSize() const { return m_size; }
	BufferUsage GetUsage() const { return m_usage; }

	virtual void Bind() {}
	virtual void Release() {}

protected:
	std::unique_ptr<matrix4x4f> m_data;
};

} }

#endif
