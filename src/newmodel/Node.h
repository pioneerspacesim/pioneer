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

	//Render is used for the generic draw traversal, even if the
	//node does not actually render anything.
	virtual void Render(Graphics::Renderer *r) { }
	virtual void Accept(NodeVisitor &v);
protected:
	//can only to be deleted using DecRefCount
	virtual ~Node() { }
};

}

#endif