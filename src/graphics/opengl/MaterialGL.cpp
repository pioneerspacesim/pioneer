// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MaterialGL.h"
#include "Program.h"
#include "RendererGL.h"
#include "Shader.h"
#include "core/Log.h"
#include "graphics/Material.h"

#include "StringF.h"
#include "graphics/Types.h"
#include "graphics/opengl/TextureGL.h"

namespace Graphics {
	namespace OGL {

		struct DrawDataBlock {
			// matrix data
			matrix4x4f uViewMatrix;
			matrix4x4f uViewMatrixInverse;
			matrix4x4f uViewProjectionMatrix;

			// Material Struct
			Color4f diffuse;
			Color4f specular;
			Color4f emission;

			// Scene struct
			Color4f ambient;
		};
		static_assert(sizeof(DrawDataBlock) == 256, "");

		static size_t s_lightDataName = Renderer::GetName("LightData");
		static size_t s_drawDataName = Renderer::GetName("DrawData");
		static size_t s_lightIntensityName = Renderer::GetName("lightIntensity");

		// XXX potential target for allocation optimization
		// (e.g. use renderer-owned freelist block allocator and align and group all three buffers into one allocation?)
		void Material::SetShader(Shader *s)
		{
			m_shader = s;
			m_activeVariant = s->GetProgramForDesc(GetDescriptor());

			// Allocate storage for texture bindings
			GLuint numTextureBindings = s->GetNumTextureBindings();
			if (numTextureBindings) {
				m_textureBindings.reset(new Texture *[numTextureBindings]);
				for (GLuint i = 0; i < numTextureBindings; i++)
					m_textureBindings.get()[i] = nullptr;
			}

			// Allocate storage for buffer bindings
			GLuint numBufferBindings = s->GetNumBufferBindings();
			if (numBufferBindings) {
				m_bufferBindings.reset(new BufferBinding[numBufferBindings]);
				for (GLuint i = 0; i < numBufferBindings; i++)
					m_bufferBindings.get()[i] = { RefCountedPtr<UniformBuffer>(), 0, 0 };
			}

			// Allocate storage for push constants
			GLuint constantStorageSize = s->GetConstantStorageSize();
			if (constantStorageSize) {
				m_pushConstants.reset(new char[constantStorageSize]);
				std::fill_n(m_pushConstants.get(), constantStorageSize, '\0');
			}

			m_perDrawBinding = s->GetBufferBindingInfo(s_drawDataName).binding;
		}

		void Material::EvaluateVariant()
		{
			MaterialDescriptor desc = GetDescriptor();
			bool variantChanged = false;
			uint32_t numLights = m_renderer->GetNumLights();

			// TODO: need a slightly better (and more generic) way to evaluate and generate state variations

			//request a new light variation if the number of lights has changed
			if (desc.lighting && desc.dirLights != numLights) {
				desc.dirLights = numLights;
				variantChanged = true;
			}

			if (variantChanged) {
				Program *p = m_shader->GetProgramForDesc(desc);
				if (p->Loaded())
					m_activeVariant = p;
			}
		}

		void Material::UpdateDrawData()
		{
			if (m_descriptor.lighting) {
				UniformBuffer *lightBuffer = m_renderer->GetLightUniformBuffer();
				SetBuffer(s_lightDataName, lightBuffer, 0, lightBuffer->GetSize());

				float intensity[4] = { 0.f, 0.f, 0.f, 0.f };
				for (uint32_t i = 0; i < m_renderer->GetNumLights(); i++)
					intensity[i] = m_renderer->GetLight(i).GetIntensity();

				SetPushConstant(s_lightIntensityName, Color4f(intensity[0], intensity[1], intensity[2], intensity[3]));
			}

			// this should always be present, but just in case...
			if (m_perDrawBinding != Shader::InvalidBinding) {
				auto buffer = m_renderer->GetDrawUniformBuffer(sizeof(DrawDataBlock));
				auto dataBlock = buffer->Allocate<DrawDataBlock>(m_perDrawBinding);
				dataBlock->diffuse = this->diffuse.ToColor4f();
				dataBlock->specular = this->specular.ToColor4f();
				dataBlock->specular.a = this->shininess;
				dataBlock->emission = this->emissive.ToColor4f();
				dataBlock->ambient = m_renderer->GetAmbientColor().ToColor4f();

				// We handle the normal matrix by transposing the orientation part of the inverse view matrix in the shader
				const matrix4x4f &mv = m_renderer->GetTransform();
				const matrix4x4f &proj = m_renderer->GetProjection();
				dataBlock->uViewMatrix = mv;
				dataBlock->uViewMatrixInverse = mv.Inverse();
				dataBlock->uViewProjectionMatrix = proj * mv;
			}
		}

