// Copyright � 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/opengl/VertexBufferGL.h"
#include "SDL_stdinc.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "graphics/opengl/RendererGL.h"
#include "utils.h"
#include <algorithm>

namespace Graphics {
	namespace OGL {

		static GLuint get_attrib_index(VertexAttrib semantic)
		{
			switch (semantic) {
			case ATTRIB_POSITION: return 0;
			case ATTRIB_POSITION2D: return 0;
			case ATTRIB_NORMAL: return 1;
			case ATTRIB_DIFFUSE: return 2;
			case ATTRIB_UV0: return 3;
			case ATTRIB_TANGENT: return 4;
			default:
				assert(false);
				return 0;
			}
		}

		static GLuint is_attr_normalized(VertexAttrib semantic)
		{
			return semantic == ATTRIB_DIFFUSE ? GL_TRUE : GL_FALSE;
		}

		static GLint get_num_components(VertexAttribFormat fmt)
		{
			switch (fmt) {
			case ATTRIB_FORMAT_FLOAT2:
				return 2;
			case ATTRIB_FORMAT_FLOAT3:
				return 3;
			case ATTRIB_FORMAT_FLOAT4:
			case ATTRIB_FORMAT_UBYTE4:
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
			case ATTRIB_FORMAT_FLOAT2:
			case ATTRIB_FORMAT_FLOAT3:
			case ATTRIB_FORMAT_FLOAT4:
			default:
				return GL_FLOAT;
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

		VertexBuffer::VertexBuffer(const VertexBufferDesc &desc, size_t stateHash) :
			Graphics::VertexBuffer(desc),
			m_vertexStateHash(stateHash)
		{
			PROFILE_SCOPED()

			m_desc.CalculateOffsets();
			assert(m_desc.stride > 0);

			//Allocate GL buffer with undefined contents
			//Critical optimisation for some architectures in cases where buffer is created and written in the same frame
			glGenBuffers(1, &m_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
			const Uint32 dataSize = m_desc.numVertices * m_desc.stride;
			glBufferData(GL_ARRAY_BUFFER, dataSize, 0, get_buffer_usage(m_desc.usage));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Allocate client data store for dynamic buffers
			if (GetDesc().usage != BUFFER_USAGE_STATIC) {
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

		Uint8 *VertexBuffer::MapInternal(BufferMapMode mode)
		{
			PROFILE_SCOPED()
			assert(mode != BUFFER_MAP_NONE);	  //makes no sense
			assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
			m_mapMode = mode;
			if (GetDesc().usage == BUFFER_USAGE_STATIC) {
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

			if (GetDesc().usage == BUFFER_USAGE_STATIC) {
				glUnmapBuffer(GL_ARRAY_BUFFER);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			} else {
				if (m_mapMode == BUFFER_MAP_WRITE) {
					const GLsizei dataSize = m_desc.numVertices * m_desc.stride;
					glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
					glBufferData(GL_ARRAY_BUFFER, dataSize, 0, get_buffer_usage(m_desc.usage));
					glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_data);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}
			}

			m_mapMode = BUFFER_MAP_NONE;
			m_written = true;
		}

#pragma pack(push, 4)
		struct PosUVVert {
			vector3f pos;
			vector2f uv;
		};

		struct PosNormVert {
			vector3f pos;
			vector3f norm;
		};

		struct PosColVert {
			vector3f pos;
			Color4ub col;
		};

		struct PosVert {
			vector3f pos;
		};

		struct PosColUVVert {
			vector3f pos;
			Color4ub col;
			vector2f uv;
		};

		struct PosNormUVVert {
			vector3f pos;
			vector3f norm;
			vector2f uv;
		};

		struct PosNormColVert {
			vector3f pos;
			vector3f norm;
			Color4ub col;
		};
#pragma pack(pop)

		static inline void CopyPosNorm(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
		{
			PosNormVert *vtxPtr = vb->Map<PosNormVert>(Graphics::BUFFER_MAP_WRITE);
			assert(vb->GetDesc().stride == sizeof(PosNormVert));
			for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
				vtxPtr[i].pos = va.position[i];
				vtxPtr[i].norm = va.normal[i];
			}
			vb->Unmap();
		}

		static inline void CopyPosUV0(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
		{
			PosUVVert *vtxPtr = vb->Map<PosUVVert>(Graphics::BUFFER_MAP_WRITE);
			assert(vb->GetDesc().stride == sizeof(PosUVVert));
			for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
				vtxPtr[i].pos = va.position[i];
				vtxPtr[i].uv = va.uv0[i];
			}
			vb->Unmap();
		}

		static inline void CopyPosCol(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
		{
			PosColVert *vtxPtr = vb->Map<PosColVert>(Graphics::BUFFER_MAP_WRITE);
			assert(vb->GetDesc().stride == sizeof(PosColVert));
			for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
				vtxPtr[i].pos = va.position[i];
				vtxPtr[i].col = va.diffuse[i];
			}
			vb->Unmap();
		}

		static inline void CopyPos(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
		{
			PosVert *vtxPtr = vb->Map<PosVert>(Graphics::BUFFER_MAP_WRITE);
			assert(vb->GetDesc().stride == sizeof(PosVert));
			for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
				vtxPtr[i].pos = va.position[i];
			}
			vb->Unmap();
		}

		static inline void CopyPosColUV0(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
		{
			PosColUVVert *vtxPtr = vb->Map<PosColUVVert>(Graphics::BUFFER_MAP_WRITE);
			assert(vb->GetDesc().stride == sizeof(PosColUVVert));
			for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
				vtxPtr[i].pos = va.position[i];
				vtxPtr[i].col = va.diffuse[i];
				vtxPtr[i].uv = va.uv0[i];
			}
			vb->Unmap();
		}

		static inline void CopyPosNormUV0(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
		{
			PosNormUVVert *vtxPtr = vb->Map<PosNormUVVert>(Graphics::BUFFER_MAP_WRITE);
			assert(vb->GetDesc().stride == sizeof(PosNormUVVert));
			for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
				vtxPtr[i].pos = va.position[i];
				vtxPtr[i].norm = va.normal[i];
				vtxPtr[i].uv = va.uv0[i];
			}
			vb->Unmap();
		}

		static inline void CopyPosNormCol(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
		{
			PosNormColVert *vtxPtr = vb->Map<PosNormColVert>(Graphics::BUFFER_MAP_WRITE);
			assert(vb->GetDesc().stride == sizeof(PosNormColVert));
			for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
				vtxPtr[i].pos = va.position[i];
				vtxPtr[i].norm = va.normal[i];
				vtxPtr[i].col = va.diffuse[i];
			}
			vb->Unmap();
		}

		// copies the contents of the VertexArray into the buffer
		bool VertexBuffer::Populate(const VertexArray &va)
		{
			PROFILE_SCOPED()
			assert(va.GetNumVerts() > 0);
			assert(va.GetNumVerts() <= m_capacity);
			bool result = false;
			const Graphics::AttributeSet as = va.GetAttributeSet();
			switch (as) {
			case Graphics::ATTRIB_POSITION:
				CopyPos(this, va);
				result = true;
				break;
			case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE:
				CopyPosCol(this, va);
				result = true;
				break;
			case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL:
				CopyPosNorm(this, va);
				result = true;
				break;
			case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0:
				CopyPosUV0(this, va);
				result = true;
				break;
			case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0:
				CopyPosColUV0(this, va);
				result = true;
				break;
			case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL | Graphics::ATTRIB_UV0:
				CopyPosNormUV0(this, va);
				result = true;
				break;
			case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL | Graphics::ATTRIB_DIFFUSE:
				CopyPosNormCol(this, va);
				result = true;
				break;
			}
			SetVertexCount(va.GetNumVerts());
			return result;
		}

		void VertexBuffer::BufferData(const size_t size, void *data)
		{
			PROFILE_SCOPED()
			assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
			if (GetDesc().usage == BUFFER_USAGE_DYNAMIC) {
				glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
				glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), static_cast<GLvoid *>(data), GL_DYNAMIC_DRAW);
			}
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
		CachedVertexBuffer::CachedVertexBuffer(const VertexBufferDesc &desc, size_t stateHash) :
			VertexBuffer(desc, stateHash)
		{
			assert(desc.usage == BufferUsage::BUFFER_USAGE_DYNAMIC);
			m_size = 0;
			m_lastFlushed = 0;
		}

		bool CachedVertexBuffer::Populate(const VertexArray &va)
		{
			assert(m_capacity - m_size >= va.GetNumVerts());

			// ugly but effective way of counting the number of non-empty vertex attribute slots
			uint32_t numAttrs = 0;
			for (; numAttrs < 8 && m_desc.attrib[numAttrs].semantic != ATTRIB_NONE; numAttrs++)
				;

			// Complicated-ish loop to deal with 16+ possible combinations of vertex formats
			// (Position is effectively required, or it would be 32)
			for (size_t idx = 0; idx < va.GetNumVerts(); idx++) {
				for (uint32_t n = 0; n < numAttrs; n++) {
					// Calculate the location of this component inside the vertex being written
					uint8_t *data = m_data + (m_size + idx) * m_desc.stride + m_desc.attrib[n].offset;
					switch (m_desc.attrib[n].semantic) {
					case ATTRIB_POSITION:
						*reinterpret_cast<vector3f *>(data) = va.position[idx];
						break;
					case ATTRIB_NORMAL:
						*reinterpret_cast<vector3f *>(data) = va.normal[idx];
						break;
					case ATTRIB_DIFFUSE:
						*reinterpret_cast<Color4ub *>(data) = va.diffuse[idx];
						break;
					case ATTRIB_UV0:
						*reinterpret_cast<vector2f *>(data) = va.uv0[idx];
						break;
					case ATTRIB_TANGENT:
						*reinterpret_cast<vector3f *>(data) = va.tangent[idx];
						break;
					default:
						assert(false && "Unimplemented vertex attribute in CachedVertexBuffer::Populate!");
						break;
					}
				}
			}

			m_size += va.GetNumVerts();
			return true;
		}

		// Send accumulated data to the GPU prior to using the buffer for rendering
		bool CachedVertexBuffer::Flush()
		{
			uint32_t dataSize = m_size * m_desc.stride;
			if (m_lastFlushed >= dataSize)
				return false;

			glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
			glBufferSubData(GL_ARRAY_BUFFER, m_lastFlushed, dataSize - m_lastFlushed, m_data + m_lastFlushed);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			m_lastFlushed = dataSize;
			m_written = true;
			return true;
		}

		// Reset the cache and associated buffer for use in a new frame
		void CachedVertexBuffer::Reset()
		{
			// respecify the buffer storage to orphan data that theoretically might still be in flight
			glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
			glBufferData(GL_ARRAY_BUFFER, m_capacity * m_desc.stride, nullptr, get_buffer_usage(m_desc.usage));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			m_size = 0;
			m_lastFlushed = 0;
			m_written = false;
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

		MeshObject::MeshObject(Graphics::VertexBuffer *vtx, Graphics::IndexBuffer *idx) :
			m_vtxBuffer(static_cast<OGL::VertexBuffer *>(vtx)),
			m_idxBuffer(static_cast<OGL::IndexBuffer *>(idx))
		{
			assert(m_vtxBuffer.Valid());

			m_vao = BuildVAOFromDesc(m_vtxBuffer->GetDesc());
			glBindVertexArray(m_vao);

			glBindVertexBuffer(0, m_vtxBuffer->GetBuffer(), 0, m_vtxBuffer->GetDesc().stride);
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

		GLuint BuildVAOFromDesc(const Graphics::VertexBufferDesc desc)
		{
			GLuint vao = 0;
			// Create the VAOs
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			//Setup the VAO pointers
			for (uint32_t i = 0; i < MAX_ATTRIBS; i++) {
				const auto &attr = desc.attrib[i];
				if (attr.semantic == ATTRIB_NONE)
					break;

				GLuint attrib = get_attrib_index(attr.semantic);
				// Enable the attribute at that location
				glEnableVertexAttribArray(attrib);
				// Tell OpenGL what the array contains
				glVertexAttribFormat(attrib, get_num_components(attr.format), get_component_type(attr.format), is_attr_normalized(attr.semantic), attr.offset);
				// All vertex attribs will be sourced from the same buffer
				glVertexAttribBinding(attrib, 0);
			}

			CHECKERRORS();

			// Set up the divisor for the instance data buffer binding
			glVertexBindingDivisor(1, 1);
			// Set up the slots for an instance buffer now, so we don't need to touch it again.
			const size_t sizeVec4 = (sizeof(float) * 4);
			for (uint32_t idx = 0; idx < 4; idx++) {
				// Each row of the matrix needs to be set separately.
				glVertexAttribFormat(InstanceBuffer::INSTOFFS_MAT0 + idx, 4, GL_FLOAT, GL_FALSE, idx * sizeVec4);
				// All instance-data attribs will be sourced from a separate buffer
				glVertexAttribBinding(InstanceBuffer::INSTOFFS_MAT0 + idx, 1);
			}

			CHECKERRORS();

			glBindVertexArray(0);
			return vao;
		}

	} //namespace OGL
} //namespace Graphics
