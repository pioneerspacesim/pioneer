// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/opengl/VertexBufferGL.h"
#include "graphics/VertexArray.h"
#include "utils.h"

namespace Graphics {
	namespace OGL {

		GLint get_num_components(VertexAttribFormat fmt)
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

		GLenum get_component_type(VertexAttribFormat fmt)
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

		VertexBuffer::VertexBuffer(const VertexBufferDesc &desc) :
			Graphics::VertexBuffer(desc)
		{
			PROFILE_SCOPED()
			//update offsets in desc
			for (Uint32 i = 0; i < MAX_ATTRIBS; i++) {
				if (m_desc.attrib[i].offset == 0)
					m_desc.attrib[i].offset = VertexBufferDesc::CalculateOffset(m_desc, m_desc.attrib[i].semantic);
			}

			//update stride in desc (respecting offsets)
			if (m_desc.stride == 0) {
				Uint32 lastAttrib = 0;
				while (lastAttrib < MAX_ATTRIBS) {
					if (m_desc.attrib[lastAttrib].semantic == ATTRIB_NONE)
						break;
					lastAttrib++;
				}

				m_desc.stride = m_desc.attrib[lastAttrib].offset + VertexBufferDesc::GetAttribSize(m_desc.attrib[lastAttrib].format);
			}
			assert(m_desc.stride > 0);
			assert(m_desc.numVertices > 0);

			//SetVertexCount(m_desc.numVertices);

			glGenVertexArrays(1, &m_vao);
			glBindVertexArray(m_vao);

			glGenBuffers(1, &m_buffer);

			//Allocate GL buffer with undefined contents
			//Critical optimisation for some architectures in cases where buffer is created and written in the same frame
			glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
			const Uint32 dataSize = m_desc.numVertices * m_desc.stride;
			const GLenum usage = (m_desc.usage == BUFFER_USAGE_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
			glBufferData(GL_ARRAY_BUFFER, dataSize, 0, usage);

			//Setup the VAO pointers
			for (Uint8 i = 0; i < MAX_ATTRIBS; i++) {
				const auto &attr = m_desc.attrib[i];
				if (attr.semantic == ATTRIB_NONE)
					break;

				// Tell OpenGL what the array contains
				const auto offset = reinterpret_cast<const GLvoid *>(attr.offset);
				switch (attr.semantic) {
				case ATTRIB_POSITION:
					glEnableVertexAttribArray(0); // Enable the attribute at that location
					glVertexAttribPointer(0, get_num_components(attr.format), get_component_type(attr.format), GL_FALSE, m_desc.stride, offset);
					break;
				case ATTRIB_NORMAL:
					glEnableVertexAttribArray(1); // Enable the attribute at that location
					glVertexAttribPointer(1, get_num_components(attr.format), get_component_type(attr.format), GL_FALSE, m_desc.stride, offset);
					break;
				case ATTRIB_DIFFUSE:
					glEnableVertexAttribArray(2); // Enable the attribute at that location
					glVertexAttribPointer(2, get_num_components(attr.format), get_component_type(attr.format), GL_TRUE, m_desc.stride, offset); // only normalise the colours
					break;
				case ATTRIB_UV0:
					glEnableVertexAttribArray(3); // Enable the attribute at that location
					glVertexAttribPointer(3, get_num_components(attr.format), get_component_type(attr.format), GL_FALSE, m_desc.stride, offset);
					break;
				case ATTRIB_UV1:
					glEnableVertexAttribArray(4); // Enable the attribute at that location
					glVertexAttribPointer(4, get_num_components(attr.format), get_component_type(attr.format), GL_FALSE, m_desc.stride, offset);
					break;
				case ATTRIB_TANGENT:
					glEnableVertexAttribArray(5); // Enable the attribute at that location
					glVertexAttribPointer(5, get_num_components(attr.format), get_component_type(attr.format), GL_FALSE, m_desc.stride, offset);
					break;
				case ATTRIB_NONE:
				default:
					break;
				}
			}

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

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
			glDeleteVertexArrays(1, &m_vao);
			delete[] m_data;
		}

		Uint8 *VertexBuffer::MapInternal(BufferMapMode mode)
		{
			PROFILE_SCOPED()
			assert(mode != BUFFER_MAP_NONE); //makes no sense
			assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
			m_mapMode = mode;
			if (GetDesc().usage == BUFFER_USAGE_STATIC) {
				glBindVertexArray(m_vao);
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
					glBufferData(GL_ARRAY_BUFFER, dataSize, 0, GL_DYNAMIC_DRAW);
					glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_data);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}
			}
			glBindVertexArray(0);

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
				glBindVertexArray(m_vao);
				glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
				glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), static_cast<GLvoid *>(data), GL_DYNAMIC_DRAW);
			}
		}

		void VertexBuffer::Bind()
		{
			assert(m_written);
			glBindVertexArray(m_vao);

			// Enable the Vertex attributes
			for (Uint8 i = 0; i < MAX_ATTRIBS; i++) {
				const auto &attr = m_desc.attrib[i];
				switch (attr.semantic) {
				case ATTRIB_POSITION: glEnableVertexAttribArray(0); break;
				case ATTRIB_NORMAL: glEnableVertexAttribArray(1); break;
				case ATTRIB_DIFFUSE: glEnableVertexAttribArray(2); break;
				case ATTRIB_UV0: glEnableVertexAttribArray(3); break;
				case ATTRIB_UV1: glEnableVertexAttribArray(4); break;
				case ATTRIB_TANGENT: glEnableVertexAttribArray(5); break;
				case ATTRIB_NONE:
				default:
					return;
				}
			}
		}

		void VertexBuffer::Release()
		{
			// Enable the Vertex attributes
			for (Uint8 i = 0; i < MAX_ATTRIBS; i++) {
				const auto &attr = m_desc.attrib[i];
				switch (attr.semantic) {
				case ATTRIB_POSITION: glDisableVertexAttribArray(0); break;
				case ATTRIB_NORMAL: glDisableVertexAttribArray(1); break;
				case ATTRIB_DIFFUSE: glDisableVertexAttribArray(2); break;
				case ATTRIB_UV0: glDisableVertexAttribArray(3); break;
				case ATTRIB_UV1: glDisableVertexAttribArray(4); break;
				case ATTRIB_TANGENT: glDisableVertexAttribArray(5); break;
				case ATTRIB_NONE:
				default:
					return;
				}
			}

			glBindVertexArray(0);
		}

		// ------------------------------------------------------------
		IndexBuffer::IndexBuffer(Uint32 size, BufferUsage hint) :
			Graphics::IndexBuffer(size, hint)
		{
			assert(size > 0);

			const GLenum usage = (hint == BUFFER_USAGE_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
			glGenBuffers(1, &m_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Uint32) * m_size, 0, usage);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			if (GetUsage() != BUFFER_USAGE_STATIC) {
				m_data = new Uint32[size];
				memset(m_data, 0, sizeof(Uint32) * size);
			} else
				m_data = nullptr;
		}

		IndexBuffer::~IndexBuffer()
		{
			glDeleteBuffers(1, &m_buffer);
			delete[] m_data;
		}

		Uint32 *IndexBuffer::Map(BufferMapMode mode)
		{
			assert(mode != BUFFER_MAP_NONE); //makes no sense
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

		void IndexBuffer::Unmap()
		{
			assert(m_mapMode != BUFFER_MAP_NONE); //not currently mapped

			if (GetUsage() == BUFFER_USAGE_STATIC) {
				glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			} else {
				if (m_mapMode == BUFFER_MAP_WRITE) {
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
					glBufferData(GL_ARRAY_BUFFER, sizeof(Uint32) * m_size, 0, GL_DYNAMIC_DRAW);
					glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(Uint32) * m_size, m_data);
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
			assert(mode != BUFFER_MAP_NONE); //makes no sense
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
			glBindBuffer(GL_ARRAY_BUFFER, m_buffer);

			// used to pass a matrix4x4f in, however each attrib array is max size of (GLSL) vec4 so must enable 4 arrays
			const size_t sizeVec4 = (sizeof(float) * 4);
			glEnableVertexAttribArray(INSTOFFS_MAT0);
			glVertexAttribPointer(INSTOFFS_MAT0, 4, GL_FLOAT, GL_FALSE, 4 * sizeVec4, reinterpret_cast<const GLvoid *>(0));
			glEnableVertexAttribArray(INSTOFFS_MAT1);
			glVertexAttribPointer(INSTOFFS_MAT1, 4, GL_FLOAT, GL_FALSE, 4 * sizeVec4, reinterpret_cast<const GLvoid *>(sizeVec4));
			glEnableVertexAttribArray(INSTOFFS_MAT2);
			glVertexAttribPointer(INSTOFFS_MAT2, 4, GL_FLOAT, GL_FALSE, 4 * sizeVec4, reinterpret_cast<const GLvoid *>(2 * sizeVec4));
			glEnableVertexAttribArray(INSTOFFS_MAT3);
			glVertexAttribPointer(INSTOFFS_MAT3, 4, GL_FLOAT, GL_FALSE, 4 * sizeVec4, reinterpret_cast<const GLvoid *>(3 * sizeVec4));

			glVertexAttribDivisor(INSTOFFS_MAT0, 1);
			glVertexAttribDivisor(INSTOFFS_MAT1, 1);
			glVertexAttribDivisor(INSTOFFS_MAT2, 1);
			glVertexAttribDivisor(INSTOFFS_MAT3, 1);
		}

		void InstanceBuffer::Release()
		{
			// see enable comment above
			glDisableVertexAttribArray(INSTOFFS_MAT0);
			glDisableVertexAttribArray(INSTOFFS_MAT1);
			glDisableVertexAttribArray(INSTOFFS_MAT2);
			glDisableVertexAttribArray(INSTOFFS_MAT3);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

	} //namespace OGL
} //namespace Graphics
