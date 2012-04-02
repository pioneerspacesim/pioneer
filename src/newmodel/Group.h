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
protected:
	virtual ~Group();
	std::vector<Node *> m_children;
};

}

#endif