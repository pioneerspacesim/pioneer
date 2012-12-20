// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
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
	MatrixTransform(const matrix4x4f &m);
	virtual const char *GetTypeName() { return "MatrixTransform"; }
	virtual void Accept(NodeVisitor &v);
	void Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd);
	const matrix4x4f &GetTransform() const { return m_transform; }
	void SetTransform(const matrix4x4f &m) { m_transform = m; }

protected:
	virtual ~MatrixTransform() { }

private:
	matrix4x4f m_transform;
};
}
#endif
