// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GRAPHICS_VERTEXBUFFER_H
#define GRAPHICS_VERTEXBUFFER_H
/**
 * A Vertex Buffer is created by filling out a
 * description struct with desired vertex attributes
 * and calling renderer->CreateVertexBuffer.
 * Can be used in combination with IndexBuffer,
 * for optimal rendering of complex geometry.
 * Call Map to write/read from the buffer, and Unmap to
 * commit the changes.
 * Buffers come in two usage flavors, static and dynamic.
 * Use Static buffer, when the geometry never changes.
 * Avoid mapping a buffer for reading, as it may be slow,
 * especially with static buffers.
 *
 * Expansion possibilities: range-based Map
 */
#include "Types.h"
#include "libs.h"

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
		Uint32 offset;
	};

	struct VertexBufferDesc {
		VertexBufferDesc();
		//byte offset of an existing attribute
		Uint32 GetOffset(VertexAttrib) const;

		//used internally for calculating offsets
		static Uint32 CalculateOffset(const VertexBufferDesc &, VertexAttrib);
		static Uint32 GetAttribSize(VertexAttribFormat);

		//semantic ATTRIB_NONE ends description (when not using all attribs)
		VertexAttribDesc attrib[MAX_ATTRIBS];
		Uint32 numVertices;
		//byte size of one vertex, if zero this is
		//automatically calculated for created buffers
		Uint32 stride;
		BufferUsage usage;
	};

	class Mappable : public RefCounted {
	public:
		virtual ~Mappable() {}
		virtual void Unmap() = 0;

		inline Uint32 GetSize() const { return m_size; }
		inline Uint32 GetCapacity() const { return m_capacity; }

	protected:
		explicit Mappable(const Uint32 size) :
			m_mapMode(BUFFER_MAP_NONE),
			m_size(size),
			m_capacity(size) {}
		BufferMapMode m_mapMode; //tracking map state

		// size is the current number of elements in the buffer
		Uint32 m_size;
		// capacity is the maximum number of elements that can be put in the buffer
		Uint32 m_capacity;
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

		virtual void Bind() = 0;
		virtual void Release() = 0;

	protected:
		virtual Uint8 *MapInternal(BufferMapMode) = 0;
		VertexBufferDesc m_desc;
	};

	// Index buffer
	class IndexBuffer : public Mappable {
	public:
		IndexBuffer(Uint32 size, BufferUsage);
		virtual ~IndexBuffer();
		virtual Uint32 *Map(BufferMapMode) = 0;

		// change the buffer data without mapping
		virtual void BufferData(const size_t, void *) = 0;

		Uint32 GetIndexCount() const { return m_indexCount; }
		void SetIndexCount(Uint32);
		BufferUsage GetUsage() const { return m_usage; }

		virtual void Bind() = 0;
		virtual void Release() = 0;

	protected:
		Uint32 m_indexCount;
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

} // namespace Graphics
#endif // GRAPHICS_VERTEXBUFFER_H
