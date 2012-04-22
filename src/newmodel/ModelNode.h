#ifndef _MODELNODE_H
#define _MODELNODE_H
/*
 *	Use another model as a submodel
 */
#include "Node.h"

namespace Newmodel {

class NModel;

class ModelNode : public Node {
public:
	ModelNode(Model *m);
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans);

protected:
	virtual ~ModelNode() { }

private:
	Model *m_model;
};

}

#endif