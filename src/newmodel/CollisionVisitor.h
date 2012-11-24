// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLLISIONVISITOR_H
#define _COLLISIONVISITOR_H
/*
 * Creates a new collision mesh based on bounding boxes
 * This is for uses when a precise collision mesh is not supplied
 * with the model
 * This can be improved to create a smarter cmesh if someone feels like it.
 */
#include "NodeVisitor.h"
#include "libs.h"

class CollMesh;

namespace SceneGraph {

class Group;
class MatrixTransform;
class StaticGeometry;

class CollisionVisitor : public NodeVisitor
{
public:
	CollisionVisitor();
	virtual void ApplyStaticGeometry(StaticGeometry &g);
	virtual void ApplyMatrixTransform(MatrixTransform &m);
	//call after traversal complete
	CollMesh *CreateCollisionMesh();

	float m_boundingRadius;

private:
	//geomtree is not built until all nodes are visited and
	//BuildCollMesh called
	CollMesh *m_collMesh;
	std::vector<matrix4x4f> m_matrixStack;
};

}
#endif
