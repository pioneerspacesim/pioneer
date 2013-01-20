// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MatrixTransform.h"
#include "NodeVisitor.h"
#include "graphics/Renderer.h"
namespace SceneGraph {

MatrixTransform::MatrixTransform(const matrix4x4f &m)
: Group()
, m_transform(m)
{
}

void MatrixTransform::Accept(NodeVisitor &nv)
{
	nv.ApplyMatrixTransform(*this);
}

void MatrixTransform::Render(Graphics::Renderer *renderer, const matrix4x4f &trans, RenderData *rd)
{
	const matrix4x4f t = trans * m_transform;
	//renderer->SetTransform(t);
	//DrawAxes(renderer);
	RenderChildren(renderer, t, rd);
}

}
