// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Color.h"
#include "graphics/Graphics.h"
#include "graphics/Span.h"
#include "graphics/Types.h"
#include "graphics/VertexBuffer.h"
#include "OpenGLLibs.h"
#include <variant>

namespace Graphics {

	class Material;
	class MeshObject;
	class VertexArray;
	class RenderTarget;

	class RendererOGL;

	namespace OGL {

		class Material;
		class MeshObject;
		class Program;
		class Shader;
		class TextureGL;
		class RenderTarget;
		class IndexBuffer;
		class UniformBuffer;
		class VertexBuffer;

		class CommandList {
		public:
			struct DrawCmd {
				MeshObject *mesh;
				Program *program = nullptr;
				size_t renderStateHash = 0;
				GLuint vertexState = 0;
				char *drawData;
			};

			struct DrawCmd2 {
				uint32_t idxBuffer : 1;
				uint32_t numVtxBuffers : 2;
				uint32_t instanceCount : 29;
				uint32_t elementCount = 0;
				GLuint vertexState = 0;
				size_t renderStateHash = 0;
				Program *program = nullptr;
				char *drawData;
			};

			struct DynamicDrawCmd {
				BufferBinding<VertexBuffer> vtxBind;
				BufferBinding<IndexBuffer> idxBind;
				Program *program = nullptr;
				size_t renderStateHash = 0;
				GLuint vertexState = 0;
				char *drawData;
			};

			struct RenderPassCmd {
				RenderTarget *renderTarget;
				ViewportExtents extents;
				ViewportExtents scissor;
				bool setRenderTarget;
				bool setScissor;
				bool clearColors;
				bool clearDepth;
				Color clearColor;
			};

			struct BlitRenderTargetCmd {
				RenderTarget *srcTarget;
				RenderTarget *dstTarget;
				ViewportExtents srcExtents;
				ViewportExtents dstExtents;
				bool resolveMSAA;
				bool blitDepthBuffer;
				bool linearFilter;
			};

			// development asserts to ensure sizes are kept reasonable.
			// if you need to go beyond these sizes, add a new command instead.
			static_assert(sizeof(DrawCmd) <= 64);
			static_assert(sizeof(DynamicDrawCmd) <= 64);
			static_assert(sizeof(RenderPassCmd) <= 64);

			void AddDrawCmd(Graphics::MeshObject *mesh, Graphics::Material *mat);
			void AddDrawCmd2(const Span<Graphics::VertexBuffer *const> vtxBuffer, Graphics::IndexBuffer *buffer, Graphics::Material *mat, uint32_t numElements, uint32_t numInstances);
			void AddDynamicDrawCmd(BufferBinding<Graphics::VertexBuffer> vtx, BufferBinding<Graphics::IndexBuffer> idx, Graphics::Material *mat);

			void AddRenderPassCmd(RenderTarget *renderTarget, ViewportExtents extents);
			void AddScissorCmd(ViewportExtents extents);
			void AddClearCmd(bool clearColors, bool clearDepth, Color color);

			// NOTE: bound render target state will be invalidated.
			// BlitRenderTargetCmd should be followed by a RenderPassCmd
			void AddBlitRenderTargetCmd(
				Graphics::RenderTarget *src, Graphics::RenderTarget *dst,
				const ViewportExtents &srcExtents,
				const ViewportExtents &dstExtents,
				bool resolveMSAA = false, bool blitDepthBuffer = false, bool linearFilter = true);

		protected:
			using Cmd = std::variant<DrawCmd, DrawCmd2, DynamicDrawCmd, RenderPassCmd, BlitRenderTargetCmd>;
			const std::vector<Cmd> &GetDrawCmds() const { return m_drawCmds; }

			bool IsEmpty() const { return m_drawCmds.empty(); }
			void Reset();

		private:
			friend class Graphics::RendererOGL;
			CommandList(Graphics::RendererOGL *r) :
				m_renderer(r)
			{
				m_drawCmds.reserve(32);
			}

			// Allocate space for all shader data that needs to be cached forward
			char *AllocDrawData(const Shader *shader, uint32_t numBuffers = 0);
			// Create and cache all material data needed for later execution of a draw command
			char *SetupMaterialData(OGL::Material *mat, uint32_t numBuffers = 0);

			// These functions are called before and after a command is executed
			void ApplyDrawData(Program *program, char *drawData) const;

			void ExecuteDrawCmd(const DrawCmd &);
			void ExecuteDrawCmd2(const DrawCmd2 &);
			void ExecuteDynamicDrawCmd(const DynamicDrawCmd &);
			void ExecuteRenderPassCmd(const RenderPassCmd &);
			void ExecuteBlitRenderTargetCmd(const BlitRenderTargetCmd &);

			static BufferBinding<UniformBuffer> *getBufferBindings(const Shader *shader, char *data);
			static TextureGL **getTextureBindings(const Shader *shader, char *data);

			// 16k-sized buckets; we're not likely to have 100s of command lists
			// (and if we do it's still a drop in the bucket)
			static constexpr size_t BUCKET_SIZE = 1UL << 14;
			struct DataBucket {
				std::unique_ptr<char[]> data;
				size_t used = 0;
				size_t capacity = BUCKET_SIZE;

				char *alloc(size_t size)
				{
					char *ret = nullptr;
					if (capacity - used >= size) {
						ret = data.get() + used;
						used += size;
					}
					return ret;
				}
			};

			Graphics::RendererOGL *m_renderer;
			std::vector<Cmd> m_drawCmds;
			std::vector<DataBucket> m_dataBuckets;
			bool m_executing = false;
		};

	}; // namespace OGL
};	   // namespace Graphics
