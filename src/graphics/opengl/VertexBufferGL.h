// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "graphics/Types.h"
#include "graphics/VertexBuffer.h"
#include "graphics/opengl/GLBufferBase.h"

#include <memory>

namespace Graphics {
	class RendererOGL;

	namespace OGL {

		class VertexBuffer : public Graphics::VertexBuffer, public GLBufferBase {
		public:
			VertexBuffer(const VertexBufferDesc &, size_t stateHash);
			~VertexBuffer();

			virtual void Unmap() override;

			// copies the contents of the VertexArray into the buffer
			virtual bool Populate(const VertexArray &) override;

			// change the buffer data without mapping
			virtual void BufferData(const size_t, void *) override final;

			virtual void Bind() override final;
			virtual void Release() override final;

			size_t GetVertexFormatHash() const { return m_vertexStateHash; }

		protected:
			virtual Uint8 *MapInternal(BufferMapMode) override;
			Uint8 *m_data;
			size_t m_vertexStateHash;
		};

		class CachedVertexBuffer : public VertexBuffer {
		public:
			CachedVertexBuffer(const VertexBufferDesc &, size_t stateHash);

			virtual bool Populate(const VertexArray &) override final;
			uint32_t GetOffset() { return m_size * m_desc.stride; }

			bool Flush();
			void Reset();

		protected:
			using VertexBuffer::BufferData;
			using VertexBuffer::Map;
			using VertexBuffer::Unmap;

		private:
			uint32_t m_lastFlushed;
		};

		class IndexBuffer : public Graphics::IndexBuffer, public GLBufferBase {
		public:
			IndexBuffer(Uint32 size, BufferUsage, IndexBufferSize);
			~IndexBuffer();

			virtual Uint32 *Map(BufferMapMode) override final;
			virtual Uint16 *Map16(BufferMapMode) override final;
			virtual void Unmap() override final;

			// change the buffer data without mapping
			virtual void BufferData(const size_t, void *) override final;

			virtual void Bind() override final;
			virtual void Release() override final;

		private:
			Uint32 *m_data;
			Uint16 *m_data16;
		};

		// Instance buffer
		class InstanceBuffer final : public Graphics::InstanceBuffer, public GLBufferBase {
		public:
			InstanceBuffer(Uint32 size, BufferUsage);
			virtual ~InstanceBuffer() override final;
			virtual matrix4x4f *Map(BufferMapMode) override final;
			virtual void Unmap() override final;

			virtual void Bind() override final;
			virtual void Release() override final;

			enum InstOffs {
				INSTOFFS_MAT0 = 6, // these value must match those of a_transform within data/shaders/opengl/attributes.glsl
				INSTOFFS_MAT1 = 7,
				INSTOFFS_MAT2 = 8,
				INSTOFFS_MAT3 = 9
			};

		protected:
			friend class MeshObject; // need access to InstOffs enum
			std::unique_ptr<matrix4x4f[]> m_data;
		};

		class MeshObject final : public Graphics::MeshObject {
		public:
			MeshObject(Graphics::VertexBuffer *vtx, Graphics::IndexBuffer *idx);
			~MeshObject() override;

			void Bind() override;
			void Release() override;

			Graphics::VertexBuffer *GetVertexBuffer() const override { return m_vtxBuffer.Get(); };
			Graphics::IndexBuffer *GetIndexBuffer() const override { return m_idxBuffer.Get(); };

		protected:
			friend class Graphics::RendererOGL;

			GLuint GetVertexArrayObject() const { return m_vao; }
			RefCountedPtr<VertexBuffer> m_vtxBuffer;
			RefCountedPtr<IndexBuffer> m_idxBuffer;
			GLuint m_vao = 0;
		};

		GLuint BuildVAOFromDesc(const Graphics::VertexBufferDesc desc);

	} // namespace OGL
} // namespace Graphics
