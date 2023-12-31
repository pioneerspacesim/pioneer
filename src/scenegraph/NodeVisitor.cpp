// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "NodeVisitor.h"
#include "Billboard.h"
#include "CollisionGeometry.h"
#include "Group.h"
#include "LOD.h"
#include "Label3D.h"
#include "MatrixTransform.h"
#include "Node.h"
#include "StaticGeometry.h"
#include "Thruster.h"
#include "Tag.h"

namespace SceneGraph {

	void NodeVisitor::ApplyNode(Node &n)
	{
		n.Traverse(*this);
	}

	void NodeVisitor::ApplyGroup(Group &g)
	{
		ApplyNode(static_cast<Node &>(g));
	}

	void NodeVisitor::ApplyStaticGeometry(StaticGeometry &g)
	{
		ApplyNode(static_cast<Node &>(g));
	}

	void NodeVisitor::ApplyLabel(Label3D &l)
	{
		ApplyNode(static_cast<Node &>(l));
	}

	void NodeVisitor::ApplyMatrixTransform(MatrixTransform &m)
	{
		ApplyGroup(static_cast<Group &>(m));
	}

	void NodeVisitor::ApplyTag(Tag &t)
	{
		ApplyMatrixTransform(static_cast<MatrixTransform &>(t));
	}

	void NodeVisitor::ApplyBillboard(Billboard &b)
	{
		ApplyNode(static_cast<Node &>(b));
	}

	void NodeVisitor::ApplyThruster(Thruster &t)
	{
		ApplyNode(static_cast<Node &>(t));
	}

	void NodeVisitor::ApplyLOD(LOD &l)
	{
		ApplyGroup(static_cast<Group &>(l));
	}

	void NodeVisitor::ApplyCollisionGeometry(CollisionGeometry &g)
	{
		ApplyNode(static_cast<Node &>(g));
	}

} // namespace SceneGraph
