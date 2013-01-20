// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelNode.h"
#include "Model.h"

namespace SceneGraph {

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
