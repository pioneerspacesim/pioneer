// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "MatrixTransform.h"
#include "matrix4x4.h"

namespace Graphics {
	class Renderer;
}

namespace SceneGraph {

	/*
	 * Stores a named slot that can be queried from outside the model
	 */
	class Tag : public MatrixTransform {
	public:
		Tag(Graphics::Renderer *r, const matrix4x4f &m);
		Tag(const Tag &, NodeCopyCache *cache = 0);

		virtual Node *Clone(NodeCopyCache *cache = 0) override;
		virtual const char *GetTypeName() const override { return "Tag"; }
		virtual void Accept(NodeVisitor &v) override;

		virtual void Save(NodeDatabase &) override;
		static Tag *Load(NodeDatabase &);

		const matrix4x4f &GetGlobalTransform() const { return m_globalTransform; }
		// Update the cached global transform so we see changes
		// to the model transform hierarchy
		void UpdateGlobalTransform();

	protected:
		virtual ~Tag() {}

	private:
		matrix4x4f m_globalTransform;
	};
} // namespace SceneGraph
