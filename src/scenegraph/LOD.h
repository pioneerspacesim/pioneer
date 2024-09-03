// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LOD_H
#define _LOD_H
/*
 * Level of detail switch node
 */
#include "Group.h"

namespace SceneGraph {

	class LOD : public Group {
	public:
		LOD(Graphics::Renderer *r);
		LOD(const LOD &, NodeCopyCache *cache = 0);
		virtual Node *Clone(NodeCopyCache *cache = 0) override;
		virtual const char *GetTypeName() const override { return "LOD"; }
		virtual void Accept(NodeVisitor &v) override;
		virtual void Render(const matrix4x4f &trans, const RenderData *rd) override;
		virtual void Render(const std::vector<matrix4x4f> &trans, const RenderData *rd) override;
		void AddLevel(float pixelRadius, Node *child);
		virtual void Save(NodeDatabase &) override;
		static LOD *Load(NodeDatabase &);

	protected:
		virtual ~LOD() {}
		std::vector<unsigned int> m_pixelSizes; // same number as children
	};

} // namespace SceneGraph

#endif
