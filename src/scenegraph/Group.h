// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_GROUP_H
#define _SCENEGRAPH_GROUP_H

#include "Node.h"
#include <vector>

namespace SceneGraph {

class Group : public Node
{
public:
	Group(Graphics::Renderer *r);
	Group(const Group&, NodeCopyCache *cache = 0);
	virtual Node *Clone(NodeCopyCache *cache = 0);
	virtual const char *GetTypeName() const { return "Group"; }
	virtual void Save(NodeDatabase&) override;
	static Group *Load(NodeDatabase&);

	virtual void AddChild(Node *child);
	virtual bool RemoveChild(Node *node); //true on success
	virtual bool RemoveChildAt(unsigned int position); //true on success
	unsigned int GetNumChildren() const { return m_children.size(); }
	Node* GetChildAt(unsigned int);
	virtual void Accept(NodeVisitor &v);
	virtual void Traverse(NodeVisitor &v);
	virtual void Render(const matrix4x4f &trans, const RenderData *rd);
	virtual Node* FindNode(const std::string &);

protected:
	virtual ~Group();
	virtual void RenderChildren(const matrix4x4f &trans, const RenderData *rd);
	std::vector<Node *> m_children;
};

}

#endif
