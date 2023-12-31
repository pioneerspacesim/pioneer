// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "graphics/BufferCommon.h"
#include "graphics/Types.h"
#include "matrix4x4.h"

/**
 * A Vertex Buffer is created by filling out a description struct with desired
 * vertex attributes and calling renderer->CreateVertexBuffer. Can be used in
 * combination with IndexBuffer, for optimal rendering of complex geometry.
 *
 * Call Map to write/read from the buffer, and Unmap to commit the changes.
 * Buffers come in two usage flavors, static and dynamic.
 * - Use a Static buffer when the geometry never changes.
 * - Use a Dynamic buffer if you'll be uploading data regularly.
 *
 * Strictly avoid mapping a buffer for reading unless you have no choice, as it
 * may be extremely slow, especially with static buffers.
 *
 * Expansion possibilities: range-based Map
 */
namespace Graphics {

	// fwd declaration
	class VertexArray;

	const Uint32 MAX_ATTRIBS = 8;

	struct VertexAttribDesc {
		//position, texcoord, normal etc.
		VertexAttrib semantic;
		//float3, float2 etc.
		VertexAttribFormat format;
		//byte offset of the attribute, if zero this
		//is automatically filled for created buffers
		uint16_t offset;
	};
	static_assert(sizeof(VertexAttribDesc) == 4);

	struct VertexBufferDesc {
		VertexBufferDesc();
		static VertexBufferDesc FromAttribSet(AttributeSet set);

		//byte offset of an existing attribute
		Uint32 GetOffset(VertexAttrib) const;

		//used internally for calculating offsets
		static Uint32 CalculateOffset(const VertexBufferDesc &, VertexAttrib);
		static Uint32 GetAttribSize(VertexAttribFormat);

		void CalculateOffsets();

		//semantic ATTRIB_NONE ends description (when not using all attribs)
		VertexAttribDesc attrib[MAX_ATTRIBS];
		Uint32 numVertices;
		//byte size of one vertex, if zero this is
		//automatically calculated for created buffers
		Uint32 stride;
		BufferUsage usage;
	};

	class VertexBuffer : public Mappable {
	public:
		VertexBuffer(const VertexBufferDesc &desc) :
			Mappable(desc.numVertices),
			m_desc(desc) {}
		virtual ~VertexBuffer();
		const VertexBufferDesc &GetDesc() const { return m_desc; }

		template <typename T>
		T *Map(BufferMapMode mode)
		{
			return reinterpret_cast<T *>(MapInternal(mode));
		}

		//Vertex count used for rendering.
		//By default the maximum set in description, but
		//you may set a smaller count for partial rendering
		bool SetVertexCount(Uint32);

		// copies the contents of the VertexArray into the buffer
		virtual bool Populate(const VertexArray &) = 0;

		// change the buffer data without mapping
		virtual void BufferData(const size_t, void *) = 0;

		// Bind the vertex buffer for use in rendering
		virtual void Bind() = 0;

		// Release the vertex buffer from rendering
		virtual void Release() = 0;

	protected:
		virtual Uint8 *MapInternal(BufferMapMode) = 0;
		VertexBufferDesc m_desc;
	};

	// Index buffer
	class IndexBuffer : public Mappable {
	public:
		IndexBuffer(Uint32 size, BufferUsage, IndexBufferSize);
		virtual ~IndexBuffer();
		virtual Uint32 *Map(BufferMapMode) = 0;
		virtual Uint16 *Map16(BufferMapMode) = 0;

		// change the buffer data without mapping
		virtual void BufferData(const size_t, void *) = 0;

		Uint32 GetIndexCount() const { return m_indexCount; }
		void SetIndexCount(Uint32);
		BufferUsage GetUsage() const { return m_usage; }
		IndexBufferSize GetElementSize() const { return m_elemSize; }

		virtual void Bind() = 0;
		virtual void Release() = 0;

	protected:
		Uint32 m_indexCount;
		IndexBufferSize m_elemSize;
		BufferUsage m_usage;
	};

	// Instance buffer
	class InstanceBuffer : public Mappable {
	public:
		InstanceBuffer(Uint32 size, BufferUsage);
		virtual ~InstanceBuffer();
		virtual matrix4x4f *Map(BufferMapMode) = 0;

		Uint32 GetInstanceCount() const { return m_instanceCount; }
		void SetInstanceCount(const Uint32);
		BufferUsage GetUsage() const { return m_usage; }

		virtual void Bind() = 0;
		virtual void Release() = 0;

	protected:
		Uint32 m_instanceCount;
		BufferUsage m_usage;
	};

	/*
     * Wraps a vertex buffer and optional index buffer into a single mesh.
	 *
	 * This class maps to OpenGL's vertex array objects, and is used to
	 * coalesce primitive data for drawing commands in one place.
	 *
	 * It is the calling code's responsibility to ensure that once a draw has
	 * been issued using a MeshObject, the MeshObject stays alive until the
	 * command list is executed or reset (e.g. SwapBuffers()). Failure to
	 * observe this requirement will result in undefined behavior.
	 */
	class MeshObject : public RefCounted {
	public:
		virtual ~MeshObject() {}

		virtual void Bind() = 0;
		virtual void Release() = 0;

		virtual VertexBuffer *GetVertexBuffer() const = 0;
		virtual IndexBuffer *GetIndexBuffer() const = 0;
	};

} // namespace Graphics
