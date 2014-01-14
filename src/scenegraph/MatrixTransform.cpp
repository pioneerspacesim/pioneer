// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MatrixTransform.h"
#include "NodeVisitor.h"
#include "NodeCopyCache.h"
#include "graphics/Renderer.h"
namespace SceneGraph {

MatrixTransform::MatrixTransform(Graphics::Renderer *r, const matrix4x4f &m)
: Group(r)
, m_transform(m)
{
}

MatrixTransform::MatrixTransform(const MatrixTransform &mt, NodeCopyCache *cache)
: Group(mt, cache)
, m_transform(mt.m_transform)
{
}

Node* MatrixTransform::Clone(NodeCopyCache *cache)
{
	return cache->Copy<MatrixTransform>(this);
}

void MatrixTransform::Accept(NodeVisitor &nv)
{
	nv.ApplyMatrixTransform(*this);
}

Node* MatrixTransform::GatherTransforms(const std::string &name, const matrix4x4f &accum, matrix4x4f &outMat)
{
	const matrix4x4f t = accum * m_transform;
	if (m_name == name) {
		outMat = t;
		return this;
	}

	Node* result = 0;
	for (std::vector<Node*>::iterator itr = m_children.begin(), itEnd = m_children.end(); itr != itEnd; ++itr)
	{
		result = (*itr)->GatherTransforms(name, t, outMat);
		if (result) break;
	}

	return result;
}

void MatrixTransform::Render(const matrix4x4f &trans, const RenderData *rd)
{
	const matrix4x4f t = trans * m_transform;
	//renderer->SetTransform(t);
	//DrawAxes();
	RenderChildren(t, rd);
}

}
