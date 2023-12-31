// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MODELNODE_H
#define _MODELNODE_H
/*
 *	Use another model as a submodel
 */
#include "Model.h"
#include "Node.h"

namespace SceneGraph {

	class ModelNode : public Node {
	public:
		ModelNode(Model *m);
		ModelNode(const ModelNode &, NodeCopyCache *cache = 0);
		virtual Node *Clone(NodeCopyCache *cache = 0);
		virtual const char *GetTypeName() const { return "ModelNode"; }
		virtual void Render(const matrix4x4f &trans, const RenderData *rd);
		virtual void Render(const std::vector<matrix4x4f> &trans, const RenderData *rd);

	protected:
		virtual ~ModelNode() {}

	private:
		Model *m_model;
	};

} // namespace SceneGraph

#endif
