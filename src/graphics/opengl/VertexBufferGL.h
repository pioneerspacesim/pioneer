// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef OGL_VERTEXBUFFER_H
#define OGL_VERTEXBUFFER_H
#include "OpenGLLibs.h"
#include "graphics/VertexBuffer.h"

namespace Graphics {
	class RendererOGL;

	namespace OGL {

		class GLBufferBase {
		public:
			GLBufferBase() :
				m_written(false) {}
			GLuint GetBuffer() const { return m_buffer; }

		protected:
			GLuint m_buffer;
			bool m_written; // to check for invalid data rendering
		};

		class VertexBuffer : public Graphics::VertexBuffer, public GLBufferBase {
		public:
			VertexBuffer(const VertexBufferDesc &);
			~VertexBuffer();

			virtual void Unmap() override final;

			// copies the contents of the VertexArray into the buffer
			virtual bool Populate(const VertexArray &) override final;

			// change the buffer data without mapping
			virtual void BufferData(const size_t, void *) override final;

			virtual void Bind() override final;
			virtual void Release() override final;

		protected:
			virtual Uint8 *MapInternal(BufferMapMode) override final;

		private:
			Uint8 *m_data;
		};

		class IndexBuffer : public Graphics::IndexBuffer, public GLBufferBase {
		public:
			IndexBuffer(Uint32 size, BufferUsage);
			~IndexBuffer();

			virtual Uint32 *Map(BufferMapMode) override final;
			virtual void Unmap() override final;

			// change the buffer data without mapping
			virtual void BufferData(const size_t, void *) override final;

			virtual void Bind() override final;
			virtual void Release() override final;

		private:
			Uint32 *m_data;
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

		protected:
			friend class MeshObject; // need access to InstOffs enum
			enum InstOffs {
				INSTOFFS_MAT0 = 6, // these value must match those of a_transform within data/shaders/opengl/attributes.glsl
				INSTOFFS_MAT1 = 7,
				INSTOFFS_MAT2 = 8,
				INSTOFFS_MAT3 = 9
			};
			std::unique_ptr<matrix4x4f[]> m_data;
		};

		class MeshObject final : public Graphics::MeshObject {
		public:
			MeshObject(RefCountedPtr<VertexBuffer> v, RefCountedPtr<IndexBuffer> i);
			~MeshObject() override;

			void Bind() override;
			void Release() override;

			Graphics::VertexBuffer *GetVertexBuffer() const override { return m_vtxBuffer.Get(); };
			Graphics::IndexBuffer *GetIndexBuffer() const override { return m_idxBuffer.Get(); };

		protected:
			GLuint m_vao;
			RefCountedPtr<VertexBuffer> m_vtxBuffer;
			RefCountedPtr<IndexBuffer> m_idxBuffer;
		};

	} // namespace OGL
} // namespace Graphics

#endif // OGL_VERTEXBUFFER_H
