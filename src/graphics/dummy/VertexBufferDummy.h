// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef DUMMY_VERTEXBUFFER_H
#define DUMMY_VERTEXBUFFER_H

#include "graphics/Types.h"
#include "graphics/VertexBuffer.h"

#include <memory>

namespace Graphics {

	namespace Dummy {

		class VertexBuffer : public Graphics::VertexBuffer {
		public:
			VertexBuffer(const VertexBufferDesc &d) :
				Graphics::VertexBuffer(d),
				m_buffer(new Uint8[m_desc.numVertices * m_desc.stride])
			{}

			// copies the contents of the VertexArray into the buffer
			virtual bool Populate(const VertexArray &) override final { return true; }

			// change the buffer data without mapping
			virtual void BufferData(const size_t, void *) override final {}

			virtual void Bind() override final {}
			virtual void Release() override final {}

			virtual void Unmap() override final {}

		protected:
			virtual Uint8 *MapInternal(BufferMapMode) override final { return m_buffer.get(); }

		private:
			std::unique_ptr<Uint8[]> m_buffer;
		};

		class IndexBuffer : public Graphics::IndexBuffer {
		public:
			IndexBuffer(Uint32 size, BufferUsage bu, IndexBufferSize el) :
				Graphics::IndexBuffer(size, bu, el)
			{
				if (el == INDEX_BUFFER_32BIT)
					m_buffer.reset(new Uint32[size]);
				else
					m_buffer16.reset(new Uint16[size]);
			}

			virtual Uint32 *Map(BufferMapMode) override final { return m_buffer.get(); }
			virtual Uint16 *Map16(BufferMapMode) override final { return m_buffer16.get(); }
			virtual void Unmap() override final {}

			virtual void BufferData(const size_t, void *) override final {}

			virtual void Bind() override final {}
			virtual void Release() override final {}

		private:
			std::unique_ptr<Uint32[]> m_buffer;
			std::unique_ptr<Uint16[]> m_buffer16;
		};

		// Instance buffer
		class InstanceBuffer : public Graphics::InstanceBuffer {
		public:
			InstanceBuffer(Uint32 size, BufferUsage hint) :
				Graphics::InstanceBuffer(size, hint),
				m_data(new matrix4x4f[size])
			{}
			virtual ~InstanceBuffer(){};
			virtual matrix4x4f *Map(BufferMapMode) override final { return m_data.get(); }
			virtual void Unmap() override final {}

			Uint32 GetSize() const { return m_size; }
			BufferUsage GetUsage() const { return m_usage; }

			virtual void Bind() override final {}
			virtual void Release() override final {}

		protected:
			std::unique_ptr<matrix4x4f> m_data;
		};

		class MeshObject final : public Graphics::MeshObject {
		public:
			MeshObject(VertexBuffer *v, IndexBuffer *i) :
				m_vtxBuffer(v),
				m_idxBuffer(i)
			{}
			virtual ~MeshObject() override final {}

			virtual Graphics::VertexBuffer *GetVertexBuffer() const override final { return m_vtxBuffer.Get(); }
			virtual Graphics::IndexBuffer *GetIndexBuffer() const override final { return m_idxBuffer.Get(); }

			virtual void Bind() override final {}
			virtual void Release() override final {}

		protected:
			RefCountedPtr<VertexBuffer> m_vtxBuffer;
			RefCountedPtr<IndexBuffer> m_idxBuffer;
		};

	} // namespace Dummy
} // namespace Graphics

#endif
