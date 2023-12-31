// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_THRUSTER_H
#define _SCENEGRAPH_THRUSTER_H
/*
 * Spaceship thruster
 */

#include "Node.h"

namespace Graphics {
	class Renderer;
	class Material;
	class MeshObject;
	class RenderState;
} // namespace Graphics

namespace SceneGraph {

	class Thruster : public Node {
	public:
		Thruster(Graphics::Renderer *, bool linear, const vector3f &pos, const vector3f &dir);
		Thruster(const Thruster &, NodeCopyCache *cache = 0);
		Node *Clone(NodeCopyCache *cache = 0) override;
		virtual void Accept(NodeVisitor &v) override;
		virtual const char *GetTypeName() const override { return "Thruster"; }
		virtual void Render(const matrix4x4f &trans, const RenderData *rd) override;
		virtual void Save(NodeDatabase &) override;
		static Thruster *Load(NodeDatabase &);
		void SetColor(const Color c) { currentColor = c; }
		const vector3f &GetDirection() { return dir; }

	private:
		// thruster geometry is shared between all instances of Thruster
		// TODO: load this as a model to allow customization
		static void CreateThrusterGeometry(Graphics::Renderer *);
		static RefCountedPtr<Graphics::MeshObject> s_thrustMesh;
		static RefCountedPtr<Graphics::MeshObject> s_glowMesh;

		RefCountedPtr<Graphics::Material> m_tMat;
		RefCountedPtr<Graphics::Material> m_glowMat;
		bool linearOnly;
		vector3f dir;
		vector3f pos;
		Color currentColor;
	};

} // namespace SceneGraph

#endif
