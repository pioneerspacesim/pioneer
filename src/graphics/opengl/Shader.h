// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "OpenGLLibs.h"
#include "Uniform.h"
#include "graphics/Material.h"
#include "graphics/Texture.h"
#include "graphics/Types.h"

#include <map>
#include <string>
#include <vector>

namespace Graphics {
	namespace OGL {

		struct ShaderException {};

		struct PushConstantData {
			size_t name;
			uint32_t index;
			// uniform location attribute
			GLuint binding;
			// Offset of the given push constant in the binary blob of constant data
			uint32_t offset;
			// Data format of this specific push constant
			ConstantDataFormat format;
		};

		struct TextureBindingData {
			size_t name;
			uint32_t index;
			// texture unit location
			GLuint binding;
			// Data format of this texture sampler binding (e.g. sampler2d)
			TextureType format;
		};

		struct BufferBindingData {
			size_t name;
			uint32_t index;
			// uniform location attribute
			GLuint binding;
			// reserved for future implementation (e.g. size / binding set)
			uint32_t _unused;
		};

		struct ProgramDef {
			std::string name;
			std::string vertexShader;
			std::string fragmentShader;
			std::string defines;
		};

		class Program;

		/*
		* The Shader serves as a reflection interface to a compiled shader pipeline / material.
		* It's responsible for storing the names, locations, and types of the graphics API's
		* data interface with the renderer.
		*
		* It's responsible for selecting the correct Program when rendering, based on runtime
		* renderer state (e.g. number of lights, render quality settings, etc.)
		*/
		class Shader {
		public:
			static constexpr GLuint InvalidBinding = UINT32_MAX;

			Shader();
			Shader(const std::string &name);
			virtual ~Shader();

			void Reload();

			Program *GetProgramForDesc(const MaterialDescriptor &desc);
			uint32_t GetNumVariants() const { return m_variants.size(); }

			TextureBindingData GetTextureBindingInfo(size_t name) const;
			size_t GetNumTextureBindings() const { return m_textureBindingInfo.size(); }
			const std::vector<TextureBindingData> &GetTextureBindings() const { return m_textureBindingInfo; }

			PushConstantData GetPushConstantInfo(size_t name) const;
			size_t GetConstantStorageSize() const { return m_constantStorageSize; }
			size_t GetNumPushConstants() const { return m_pushConstantInfo.size(); }
			const std::vector<PushConstantData> &GetPushConstantBindings() const { return m_pushConstantInfo; }

			BufferBindingData GetBufferBindingInfo(size_t name) const;
			size_t GetNumBufferBindings() const { return m_bufferBindingInfo.size(); }
			const std::vector<BufferBindingData> &GetBufferBindings() const { return m_bufferBindingInfo; }

			const std::string &GetString(size_t name) const { return m_nameMap.at(name); }

		protected:
			friend class Program;

			std::string GetProgramDefines(const MaterialDescriptor &desc);
			Program *LoadProgram(const MaterialDescriptor &desc);

			size_t AddTextureBinding(const std::string &name, TextureType type, uint32_t binding);
			size_t AddConstantBinding(const std::string &name, ConstantDataFormat format, uint32_t binding);
			size_t AddBufferBinding(const std::string &name, uint32_t binding);

		protected:
			ProgramDef m_programDef;
			uint32_t m_constantStorageSize;
			std::vector<std::pair<MaterialDescriptor, Program *>> m_variants;

			std::vector<TextureBindingData> m_textureBindingInfo;
			std::vector<PushConstantData> m_pushConstantInfo;
			std::vector<BufferBindingData> m_bufferBindingInfo;

			std::map<size_t, std::string> m_nameMap;
		};

	} // namespace OGL
} // namespace Graphics
