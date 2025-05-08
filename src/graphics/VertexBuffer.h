// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
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

	// tuned to ensure the size of VertexFormatDesc == 64
	constexpr uint32_t MAX_ATTRIBS = 12;
	constexpr uint32_t MAX_BINDINGS = 4;

#pragma pack(push, 1)

	struct VertexAttribDesc {
		// size of the vertex data element
		VertexAttribFormat format = VertexAttribFormat::ATTRIB_FORMAT_NONE;
		// program location index of this vertex
		uint8_t location = 0;
		// buffer binding source for this vertex attrib
		uint16_t binding : 4;
		// byte offset of the attribute, if zero this is automatically filled
		// when creating a vertex format desc
		uint16_t offset : 12;
	};
	static_assert(sizeof(VertexAttribDesc) == 4);

	struct VertexBindingDesc {
		// stride between vertices in the buffer
		uint16_t stride = 0;
		// Is this binding entry used by the vertex format
		uint8_t enabled = 0;
		// rate of vertex advancement in the buffer
		VertexAttribRate rate = VertexAttribRate::ATTRIB_RATE_NORMAL;
	};
	static_assert(sizeof(VertexBindingDesc) == 4);

#pragma pack(pop)

	// Enumerates the possible reasons for VertexFormatDesc validation to fail
	enum class InvalidVertexFormatReason {
		OK = 0,
		InvalidBinding = 1,
		LocationOverlap = 2
	};

	// Return the index of the given attribute within the passed AttributeSet.
	size_t GetAttributeIndexInSet(AttributeSet set, VertexAttrib attrib);
	// Return the number of active VertexAttribs in the given set.
	size_t GetNumActiveAttribsInSet(AttributeSet set);
	// Populate the passed list of individual vertex attributes that comprise the passed AttributeSet.
	void GetActiveAttribsInSet(AttributeSet set, VertexAttrib *attribs, size_t numAttribs);

	struct VertexFormatDesc {
		VertexFormatDesc();

		// Create a vertex format descriptor from the given attribute set.
		// Attributes are mapped to predefined locations and sourced from buffer binding 0
		static VertexFormatDesc FromAttribSet(AttributeSet set);

		// Run a validation check on the layout of this vertex format
		InvalidVertexFormatReason ValidateDesc() const;

		// Return a hash of this vertex format descriptor
		uint64_t Hash() const;

		size_t GetNumAttribs() const;
		size_t GetNumBindings() const;

		VertexAttribDesc attribs[MAX_ATTRIBS];
		VertexBindingDesc bindings[MAX_BINDINGS];
	};
	static_assert(sizeof(VertexFormatDesc) == 64);

	class VertexBuffer : public Mappable {
	public:
		VertexBuffer(BufferUsage usage, uint32_t size, uint32_t stride) :
			Mappable(size),
			m_stride(stride),
			m_usage(usage) {}
		virtual ~VertexBuffer();

		uint32_t GetStride() const { return m_stride; }

		template <typename T>
		T *Map(BufferMapMode mode)
		{
			return reinterpret_cast<T *>(MapInternal(mode));
		}

		// Map the given range of the buffer and return a pointer to it.
		// Note that all values are in bytes not vertices.
		template <typename T>
		T *MapRange(size_t start, size_t size, BufferMapMode mode)
		{
			return reinterpret_cast<T *>(MapRangeInternal(mode, start * sizeof(T), size * sizeof(T)));
		}

		// Unmap the last-mapped range and flush data to GPU
		// If the buffer is dynamic, flush=false may be specified in which case a separate call to FlushRange must be used.
		virtual void UnmapRange(bool flush = true) = 0;

		// Explicitly flush (i.e. upload) the given range of the dynamic buffer to the GPU.
		// This buffer must have been created with BUFFER_USAGE_DYNAMIC and previously mapped with BUFFER_MAP_WRITE.
		// Note that all values are in bytes not vertices.
		virtual void FlushRange(size_t start, size_t size) = 0;

		//Vertex count used for rendering.
		//By default the maximum set in description, but
		//you may set a smaller count for partial rendering
		bool SetVertexCount(Uint32);

		// change the buffer data without mapping
		virtual void BufferData(const size_t, void *) = 0;

		// Recreate the underlying GPU memory to an empty state.
		// Only allowed for BUFFER_USAGE_DYNAMIC buffers.
		// Not guaranteed to have an effect outside of the OpenGL backend, should not be used as a way to clear GPU buffers.
		virtual void Reset() = 0;

		// Bind the vertex buffer for use in rendering
		virtual void Bind() = 0;

		// Release the vertex buffer from rendering
		virtual void Release() = 0;

	protected:
		virtual uint8_t *MapInternal(BufferMapMode) = 0;
		virtual uint8_t *MapRangeInternal(BufferMapMode, size_t start, size_t length) = 0;
		uint32_t m_stride;
		BufferUsage m_usage;
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
		virtual const VertexFormatDesc &GetFormat() const = 0;
	};

} // namespace Graphics
