#ifndef _MATRIXTRANSFORM_H
#define _MATRIXTRANSFORM_H
/*
 * Applies a matrix transform to child nodes
 */
#include "Group.h"
#include "matrix4x4.h"
namespace Graphics { class Renderer; }

namespace Newmodel {
class MatrixTransform : public Group {
public:
	MatrixTransform(const matrix4x4f &m);
	void Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd);

protected:
	virtual ~MatrixTransform() { }

private:
	matrix4x4f m_transform;
};
}
#endif