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
	virtual void Accept(NodeVisitor &v);
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans);
protected:
	virtual ~Group();
	virtual void RenderChildren(Graphics::Renderer *r, const matrix4x4f &trans);
	std::vector<Node *> m_children;
};

}

#endif