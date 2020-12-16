

#include "UniformBuffer.h"
#include "graphics/Types.h"
#include "graphics/VertexBuffer.h"

using namespace Graphics::OGL;

// From OpenGL, the minimum alignment of a buffer range binding
static constexpr int MIN_BUFFER_ALIGNMENT_MASK = 256 - 1;

UniformBuffer::UniformBuffer(uint32_t size, BufferUsage usage) :
	Mappable(size)
{
	glGenBuffers(1, &m_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, (usage == BUFFER_USAGE_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::~UniformBuffer()
{
	assert(m_mapMode == BUFFER_MAP_NONE);
	glDeleteBuffers(1, &m_buffer);
}

void UniformBuffer::Unmap()
{
	assert(m_mapMode != BUFFER_MAP_NONE);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::BufferData(const size_t size, void *data)
{
	assert(m_mapMode == BUFFER_MAP_NONE);
	glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::Bind(uint32_t binding)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_buffer);
}

void UniformBuffer::Release()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void *UniformBuffer::MapInternal(BufferMapMode mode)
{
	assert(m_mapMode == BUFFER_MAP_NONE);
	glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
	void *data = glMapBuffer(GL_UNIFORM_BUFFER, (mode == BUFFER_MAP_READ) ? GL_READ_ONLY : GL_WRITE_ONLY);
	m_mapMode = mode;
	return data;
}

UniformLinearBuffer::UniformLinearBuffer(uint32_t size) :
	Mappable(0),
	m_numAllocs(0)
{
	glGenBuffers(1, &m_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	m_capacity = size;
}

UniformLinearBuffer::~UniformLinearBuffer()
{
	assert(m_mapMode == BUFFER_MAP_NONE);
	glDeleteBuffers(1, &m_buffer);
}
void UniformLinearBuffer::Reset()
{
	m_size = 0;
	m_numAllocs = 0;
}

void *UniformLinearBuffer::AllocInternal(uint32_t binding, size_t size)
{
	assert(m_mapMode == BUFFER_MAP_NONE);
	m_mapMode = BUFFER_MAP_WRITE;

	if (size > FreeSize())
		assert(0 && "Attempt to allocate beyond UniformLinearBuffer free size!");

	// Bind the buffer to the specified binding index as well as the GL_UNIFORM_BUFFER_TARGET
	glBindBufferRange(GL_UNIFORM_BUFFER, binding, m_buffer, m_size, size);

	// Map and invalidate the range to (hopefully) avoid implicit synchronization
	// TODO: see if this needs to happen-before the BindBufferRange call...
	void *data = glMapBufferRange(GL_UNIFORM_BUFFER, m_size, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

	// Consume the buffer in increments of aligned size
	m_size += (size + MIN_BUFFER_ALIGNMENT_MASK) & ~MIN_BUFFER_ALIGNMENT_MASK;
	m_numAllocs++;

	return data;
}

void UniformLinearBuffer::Unmap()
{
	assert(m_mapMode == BUFFER_MAP_WRITE);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	m_mapMode = BUFFER_MAP_NONE;
}