		void Material::Apply()
		{
			PROFILE_SCOPED()
			EvaluateVariant();
			UpdateDrawData();

			m_activeVariant->Use();

			for (auto &info : m_shader->GetBufferBindings()) {
				BufferBinding &bind = m_bufferBindings[info.index];
				if (bind.buffer)
					bind.buffer->BindRange(info.binding, bind.offset, bind.size);
			}

			for (auto &info : m_shader->GetTextureBindings()) {
				glActiveTexture(GL_TEXTURE0 + info.binding);
				if (m_textureBindings[info.index])
					m_textureBindings[info.index]->Bind();
				else
					glBindTexture(GL_TEXTURE_2D, 0);
			}

			for (auto &info : m_shader->GetPushConstantBindings()) {
				GLuint location = m_activeVariant->GetConstantLocation(info.binding);
				if (location == GL_INVALID_INDEX)
					continue;

				Uniform setter(location);
				switch (info.format) {
				case ConstantDataFormat::DATA_FORMAT_INT:
					setter.Set(*reinterpret_cast<int *>(m_pushConstants.get() + info.offset));
					break;
				case ConstantDataFormat::DATA_FORMAT_FLOAT:
					setter.Set(*reinterpret_cast<float *>(m_pushConstants.get() + info.offset));
					break;
				case ConstantDataFormat::DATA_FORMAT_FLOAT3:
					setter.Set(*reinterpret_cast<vector3f *>(m_pushConstants.get() + info.offset));
					break;
				case ConstantDataFormat::DATA_FORMAT_FLOAT4:
					setter.Set(*reinterpret_cast<Color4f *>(m_pushConstants.get() + info.offset));
					break;
				case ConstantDataFormat::DATA_FORMAT_MAT3:
					setter.Set(*reinterpret_cast<matrix3x3f *>(m_pushConstants.get() + info.offset));
					break;
				case ConstantDataFormat::DATA_FORMAT_MAT4:
					setter.Set(*reinterpret_cast<matrix4x4f *>(m_pushConstants.get() + info.offset));
					break;
				default:
					assert(false);
					break;
				}
			}
		}

		void Material::Unapply()
		{
			// Push constants (glUniforms) don't need to be unbound
			// Uniform buffers also don't need to be unbound

			// Unbinding textures is probably also not needed, (and not performant)
			// but included here to ensure we don't have any state leakage
			for (auto &info : m_shader->GetTextureBindings()) {
				if (m_textureBindings[info.index]) {
					glActiveTexture(GL_TEXTURE0 + info.binding);
					m_textureBindings[info.index]->Unbind();
				}
			}
		}

		bool Material::IsProgramLoaded() const
		{
			return m_activeVariant && m_activeVariant->Loaded();
		}

		// Copy all material data from this material into another, possibly different one
		void Material::Copy(OGL::Material *mat) const
		{
			mat->diffuse = diffuse;
			mat->specular = specular;
			mat->emissive = emissive;
			mat->shininess = shininess;

			for (auto &texBinding : m_shader->GetTextureBindings()) {
				mat->SetTexture(texBinding.name, m_textureBindings.get()[texBinding.index]);
			}

			for (auto &bufferBinding : m_shader->GetBufferBindings()) {
				auto &buffer = m_bufferBindings.get()[bufferBinding.index];
				// clone the buffer reference if present
				if (buffer.buffer)
					mat->SetBuffer(bufferBinding.name, buffer.buffer.Get(), buffer.offset, buffer.size);
			}

			for (auto &info : m_shader->GetPushConstantBindings()) {
				switch (info.format) {
				case ConstantDataFormat::DATA_FORMAT_INT:
					mat->SetPushConstant(info.name, *reinterpret_cast<int *>(m_pushConstants.get() + info.offset));
					break;
				case ConstantDataFormat::DATA_FORMAT_FLOAT:
					mat->SetPushConstant(info.name, *reinterpret_cast<float *>(m_pushConstants.get() + info.offset));
					break;
				case ConstantDataFormat::DATA_FORMAT_FLOAT3:
					mat->SetPushConstant(info.name, *reinterpret_cast<vector3f *>(m_pushConstants.get() + info.offset));
					break;
				case ConstantDataFormat::DATA_FORMAT_FLOAT4:
					mat->SetPushConstant(info.name, *reinterpret_cast<Color4f *>(m_pushConstants.get() + info.offset));
					break;
				case ConstantDataFormat::DATA_FORMAT_MAT3:
					mat->SetPushConstant(info.name, *reinterpret_cast<matrix3x3f *>(m_pushConstants.get() + info.offset));
					break;
				case ConstantDataFormat::DATA_FORMAT_MAT4:
					mat->SetPushConstant(info.name, *reinterpret_cast<matrix4x4f *>(m_pushConstants.get() + info.offset));
					break;
				default:
					assert(false);
					break;
				}
			}
		}

