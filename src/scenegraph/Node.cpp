// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Node.h"
#include "NodeVisitor.h"
#include "graphics/Renderer.h"

namespace SceneGraph {

Node::Node()
: m_name("")
, m_nodeMask(NODE_SOLID)
{
}

Node::Node(unsigned int nodemask)
: m_name("")
, m_nodeMask(nodemask)
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

void Node::DrawAxes(Graphics::Renderer *r)
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

	r->SetBlendMode(Graphics::BLEND_SOLID);
	r->DrawLines(6, vtsXYZ, colors);
}

}
