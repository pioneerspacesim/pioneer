// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MatrixTransform.h"
#include "BaseLoader.h"
#include "NodeCopyCache.h"
#include "NodeVisitor.h"
#include "Serializer.h"

#include "graphics/Renderer.h"

namespace SceneGraph {

	MatrixTransform::MatrixTransform(Graphics::Renderer *r, const matrix4x4f &m) :
		Group(r),
		m_transform(m)
	{
	}

	MatrixTransform::MatrixTransform(const MatrixTransform &mt, NodeCopyCache *cache) :
		Group(mt, cache),
		m_transform(mt.m_transform)
	{
	}

	Node *MatrixTransform::Clone(NodeCopyCache *cache)
	{
		return cache->Copy<MatrixTransform>(this);
	}

	void MatrixTransform::Accept(NodeVisitor &nv)
	{
		nv.ApplyMatrixTransform(*this);
	}

	matrix4x4f MatrixTransform::CalcGlobalTransform() const
	{
		return GetParent() ? GetParent()->CalcGlobalTransform() * m_transform : m_transform;
	}

	void MatrixTransform::Render(const matrix4x4f &trans, const RenderData *rd)
	{
		const matrix4x4f t = trans * m_transform;
		RenderChildren(t, rd);
	}

	static const matrix4x4f s_ident(matrix4x4f::Identity());
	void MatrixTransform::Render(const std::vector<matrix4x4f> &trans, const RenderData *rd)
	{
		if (0 == memcmp(&m_transform, &s_ident, sizeof(matrix4x4f))) {
			// m_transform is identity so avoid performing all multiplications
			RenderChildren(trans, rd);
		} else {
			// m_transform is valid, modify all positions by it
			const size_t transSize = trans.size();
			std::vector<matrix4x4f> t;
			t.resize(transSize);
			for (size_t tIdx = 0; tIdx < transSize; tIdx++) {
				t[tIdx] = trans[tIdx] * m_transform;
			}
			RenderChildren(t, rd);
		}
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

} // namespace SceneGraph
