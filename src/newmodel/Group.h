#ifndef _GROUP_H
#define _GROUP_H

#include "Node.h"
#include <vector>

namespace Newmodel
{

class Group : public Node
{
public:
	Group() { }
	virtual void AddChild(Node *child);
	virtual void Render(Graphics::Renderer *r);
	virtual void Accept(NodeVisitor &v);
protected:
	virtual ~Group();
	std::vector<Node *> m_children;
};

}

#endif