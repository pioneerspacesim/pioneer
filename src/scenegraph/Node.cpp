// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Node.h"
#include "NodeVisitor.h"
#include "Serializer.h"
#include "graphics/Drawables.h"
#include "graphics/Renderer.h"

namespace SceneGraph {

	Node::Node(Graphics::Renderer *r) :
		m_name(""),
		m_nodeMask(NODE_SOLID),
		m_nodeFlags(0),
		m_renderer(r),
		m_parent(nullptr)
	{
	}

	Node::Node(Graphics::Renderer *r, unsigned int nodemask) :
		m_name(""),
		m_nodeMask(nodemask),
		m_nodeFlags(0),
		m_renderer(r),
		m_parent(nullptr)
	{
	}

	Node::Node(const Node &node, NodeCopyCache *cache) :
		m_name(node.m_name),
		m_nodeMask(node.m_nodeMask),
		m_nodeFlags(node.m_nodeFlags),
		m_renderer(node.m_renderer),
		m_parent(nullptr)
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

	Node *Node::FindNode(const std::string &name)
	{
		if (m_name == name)
			return this;
		else
			return nullptr;
	}

	void Node::DrawAxes()
	{
		Graphics::Drawables::Axes3D *axes = Graphics::Drawables::GetAxes3DDrawable(m_renderer);
		axes->Draw(m_renderer);
	}

	void Node::Save(NodeDatabase &db)
	{
		db.wr->String(GetTypeName());
		db.wr->String(m_name.c_str());
		db.wr->Int32(m_nodeMask);
		db.wr->Int32(m_nodeFlags);
	}

} // namespace SceneGraph
