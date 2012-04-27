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

void Group::Render(Graphics::Renderer *renderer, const matrix4x4f &trans, RenderData *rd)
{
	RenderChildren(renderer, trans, rd);
}

void Group::RenderChildren(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd)
{
	for(std::vector<Node*>::iterator itr = m_children.begin();
		itr != m_children.end();
		++itr)
	{
		(*itr)->Render(r, trans, rd);
	}
}

}