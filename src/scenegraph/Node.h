// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _NODE_H
#define _NODE_H
/*
 * Generic node for the model scenegraph
 */
#include "libs.h"
#include "RefCounted.h"

namespace Graphics { class Renderer; }

namespace SceneGraph
{

class NodeVisitor;
class NodeCopyCache;

enum NodeMask {
	NODE_SOLID = 0x1,
	NODE_TRANSPARENT = 0x2,
	MASK_IGNORE = 0x4
};

//Small structure used internally to pass rendering data
struct RenderData
{
	float linthrust[3];		// 1.0 to -1.0
	float angthrust[3];		// 1.0 to -1.0

	float boundingRadius;	//updated by model and passed to submodels
	unsigned int nodemask;

	RenderData()
	: linthrust()
	, angthrust()
	, boundingRadius(0.f)
	, nodemask(NODE_SOLID) //draw solids
	{
	}
};

class Node : public RefCounted
{
public:
	Node(Graphics::Renderer *r);
	Node(Graphics::Renderer *r, unsigned int nodemask);
	Node(const Node&, NodeCopyCache*);
	virtual Node *Clone(NodeCopyCache*) = 0; //implement clone to return shallow or deep copy
	virtual const char *GetTypeName() const { return "Node"; }
	virtual void Accept(NodeVisitor &v);
	virtual void Traverse(NodeVisitor &v);
	virtual void Render(const matrix4x4f &trans, const RenderData *rd) { }
	void DrawAxes();
	void SetName(const std::string &name) { m_name = name; }
	const std::string &GetName() const { return m_name; }

	virtual Node* FindNode(const std::string &);
	virtual Node* GatherTransforms(const std::string &, const matrix4x4f &, matrix4x4f &);

	unsigned int GetNodeMask() const { return m_nodeMask; }
	void SetNodeMask(unsigned int m) { m_nodeMask = m; }

	Graphics::Renderer *GetRenderer() const { return m_renderer; }

protected:
	//can only to be deleted using DecRefCount
	virtual ~Node() { }
	std::string m_name;
	unsigned int m_nodeMask;
	Graphics::Renderer *m_renderer;
};

}

#endif
