// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MaterialGL.h"
#include "Program.h"
#include "RendererGL.h"
#include "graphics/Material.h"

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

		void Material::Apply()
		{
			m_program->Use();
			if (GetDescriptor().lighting)
				m_renderer->GetLightUniformBuffer()->Bind(0);

			auto buffer = m_renderer->GetDrawUniformBuffer(sizeof(DrawDataBlock));
			{
				auto dataBlock = buffer->Allocate<DrawDataBlock>(1);
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

		void Material::Unapply()
		{
		}

		bool Material::IsProgramLoaded() const
		{
			return m_program->Loaded();
		}

		void Material::SetCommonUniforms(const matrix4x4f &mv, const matrix4x4f &proj)
		{
			CHECKERRORS();
		}

	} // namespace OGL
} // namespace Graphics
