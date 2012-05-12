#ifndef _NODE_H
#define _NODE_H
/*
 * Generic node for the model scenegraph
 */
#include "libs.h"
#include "RefCounted.h"
#include "LmrTypes.h" //for renderdata

namespace Graphics { class Renderer; }

namespace Newmodel
{

class NodeVisitor;

class Node : public RefCounted
{
public:
	Node() { }
	Node *m_parent;

	virtual void Accept(NodeVisitor &v);
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd) { }
protected:
	//can only to be deleted using DecRefCount
	virtual ~Node() { }
};

}

#endif
