#include "Group.h"
#include "NodeVisitor.h"

namespace Newmodel {

Group::~Group()
{
	for(std::vector<Node*>::iterator itr = m_children.begin();
		itr != m_children.end();
		++itr)
	{
		(*itr)->DecRefCount();
	}
}

void Group::AddChild(Node *child)
{
	child->IncRefCount();
	m_children.push_back(child);
}

void Group::Accept(NodeVisitor &nv)
{
	nv.ApplyGroup(*this);
	for(std::vector<Node*>::iterator itr = m_children.begin();
		itr != m_children.end();
		++itr)
	{
		(*itr)->Accept(nv);
	}
}

}