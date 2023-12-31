// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _NODEVISITOR_H
#define _NODEVISITOR_H
/*
 * Node visitor using the "double dispatch" model
 * where the nodes call the appropriate Apply* method
 *
 * Start traversal with node->Accept(visitor)!
 */
namespace SceneGraph {

	class Billboard;
	class CollisionGeometry;
	class Group;
	class Label3D;
	class LOD;
	class MatrixTransform;
	class Node;
	class StaticGeometry;
	class Tag;
	class Thruster;

	class NodeVisitor {
	public:
		virtual ~NodeVisitor() {}
		virtual void ApplyNode(Node &);
		virtual void ApplyGroup(Group &);
		virtual void ApplyStaticGeometry(StaticGeometry &);
		virtual void ApplyLabel(Label3D &);
		virtual void ApplyMatrixTransform(MatrixTransform &);
		virtual void ApplyTag(Tag &);
		virtual void ApplyBillboard(Billboard &);
		virtual void ApplyThruster(Thruster &);
		virtual void ApplyLOD(LOD &);
		virtual void ApplyCollisionGeometry(CollisionGeometry &);
	};

} // namespace SceneGraph
#endif
