// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelNode.h"
#include "Model.h"

namespace SceneGraph {

ModelNode::ModelNode(Model *m)
: Node(m->GetRenderer())
, m_model(m)
{
}

ModelNode::ModelNode(const ModelNode &modelNode, NodeCopyCache *cache)
: Node(modelNode, cache)
, m_model(modelNode.m_model)
{
}

Node* ModelNode::Clone(NodeCopyCache *cache)
{
	return this; //modelnodes are shared
}

void ModelNode::Render(const matrix4x4f &trans, const RenderData *rd)
{
	//slight hack here
	RenderData newrd = *rd;
	newrd.nodemask |= MASK_IGNORE;
	m_model->Render(trans, &newrd);
}

}
