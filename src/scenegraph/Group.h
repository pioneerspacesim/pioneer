// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_GROUP_H
#define _SCENEGRAPH_GROUP_H

#include "Node.h"
#include "matrix4x4.h"
#include <vector>

namespace SceneGraph {

	class Group : public Node {
	public:
		Group(Graphics::Renderer *r);
		Group(const Group &, NodeCopyCache *cache = 0);
		Node *Clone(NodeCopyCache *cache = 0) override;
		const char *GetTypeName() const override { return "Group"; }
		void Save(NodeDatabase &) override;
		static Group *Load(NodeDatabase &);

		virtual void AddChild(Node *child);
		virtual bool RemoveChild(Node *node); //true on success
		virtual bool RemoveChildAt(unsigned int position); //true on success
		unsigned int GetNumChildren() const { return static_cast<Uint32>(m_children.size()); }
		Node *GetChildAt(unsigned int);
		void Accept(NodeVisitor &v) override;
		void Traverse(NodeVisitor &v) override;
		void Render(const matrix4x4f &trans, const RenderData *rd) override;
		void RenderInstanced(const std::vector<matrix4x4f> &trans, const RenderData *rd) override;
		Node *FindNode(const std::string &) override;

		// Walk the node hierarchy to the root of the model and compute the global transform of this node.
		// The result of this *should* be cached if the model has not changed
		virtual matrix4x4f CalcGlobalTransform() const;

	protected:
		virtual ~Group();
		virtual void RenderChildren(const matrix4x4f &trans, const RenderData *rd);
		virtual void RenderChildren(const std::vector<matrix4x4f> &trans, const RenderData *rd);
		std::vector<Node *> m_children;
	};

} // namespace SceneGraph

#endif
