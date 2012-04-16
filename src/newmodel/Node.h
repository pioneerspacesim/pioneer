#ifndef _NODE_H
#define _NODE_H
/*
 * Generic node for the model scenegraph
 */
#include "RefCounted.h"

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
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans) { }
protected:
	//can only to be deleted using DecRefCount
	virtual ~Node() { }
};

}

#endif