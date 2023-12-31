// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "UniformBuffer.h"

#include "graphics/Types.h"
#include "profiler/Profiler.h"

using Graphics::BufferBinding;
using namespace Graphics::OGL;

// From OpenGL, the minimum alignment of a buffer range binding
static constexpr int MIN_BUFFER_ALIGNMENT_MASK = 256 - 1;

UniformBuffer::UniformBuffer(uint32_t size, BufferUsage usage) :
	Graphics::UniformBuffer(size, usage)
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
	m_mapMode = BUFFER_MAP_NONE;
}

void UniformBuffer::BufferData(const size_t size, void *data)
{
	assert(m_mapMode == BUFFER_MAP_NONE);
	glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::BindRange(uint32_t binding, uint32_t offset, uint32_t size)
{
	glBindBufferRange(GL_UNIFORM_BUFFER, binding, m_buffer, offset, size);
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

// ============================================================================
// Uniform Buffer- Backed Linear Allocator
//

UniformLinearBuffer::UniformLinearBuffer(uint32_t size) :
	UniformBuffer(size, BUFFER_USAGE_DYNAMIC),
	m_lastFlush(0),
	m_numAllocs(0)
{
	glGenBuffers(1, &m_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	m_size = 0;
	m_data.reset(new char[size]);
}

UniformLinearBuffer::~UniformLinearBuffer()
{
	assert(m_mapMode == BUFFER_MAP_NONE);
	glDeleteBuffers(1, &m_buffer);
	m_data.reset();
}

void UniformLinearBuffer::Reset()
{
	m_size = 0;
	m_lastFlush = 0;
	m_numAllocs = 0;

	glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
	glBufferData(GL_UNIFORM_BUFFER, m_capacity, nullptr, GL_DYNAMIC_DRAW);
}

void UniformLinearBuffer::Flush()
{
	PROFILE_SCOPED()
	assert(m_mapMode == BUFFER_MAP_NONE);

	// If we don't have anything new, don't flush
	if (m_lastFlush == m_size)
		return;

	// Bind the buffer to the specified binding index as well as the GL_UNIFORM_BUFFER_TARGET
	glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);

	// Map and invalidate the range to avoid implicit synchronization
	// WARNING: this still causes a client->server CPU-side stall because all GL state commands in flight
	// (e.g. glDeleteBuffers) have to be checked for side-effects before a mapping can be returned.
	// Try to only call this a few times per frame before executing drawing commands.
	// It is still *significantly* faster than glBufferSubData
	void *data = glMapBufferRange(GL_UNIFORM_BUFFER, m_lastFlush, m_size - m_lastFlush,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

	// Copy the data we wrote last into the buffer
	memcpy(data, m_data.get() + m_lastFlush, m_size - m_lastFlush);

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	m_lastFlush = m_size;
}

BufferBinding<UniformBuffer> UniformLinearBuffer::Allocate(void *data, size_t size)
{
	PROFILE_SCOPED()
	assert(m_mapMode == BUFFER_MAP_NONE);
	if (size > FreeSize())
		assert(0 && "Attempt to allocate beyond UniformLinearBuffer free size!");

	uint32_t offset = m_size;
	memcpy(m_data.get() + offset, data, size);

	// Consume the buffer in increments of aligned size
	m_size += (size + MIN_BUFFER_ALIGNMENT_MASK) & ~MIN_BUFFER_ALIGNMENT_MASK;
	m_numAllocs++;

	return { this, offset, uint32_t(size) };
}

void *UniformLinearBuffer::AllocInternal(size_t size, BufferBinding<UniformBuffer> &outBinding)
{
	PROFILE_SCOPED()
	assert(m_mapMode == BUFFER_MAP_NONE);
	m_mapMode = BUFFER_MAP_WRITE;

	if (size > FreeSize())
		assert(0 && "Attempt to allocate beyond UniformLinearBuffer free size!");

	uint32_t offset = m_size;
	// Consume the buffer in increments of aligned size
	m_size += (size + MIN_BUFFER_ALIGNMENT_MASK) & ~MIN_BUFFER_ALIGNMENT_MASK;
	m_numAllocs++;

	outBinding = { this, offset, uint32_t(size) };
	return m_data.get() + offset;
}

void UniformLinearBuffer::Unmap()
{
	assert(m_mapMode == BUFFER_MAP_WRITE);
	m_mapMode = BUFFER_MAP_NONE;
}
