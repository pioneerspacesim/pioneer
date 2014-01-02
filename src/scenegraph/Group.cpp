// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Group.h"
#include "NodeVisitor.h"
#include "NodeCopyCache.h"

namespace SceneGraph {

Group::Group(Graphics::Renderer *r)
: Node(r, NODE_SOLID | NODE_TRANSPARENT)
{
}

Group::~Group()
{
	for(std::vector<Node*>::iterator itr = m_children.begin(), itEnd = m_children.end(); itr != itEnd; ++itr)
	{
		(*itr)->DecRefCount();
	}
}

Group::Group(const Group &group, NodeCopyCache *cache)
: Node(group, cache)
{
	for(std::vector<Node*>::const_iterator itr = group.m_children.begin();
		itr != group.m_children.end();
		++itr)
	{
		Node *node = (*itr)->Clone(cache);
		AddChild(node);
	}
}

Node* Group::Clone(NodeCopyCache *cache)
{
	return cache->Copy<Group>(this);
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

Node* Group::GetChildAt(unsigned int idx)
{
	return m_children.at(idx);
}

Node* Group::FindNode(const std::string &name)
{
	if (m_name == name)
		return this;

	Node* result = 0;
	for(std::vector<Node*>::iterator itr = m_children.begin(), itEnd = m_children.end(); itr != itEnd; ++itr)
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
	for(std::vector<Node*>::iterator itr = m_children.begin(), itEnd = m_children.end(); itr != itEnd; ++itr)
	{
		(*itr)->Accept(nv);
	}
}

void Group::Render(const matrix4x4f &trans, const RenderData *rd)
{
	RenderChildren(trans, rd);
}

void Group::RenderChildren(const matrix4x4f &trans, const RenderData *rd)
{
	for(std::vector<Node*>::iterator itr = m_children.begin(), itEnd = m_children.end(); itr != itEnd; ++itr)
	{
		if((*itr)->GetNodeMask() & rd->nodemask)
			(*itr)->Render(trans, rd);
	}
}

}
