// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MaterialGL.h"
#include "Program.h"
#include "RendererGL.h"

namespace Graphics {
namespace OGL {

void Material::Apply()
{
	m_program->Use();
	m_program->invLogZfarPlus1.Set(m_renderer->m_invLogZfarPlus1);
}

void Material::Unapply()
{
}

void Material::SetCommonUniforms(const matrix4x4f& mv, const matrix4x4f& proj)
{
	const matrix4x4f ViewProjection = proj * mv;
	const matrix3x3f orient(mv.GetOrient());
	const matrix4x4f NormalMatrix(orient.Inverse().Transpose());
	//const matrix4x4f NormalMatrix(mv.Inverse().Transpose());

	m_program->uProjectionMatrix.Set( proj );
	m_program->uViewMatrix.Set( mv );
	m_program->uViewMatrixInverse.Set( mv.Inverse() );
	m_program->uViewProjectionMatrix.Set( ViewProjection );
	m_program->uNormalMatrix.Set( NormalMatrix );
	CheckRenderErrors();
}

}
}
