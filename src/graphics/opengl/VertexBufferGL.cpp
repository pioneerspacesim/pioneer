// Copyright � 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/opengl/VertexBufferGL.h"
#include "SDL_stdinc.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/opengl/OpenGLLibs.h"
#include "graphics/opengl/RendererGL.h"

#include "profiler/Profiler.h"

#include <algorithm>

namespace Graphics {
	namespace OGL {

		static GLuint is_attr_normalized(VertexAttribFormat fmt)
		{
			return fmt == ATTRIB_FORMAT_UBYTE4 ? GL_TRUE : GL_FALSE;
		}

		static GLuint get_num_locations(VertexAttribFormat fmt)
		{
			switch (fmt) {
			case ATTRIB_FORMAT_MAT3x4:
			case ATTRIB_FORMAT_MAT4x4:
				return 4;
			case ATTRIB_FORMAT_MAT3:
				return 3;
			default:
				return 1;
			}
		}

		static GLint get_num_components(VertexAttribFormat fmt)
		{
			switch (fmt) {
			case ATTRIB_FORMAT_FLOAT:
				return 1;
			case ATTRIB_FORMAT_FLOAT2:
				return 2;
			case ATTRIB_FORMAT_FLOAT3:
				return 3;
			case ATTRIB_FORMAT_FLOAT4:
			case ATTRIB_FORMAT_UBYTE4:
				return 4;
			case ATTRIB_FORMAT_MAT3:
			case ATTRIB_FORMAT_MAT3x4:
				return 3;
			case ATTRIB_FORMAT_MAT4x4:
				return 4;
			default:
				assert(false);
				return 0;
			}
		}

		static GLenum get_component_type(VertexAttribFormat fmt)
		{
			switch (fmt) {
			case ATTRIB_FORMAT_UBYTE4:
				return GL_UNSIGNED_BYTE;
			case ATTRIB_FORMAT_FLOAT:
			case ATTRIB_FORMAT_FLOAT2:
			case ATTRIB_FORMAT_FLOAT3:
			case ATTRIB_FORMAT_FLOAT4:
			case ATTRIB_FORMAT_MAT3:
			case ATTRIB_FORMAT_MAT3x4:
			case ATTRIB_FORMAT_MAT4x4:
			default:
				return GL_FLOAT;
			}
		}

		static GLuint get_component_size(VertexAttribFormat fmt)
		{
			switch (fmt) {
			case ATTRIB_FORMAT_UBYTE4:
				return sizeof(uint32_t);
			case ATTRIB_FORMAT_FLOAT:
				return sizeof(float);
			case ATTRIB_FORMAT_FLOAT2:
				return sizeof(float) * 2;
			case ATTRIB_FORMAT_FLOAT3:
				return sizeof(float) * 3;
			case ATTRIB_FORMAT_FLOAT4:
				return sizeof(float) * 4;
			case ATTRIB_FORMAT_MAT3:
				return sizeof(float) * 3;
			case ATTRIB_FORMAT_MAT3x4:
				return sizeof(float) * 4;
			case ATTRIB_FORMAT_MAT4x4:
				return sizeof(float) * 4;
			default:
				return 4;
			}
		}

		static GLenum get_buffer_usage(BufferUsage use)
		{
			switch (use) {
			case BUFFER_USAGE_STATIC:
				return GL_STATIC_DRAW;
			case BUFFER_USAGE_DYNAMIC:
				return GL_DYNAMIC_DRAW;
			default:
				assert(false);
				return 0;
			}
		}

		VertexBuffer::VertexBuffer(BufferUsage usage, uint32_t numVertices, uint32_t stride) :
			Graphics::VertexBuffer(usage, numVertices, stride),
			m_mapStart(0),
			m_mapLength(0)
		{
			PROFILE_SCOPED()

			assert(m_stride > 0);

			//Allocate GL buffer with undefined contents
			//Critical optimisation for some architectures in cases where buffer is created and written in the same frame
			glGenBuffers(1, &m_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
			const Uint32 dataSize = numVertices * m_stride;
			glBufferData(GL_ARRAY_BUFFER, dataSize, 0, get_buffer_usage(usage));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Allocate client data store for dynamic buffers
			if (usage != BUFFER_USAGE_STATIC) {
				m_data = new Uint8[dataSize];
				memset(m_data, 0, dataSize);
			} else
				m_data = nullptr;
		}

		VertexBuffer::~VertexBuffer()
		{
			glDeleteBuffers(1, &m_buffer);
			delete[] m_data;
		}

		uint8_t *VertexBuffer::MapInternal(BufferMapMode mode)
		{
			PROFILE_SCOPED()
			assert(mode != BUFFER_MAP_NONE);	  //makes no sense
			assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
			m_mapMode = mode;
			if (m_usage == BUFFER_USAGE_STATIC) {
				glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
				if (mode == BUFFER_MAP_READ)
					return reinterpret_cast<Uint8 *>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY));
				else if (mode == BUFFER_MAP_WRITE)
					return reinterpret_cast<Uint8 *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
			}

