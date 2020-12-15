// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MaterialGL.h"
#include "Program.h"
#include "RendererGL.h"

namespace Graphics {
	namespace OGL {

		void Material::Apply()
		{
			m_program->Use();
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
			const matrix4x4f ViewProjection = proj * mv;
			const matrix3x3f orient(mv.GetOrient());
			const matrix3x3f NormalMatrix(orient.Inverse());

			m_program->uViewMatrix.Set(mv);
			m_program->uViewMatrixInverse.Set(mv.Inverse());
			m_program->uViewProjectionMatrix.Set(ViewProjection);
			m_program->uNormalMatrix.Set(NormalMatrix);
			CHECKERRORS();
		}

	} // namespace OGL
} // namespace Graphics
