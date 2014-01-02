// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Node.h"
#include "NodeVisitor.h"
#include "graphics/Renderer.h"

namespace SceneGraph {

Node::Node(Graphics::Renderer *r)
: m_name("")
, m_nodeMask(NODE_SOLID)
, m_renderer(r)
{
}

Node::Node(Graphics::Renderer *r, unsigned int nodemask)
: m_name("")
, m_nodeMask(nodemask)
, m_renderer(r)
{
}

Node::Node(const Node &node, NodeCopyCache *cache)
: m_name(node.m_name)
, m_nodeMask(node.m_nodeMask)
, m_renderer(node.m_renderer)
{
}

void Node::Accept(NodeVisitor &v)
{
	v.ApplyNode(*this);
}

void Node::Traverse(NodeVisitor &v)
{
}

Node* Node::FindNode(const std::string &name)
{
	if (m_name == name)
		return this;
	else
		return 0;
}

void Node::DrawAxes()
{
	//Draw plain XYZ axes using the current transform
	const vector3f vtsXYZ[] = {
		vector3f(0.f, 0.f, 0.f),
		vector3f(1.f, 0.f, 0.f),
		vector3f(0.f, 0.f, 0.f),
		vector3f(0.f, 1.f, 0.f),
		vector3f(0.f, 0.f, 0.f),
		vector3f(0.f, 0.f, 1.f),
	};
	const Color colors[] = {
		Color::RED,
		Color::RED,
		Color::BLUE,
		Color::BLUE,
		Color::GREEN,
		Color::GREEN,
	};

	m_renderer->SetBlendMode(Graphics::BLEND_SOLID);
	m_renderer->DrawLines(6, vtsXYZ, colors);
}

}
