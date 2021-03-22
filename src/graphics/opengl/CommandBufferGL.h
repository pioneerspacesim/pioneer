
#pragma once

#include "graphics/Types.h"
#include "graphics/VertexBuffer.h"

namespace Graphics {

	class Material;
	class MeshObject;
	class VertexArray;

	class RendererOGL;

	namespace OGL {

		class InstanceBuffer;
		class Material;
		class MeshObject;
		class Program;
		class Shader;
		class TextureGL;
		class UniformBuffer;
		struct UniformBufferBinding;

		class CommandList {
		public:
			struct DrawCmd {
				MeshObject *mesh;
				InstanceBuffer *inst = nullptr;
				const Shader *shader = nullptr;
				Program *program = nullptr;
				size_t renderStateHash = 0;
				char *drawData;
			};

			void AddDrawCmd(Graphics::MeshObject *mesh, Graphics::Material *mat, Graphics::InstanceBuffer *inst = nullptr);
			const std::vector<DrawCmd> &GetDrawCmds() const { return m_drawCmds; }

			bool IsEmpty() const { return m_drawCmds.empty(); }
			void Reset();

		private:
			friend class Graphics::RendererOGL;
			CommandList()
			{
				m_drawCmds.reserve(32);
			}

			// Allocate space for all shader data that needs to be cached forward
			char *AllocDrawData(const Shader *shader);
			// Create and cache all material data needed for later execution of a draw command
			char *SetupMaterialData(OGL::Material *mat);

			// These functions are called before and after a command is executed
			void ApplyDrawData(const DrawCmd &cmd) const;
			void CleanupDrawData(const DrawCmd &cmd) const;

			static UniformBufferBinding *getBufferBindings(const Shader *shader, char *data);
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

			std::vector<DrawCmd> m_drawCmds;
			std::vector<DataBucket> m_dataBuckets;
			bool m_executing = false;
		};

	}; // namespace OGL
};	   // namespace Graphics
