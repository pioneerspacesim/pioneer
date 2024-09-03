// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLLISIONVISITOR_H
#define _COLLISIONVISITOR_H
/*
 * Creates a new collision mesh from CollisionGeometry nodes
 * or the nodes' AABB, when no CGeoms found.
 */
#include "CollMesh.h"
#include "NodeVisitor.h"
#include "matrix4x4.h"

namespace SceneGraph {
	class Group;
	class MatrixTransform;
	class StaticGeometry;

	class CollisionVisitor : public NodeVisitor {
	public:
		CollisionVisitor();
		virtual void ApplyStaticGeometry(StaticGeometry &);
		virtual void ApplyMatrixTransform(MatrixTransform &);
		virtual void ApplyCollisionGeometry(CollisionGeometry &);
		//call after traversal complete
		RefCountedPtr<CollMesh> CreateCollisionMesh();
		float GetBoundingRadius() const { return m_boundingRadius; }

	private:
		void ApplyDynamicCollisionGeometry(CollisionGeometry &);
		void AabbToMesh(const Aabb &);
		//geomtree is not built until all nodes are visited and
		//BuildCollMesh called
		RefCountedPtr<CollMesh> m_collMesh;
		std::vector<matrix4x4f> m_matrixStack;
		float m_boundingRadius;
		bool m_properData;

		//temporary arrays for static geometry
		std::vector<vector3f> m_vertices;
		std::vector<Uint32> m_indices;
		std::vector<Uint32> m_flags;

		Uint32 m_totalTris;
	};
} // namespace SceneGraph
#endif
