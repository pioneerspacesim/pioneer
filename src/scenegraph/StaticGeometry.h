// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _STATICGEOMETRY_H
#define _STATICGEOMETRY_H
/*
 * Geometry node containing one or more meshes.
 */
#include "Aabb.h"
#include "Node.h"
#include "graphics/Renderer.h"
#include "graphics/VertexBuffer.h"
#include "libs.h"

namespace SceneGraph {

	class NodeVisitor;

	class StaticGeometry : public Node {
	public:
		struct Mesh {
			RefCountedPtr<Graphics::VertexBuffer> vertexBuffer;
			RefCountedPtr<Graphics::IndexBuffer> indexBuffer;
			RefCountedPtr<Graphics::Material> material;
		};
		StaticGeometry(Graphics::Renderer *r);
		StaticGeometry(const StaticGeometry &, NodeCopyCache *cache = 0);
		virtual Node *Clone(NodeCopyCache *cache = 0) override;
		virtual const char *GetTypeName() const override { return "StaticGeometry"; }
		virtual void Accept(NodeVisitor &nv) override;
		virtual void Render(const matrix4x4f &trans, const RenderData *rd) override;
		virtual void Render(const std::vector<matrix4x4f> &trans, const RenderData *rd) override;

		virtual void Save(NodeDatabase &) override;
		static StaticGeometry *Load(NodeDatabase &);

		void AddMesh(RefCountedPtr<Graphics::VertexBuffer>,
			RefCountedPtr<Graphics::IndexBuffer>,
			RefCountedPtr<Graphics::Material>);
		unsigned int GetNumMeshes() const { return static_cast<Uint32>(m_meshes.size()); }
		Mesh &GetMeshAt(unsigned int i);

		void SetRenderState(Graphics::RenderState *s) { m_renderState = s; }

		Aabb m_boundingBox;
		Graphics::BlendMode m_blendMode;

	protected:
		~StaticGeometry();
		void DrawBoundingBox(const Aabb &bb);
		std::vector<Mesh> m_meshes;
		std::vector<RefCountedPtr<Graphics::Material>> m_instanceMaterials;
		Graphics::RenderState *m_renderState;
		RefCountedPtr<Graphics::InstanceBuffer> m_instBuffer;
	};

} // namespace SceneGraph
#endif
