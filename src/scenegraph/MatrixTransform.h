// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MATRIXTRANSFORM_H
#define _MATRIXTRANSFORM_H
/*
 * Applies a matrix transform to child nodes
 */
#include "Group.h"
#include "matrix4x4.h"
namespace Graphics { class Renderer; }

namespace SceneGraph {
class MatrixTransform : public Group {
public:
	MatrixTransform(Graphics::Renderer *r, const matrix4x4f &m);
	MatrixTransform(const MatrixTransform&, NodeCopyCache *cache = 0);
	virtual Node *Clone(NodeCopyCache *cache = 0);
	virtual const char *GetTypeName() const { return "MatrixTransform"; }
	virtual void Accept(NodeVisitor &v);
	virtual void Save(NodeDatabase&) override;
	static MatrixTransform *Load(NodeDatabase&);
	void Render(const matrix4x4f &trans, const RenderData *rd);
	const matrix4x4f &GetTransform() const { return m_transform; }
	void SetTransform(const matrix4x4f &m) { m_transform = m; }

protected:
	virtual ~MatrixTransform() { }

private:
	matrix4x4f m_transform;
};
}
#endif
