#ifndef _NODE_H
#define _NODE_H

#include "RefCounted.h"

namespace Graphics { class Renderer; }

namespace Newmodel
{

class Node : public RefCounted
{
public:
	Node() { }
	Node *m_parent;

	//Render is used for the generic draw traversal, even if the
	//node does not actually render anything.
	virtual void Render(Graphics::Renderer *r) { }

protected:
	//can only to be deleted using DecRefCount
	virtual ~Node() { }
};

}

#endif