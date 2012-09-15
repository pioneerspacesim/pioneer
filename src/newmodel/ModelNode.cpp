#include "ModelNode.h"
#include "NModel.h"

namespace Newmodel {

ModelNode::ModelNode(Model *m) :
	Node(),
	m_model(m)
{
}

void ModelNode::Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd)
{
	//slight hack here
	rd->nodemask |= MASK_IGNORE;
	m_model->Render(r, trans, rd);
	rd->nodemask &= ~MASK_IGNORE;
}

}
