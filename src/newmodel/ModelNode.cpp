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
	//XXX argh
	ModelParams par;
	par.screenWidth = rd->scrWidth;
	m_model->Render(r, trans, &par);
}

}