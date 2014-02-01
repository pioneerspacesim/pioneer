// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Node.h"
#include "NodeVisitor.h"
#include "graphics/Renderer.h"

namespace SceneGraph {

Node::Node(Graphics::Renderer *r)
: m_name("")
, m_nodeMask(NODE_SOLID)
, m_nodeFlags(0)
, m_renderer(r)
{
}

Node::Node(Graphics::Renderer *r, unsigned int nodemask)
: m_name("")
, m_nodeMask(nodemask)
, m_nodeFlags(0)
, m_renderer(r)
{
}

Node::Node(const Node &node, NodeCopyCache *cache)
: m_name(node.m_name)
, m_nodeMask(node.m_nodeMask)
, m_nodeFlags(node.m_nodeFlags)
, m_renderer(node.m_renderer)
{
}

Node::~Node()
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

	Graphics::RenderStateDesc rsd;
	m_renderer->DrawLines(6, vtsXYZ, colors, m_renderer->CreateRenderState(rsd));
}

void Node::Save(NodeDatabase &db)
{
    db.wr->String(GetTypeName());
	db.wr->String(m_name.c_str());
    db.wr->Int32(m_nodeMask);
    db.wr->Int32(m_nodeFlags);
}

}