		bool Material::SetTexture(size_t name, Texture *tex)
		{
			TextureBindingData info = m_shader->GetTextureBindingInfo(name);
			if (info.binding == Shader::InvalidBinding || (tex && info.format != tex->GetDescriptor().type))
				return false;

			m_textureBindings.get()[info.index] = tex;
			return true;
		}

		bool Material::SetBuffer(size_t name, void *buffer, size_t size, BufferUsage usage)
		{
			BufferBindingData info = m_shader->GetBufferBindingInfo(name);
			if (info.binding == Shader::InvalidBinding)
				return false;

			auto &bufferSlot = m_bufferBindings[info.index];

			if (usage == BUFFER_USAGE_DYNAMIC) {
				auto allocation = m_renderer->GetDrawUniformBuffer(size)->Allocate(buffer, size);
				bufferSlot.buffer.Reset(allocation.buffer);
				bufferSlot.offset = allocation.offset;
				bufferSlot.size = allocation.size;
			} else {
				UniformBuffer *buffer = m_renderer->CreateUniformBuffer(size, BUFFER_USAGE_STATIC);
				buffer->BufferData(size, buffer);

				bufferSlot.buffer.Reset(buffer);
				bufferSlot.offset = 0;
				bufferSlot.size = size;
			}

			return true;
		}

		bool Material::SetBuffer(size_t name, UniformBuffer *buffer, uint32_t offset, uint32_t size)
		{
			BufferBindingData info = m_shader->GetBufferBindingInfo(name);
			if (info.binding == Shader::InvalidBinding)
				return false;

			auto &bufferSlot = m_bufferBindings[info.index];
			bufferSlot.buffer.Reset(buffer);
			bufferSlot.offset = offset;
			bufferSlot.size = size;

			return true;
		}

		// Push Constants use a single compact binary blob allocated at material creation time
		// to cache data for constants.
		// When draw commands are pushed to the hardware, this cache is sent as glUniformX calls
		// in a single batch. This solution is probably overkill, as a material will likely never
		// have more than 64 bytes of push constant data in total.
		// This does future-proof us for vulkan, where push constants are sent as a single binary
		// upload instead of the driver caching glUniform calls
		template <typename T>
		bool SetConstant(size_t name, T &data, ConstantDataFormat format, Shader *p, char *stor)
		{
			PushConstantData info = p->GetPushConstantInfo(name);
			if (info.binding == Shader::InvalidBinding || info.format != format)
				return false;

			*reinterpret_cast<T *>(stor + info.offset) = data;
			return true;
		}

		bool Material::SetPushConstant(size_t name, int i)
		{
			return SetConstant(name, i, ConstantDataFormat::DATA_FORMAT_INT, m_shader, m_pushConstants.get());
		}

		bool Material::SetPushConstant(size_t name, float f)
		{
			return SetConstant(name, f, ConstantDataFormat::DATA_FORMAT_FLOAT, m_shader, m_pushConstants.get());
		}

		bool Material::SetPushConstant(size_t name, vector3f v3)
		{
			return SetConstant(name, v3, ConstantDataFormat::DATA_FORMAT_FLOAT3, m_shader, m_pushConstants.get());
		}

		// this is honestly a horrible hack, but it's easier than trying to fully implement a vector4 type
		bool Material::SetPushConstant(size_t name, vector3f v4, float f4)
		{
			Color4f vector(v4.x, v4.y, v4.z, f4);
			return SetConstant(name, vector, ConstantDataFormat::DATA_FORMAT_FLOAT4, m_shader, m_pushConstants.get());
		}

		bool Material::SetPushConstant(size_t name, Color c)
		{
			Color4f color = c.ToColor4f();
			return SetConstant(name, color, ConstantDataFormat::DATA_FORMAT_FLOAT4, m_shader, m_pushConstants.get());
		}

		bool Material::SetPushConstant(size_t name, matrix3x3f mat3)
		{
			return SetConstant(name, mat3, ConstantDataFormat::DATA_FORMAT_MAT3, m_shader, m_pushConstants.get());
		}

		bool Material::SetPushConstant(size_t name, matrix4x4f mat4)
		{
			return SetConstant(name, mat4, ConstantDataFormat::DATA_FORMAT_MAT4, m_shader, m_pushConstants.get());
		}

	} // namespace OGL
} // namespace Graphics