			return m_data;
		}

		void VertexBuffer::Unmap()
		{
			PROFILE_SCOPED()
			assert(m_mapMode != BUFFER_MAP_NONE); //not currently mapped

			if (m_usage == BUFFER_USAGE_STATIC) {
				glUnmapBuffer(GL_ARRAY_BUFFER);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			} else {
				if (m_mapMode == BUFFER_MAP_WRITE) {
					const GLsizei dataSize = m_size * m_stride;
					glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
					glBufferData(GL_ARRAY_BUFFER, dataSize, 0, get_buffer_usage(m_usage));
					glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_data);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}
			}

			m_mapMode = BUFFER_MAP_NONE;
			m_written = true;
		}

		uint8_t *VertexBuffer::MapRangeInternal(BufferMapMode mode, size_t start, size_t range)
		{
			PROFILE_SCOPED()
			assert(mode != BUFFER_MAP_NONE);
			assert(m_mapMode == BUFFER_MAP_NONE);
			assert(m_mapLength == 0);
			assert(start + range <= m_capacity * m_stride);

			m_mapMode = mode;
			m_mapStart = start;
			m_mapLength = range;

			if (m_usage == BUFFER_USAGE_STATIC) {
				glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
				return reinterpret_cast<uint8_t *>(glMapBufferRange(GL_ARRAY_BUFFER, start, range, mode == BUFFER_MAP_READ ? GL_READ_ONLY : GL_WRITE_ONLY));
			}

			return m_data + start;
		}

		void VertexBuffer::UnmapRange(bool flush)
		{
			PROFILE_SCOPED()
			assert(m_mapMode != BUFFER_MAP_NONE);
			assert(m_mapLength != 0);

			if (m_usage == BUFFER_USAGE_STATIC) {
				glUnmapBuffer(GL_ARRAY_BUFFER);
			} else if (m_mapMode == BUFFER_MAP_WRITE && flush) {
				glBufferSubData(GL_ARRAY_BUFFER, m_mapStart, m_mapLength, m_data + m_mapStart);
			}

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			m_mapMode = BUFFER_MAP_NONE;
			m_mapStart = 0;
			m_mapLength = 0;
		}

		void VertexBuffer::FlushRange(size_t start, size_t range)
		{
			PROFILE_SCOPED()
			assert(m_mapMode == BUFFER_MAP_NONE);
			assert(m_usage == BUFFER_USAGE_DYNAMIC);
			assert(start + range <= m_capacity * m_stride);

			glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
			glBufferSubData(GL_ARRAY_BUFFER, start, range, m_data + start);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void VertexBuffer::BufferData(const size_t size, void *data)
		{
			PROFILE_SCOPED()
			assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
			if (m_usage == BUFFER_USAGE_DYNAMIC) {
				glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
				glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), static_cast<GLvoid *>(data), GL_DYNAMIC_DRAW);
			}
		}

		void VertexBuffer::Reset()
		{
			PROFILE_SCOPED()
			assert(m_usage == BUFFER_USAGE_DYNAMIC);
			assert(m_mapMode == BUFFER_MAP_NONE);

			glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
			glBufferData(GL_ARRAY_BUFFER, m_capacity * m_stride, nullptr, get_buffer_usage(m_usage));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void VertexBuffer::Bind()
		{
			assert(m_written);
			glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		}

		void VertexBuffer::Release()
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		// ------------------------------------------------------------
		IndexBuffer::IndexBuffer(Uint32 size, BufferUsage hint, IndexBufferSize elem) :
			Graphics::IndexBuffer(size, hint, elem),
			m_data(nullptr),
			m_data16(nullptr)
		{
			const GLenum usage = (hint == BUFFER_USAGE_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
			const GLuint gl_size = (elem == INDEX_BUFFER_16BIT ? sizeof(Uint16) : sizeof(Uint32)) * m_size;
			glGenBuffers(1, &m_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, gl_size, 0, usage);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			if (GetUsage() != BUFFER_USAGE_STATIC) {
				if (elem == INDEX_BUFFER_16BIT) {
					m_data16 = new Uint16[size];
					memset(m_data16, 0, sizeof(Uint16) * size);
				} else {
					m_data = new Uint32[size];
					memset(m_data, 0, sizeof(Uint32) * size);
				}
			}
		}

		IndexBuffer::~IndexBuffer()
		{
			glDeleteBuffers(1, &m_buffer);
			if (m_elemSize == INDEX_BUFFER_16BIT)
				delete[] m_data16;
			else
				delete[] m_data;
		}

		Uint32 *IndexBuffer::Map(BufferMapMode mode)
		{
			assert(mode != BUFFER_MAP_NONE);	  //makes no sense
			assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
			m_mapMode = mode;
			if (GetUsage() == BUFFER_USAGE_STATIC) {
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
				if (mode == BUFFER_MAP_READ)
					return reinterpret_cast<Uint32 *>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY));
				else if (mode == BUFFER_MAP_WRITE)
					return reinterpret_cast<Uint32 *>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
			}

			return m_data;
		}

		Uint16 *IndexBuffer::Map16(BufferMapMode mode)
		{
			assert(mode != BUFFER_MAP_NONE);	  //makes no sense
			assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
			m_mapMode = mode;
			if (GetUsage() == BUFFER_USAGE_STATIC) {
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
				if (mode == BUFFER_MAP_READ)
					return reinterpret_cast<Uint16 *>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY));
				else if (mode == BUFFER_MAP_WRITE)
					return reinterpret_cast<Uint16 *>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
			}

			return m_data16;
		}

		void IndexBuffer::Unmap()
		{
			assert(m_mapMode != BUFFER_MAP_NONE); //not currently mapped

			if (GetUsage() == BUFFER_USAGE_STATIC) {
				glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			} else {
				if (m_mapMode == BUFFER_MAP_WRITE) {
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
					if (m_elemSize == INDEX_BUFFER_16BIT) {
						glBufferData(GL_ARRAY_BUFFER, sizeof(Uint16) * m_size, 0, GL_DYNAMIC_DRAW);
						glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(Uint16) * m_size, m_data16);
					} else {
						glBufferData(GL_ARRAY_BUFFER, sizeof(Uint32) * m_size, 0, GL_DYNAMIC_DRAW);
						glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(Uint32) * m_size, m_data);
					}
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				}
			}

			m_mapMode = BUFFER_MAP_NONE;
			m_written = true;
		}

		void IndexBuffer::BufferData(const size_t size, void *data)
		{
			PROFILE_SCOPED()
			assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
			if (GetUsage() == BUFFER_USAGE_DYNAMIC) {
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), static_cast<GLvoid *>(data), GL_DYNAMIC_DRAW);
			}
		}

		void IndexBuffer::Bind()
		{
			assert(m_written);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
		}

		void IndexBuffer::Release()
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		// ------------------------------------------------------------
		InstanceBuffer::InstanceBuffer(Uint32 size, BufferUsage hint) :
			Graphics::InstanceBuffer(size, hint)
		{
			assert(size > 0);

			const GLenum usage = (hint == BUFFER_USAGE_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
			glGenBuffers(1, &m_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(matrix4x4f) * m_size, 0, usage);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			if (GetUsage() != BUFFER_USAGE_STATIC) {
				m_data.reset(new matrix4x4f[size]);
				std::fill_n(m_data.get(), size, matrix4x4f(0.f));
			}
		}

		InstanceBuffer::~InstanceBuffer()
		{
			glDeleteBuffers(1, &m_buffer);
		}

		matrix4x4f *InstanceBuffer::Map(BufferMapMode mode)
		{
			PROFILE_SCOPED()
			assert(mode != BUFFER_MAP_NONE);	  //makes no sense
			assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
			m_mapMode = mode;
			if (GetUsage() == BUFFER_USAGE_STATIC) {
				glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
				if (mode == BUFFER_MAP_READ)
					return reinterpret_cast<matrix4x4f *>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY));
				else if (mode == BUFFER_MAP_WRITE)
					return reinterpret_cast<matrix4x4f *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
			}

			return m_data.get();
		}

		void InstanceBuffer::Unmap()
		{
			PROFILE_SCOPED()
			assert(m_mapMode != BUFFER_MAP_NONE); //not currently mapped

			if (GetUsage() == BUFFER_USAGE_STATIC) {
				glUnmapBuffer(GL_ARRAY_BUFFER);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			} else {
				if (m_mapMode == BUFFER_MAP_WRITE) {
					glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
					glBufferData(GL_ARRAY_BUFFER, sizeof(matrix4x4f) * m_size, 0, GL_DYNAMIC_DRAW);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(matrix4x4f) * m_size, m_data.get());
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}
			}

			m_mapMode = BUFFER_MAP_NONE;
			m_written = true;
		}

		void InstanceBuffer::Bind()
		{
			assert(m_written);

			// used to pass a matrix4x4f in, however each attrib array is max size of (GLSL) vec4 so must enable 4 arrays
			glEnableVertexAttribArray(INSTOFFS_MAT0);
			glEnableVertexAttribArray(INSTOFFS_MAT1);
			glEnableVertexAttribArray(INSTOFFS_MAT2);
			glEnableVertexAttribArray(INSTOFFS_MAT3);

			glBindVertexBuffer(1, m_buffer, 0, sizeof(float) * 16);
		}

		void InstanceBuffer::Release()
		{
			glBindVertexBuffer(1, 0, 0, sizeof(float) * 16);

			// see enable comment above
			glDisableVertexAttribArray(INSTOFFS_MAT0);
			glDisableVertexAttribArray(INSTOFFS_MAT1);
			glDisableVertexAttribArray(INSTOFFS_MAT2);
			glDisableVertexAttribArray(INSTOFFS_MAT3);
		}

		MeshObject::MeshObject(const Graphics::VertexFormatDesc &fmt, Graphics::VertexBuffer *vtx, Graphics::IndexBuffer *idx) :
			m_vtxBuffer(static_cast<OGL::VertexBuffer *>(vtx)),
			m_idxBuffer(static_cast<OGL::IndexBuffer *>(idx)),
			m_format(fmt)
		{
			assert(m_vtxBuffer.Valid());

			m_vao = BuildVAOFromDesc(fmt);
			glBindVertexArray(m_vao);

			glBindVertexBuffer(0, m_vtxBuffer->GetBuffer(), 0, fmt.bindings[0].stride);
			if (m_idxBuffer)
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxBuffer->GetBuffer());

			glBindVertexArray(0);
		}

		MeshObject::~MeshObject()
		{
			glDeleteVertexArrays(1, &m_vao);
		}

		void MeshObject::Bind()
		{
			glBindVertexArray(m_vao);
		}

		void MeshObject::Release()
		{
			glBindVertexArray(0);
		}

		GLuint BuildVAOFromDesc(const Graphics::VertexFormatDesc desc)
		{
			GLuint vao = 0;
			// Create the VAOs
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			// Setup VertexArrayObject pointers (vertex input descriptor state)
			// FORMAT_NONE is the implicit null-terminator in the attribs list
			for (size_t i = 0; i < MAX_ATTRIBS && desc.attribs[i].format; i++) {
				const VertexAttribDesc &attr = desc.attribs[i];

				// Passing matrix data requires configuring multiple contiguous locations
				size_t num_locations = get_num_locations(attr.format);

				for (size_t l = 0; l < num_locations; l++) {
					GLuint loc = attr.location + l;

					// Enable the attribute at that location
					glEnableVertexAttribArray(loc);
					// Tell OpenGL what the array contains
					glVertexAttribFormat(loc, get_num_components(attr.format), get_component_type(attr.format), is_attr_normalized(attr.format), attr.offset + get_component_size(attr.format) * l);
					// Point the attribute to the correct input buffer
					glVertexAttribBinding(loc, attr.binding);
				}
			}

			CHECKERRORS();

			// Setup buffer binding divisors for instance buffers
			// The list of bindings is null-terminated by the presence of an enabled==false binding
			for (size_t i = 0; i < MAX_BINDINGS && desc.bindings[i].enabled; i++) {
				const VertexBindingDesc &binding = desc.bindings[i];

				glVertexBindingDivisor(i, binding.rate == ATTRIB_RATE_INSTANCE ? 1 : 0);
			}

			CHECKERRORS();

			glBindVertexArray(0);
			return vao;
		}

	} //namespace OGL
} //namespace Graphics
