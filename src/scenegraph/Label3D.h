// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_LABEL_H
#define _SCENEGRAPH_LABEL_H
/*
 * Text geometry node, mostly for ship labels
 */
#include "Node.h"
#include "graphics/Material.h"
#include "graphics/VertexBuffer.h"
#include "text/DistanceFieldFont.h"

#include <memory>

namespace Graphics {
	class Renderer;
	class RenderState;
} // namespace Graphics

namespace SceneGraph {

	class Label3D : public Node {
	public:
		Label3D(Graphics::Renderer *r, RefCountedPtr<Text::DistanceFieldFont>);
		Label3D(const Label3D &, NodeCopyCache *cache = 0);
		virtual Node *Clone(NodeCopyCache *cache = 0);
		virtual const char *GetTypeName() const { return "Label3D"; }
		void SetText(const std::string &);
		virtual void Render(const matrix4x4f &trans, const RenderData *rd);
		virtual void Accept(NodeVisitor &v);

	private:
		RefCountedPtr<Graphics::Material> m_material;
		std::unique_ptr<Graphics::VertexArray> m_geometry;
		std::unique_ptr<Graphics::MeshObject> m_textMesh;
		RefCountedPtr<Text::DistanceFieldFont> m_font;
	};

} // namespace SceneGraph

#endif
