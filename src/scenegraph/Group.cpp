// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Group.h"
#include "NodeVisitor.h"

namespace SceneGraph {

Group::Group()
: Node(NODE_SOLID | NODE_TRANSPARENT)
{
}

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

bool Group::RemoveChild(Node *node)
{
	if (!node) return false;
	for(std::vector<Node*>::iterator itr = m_children.begin();
		itr != m_children.end();
		++itr)
	{
		if((*itr) == node) {
			itr = m_children.erase(itr);
			node->DecRefCount();
			return true;
		}
	}
	return false;
}

bool Group::RemoveChildAt(unsigned int idx)
{
	if (m_children.empty() || idx > m_children.size() - 1) return false;
	Node *node = m_children.at(idx);
	node->DecRefCount();
	m_children.erase(m_children.begin() + idx);
	return true;
}

Node* Group::FindNode(const std::string &name)
{
	if (m_name == name)
		return this;

	Node* result = 0;
	for(std::vector<Node*>::iterator itr = m_children.begin();
		itr != m_children.end();
		++itr)
	{
		result = (*itr)->FindNode(name);
		if (result) break;
	}

	return result;
}

void Group::Accept(NodeVisitor &nv)
{
	nv.ApplyGroup(*this);
}

void Group::Traverse(NodeVisitor &nv)
{
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
		if((*itr)->GetNodeMask() & rd->nodemask)
			(*itr)->Render(r, trans, rd);
	}
}

}
