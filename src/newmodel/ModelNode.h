#ifndef _MODELNODE_H
#define _MODELNODE_H
/*
 *	Use another model as a submodel
 */
#include "Node.h"
#include "Model.h"

namespace Newmodel {

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
