// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MatrixTransform.h"
#include "NodeVisitor.h"
#include "NodeCopyCache.h"
#include "BaseLoader.h"
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

void MatrixTransform::Render(const matrix4x4f &trans, const RenderData *rd)
{
	const matrix4x4f t = trans * m_transform;
	//renderer->SetTransform(t);
	//DrawAxes();
	RenderChildren(t, rd);
}

void MatrixTransform::Save(NodeDatabase &db)
{
	Group::Save(db);
	for (Uint32 i = 0; i < 16; i++)
		db.wr->Float(m_transform[i]);
}

MatrixTransform *MatrixTransform::Load(NodeDatabase &db)
{
	matrix4x4f matrix;
	for (Uint32 i = 0; i < 16; i++)
		matrix[i] = db.rd->Float();
	MatrixTransform *mt = new MatrixTransform(db.loader->GetRenderer(), matrix);
	return mt;
}

}
