// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MODELNODE_H
#define _MODELNODE_H
/*
 *	Use another model as a submodel
 */
#include "Node.h"
#include "Model.h"

namespace SceneGraph {

class ModelNode : public Node {
public:
	ModelNode(Model *m);
	virtual const char *GetTypeName() { return "ModelNode"; }
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd);

protected:
	virtual ~ModelNode() { }

private:
	Model *m_model;
};

}

#endif
