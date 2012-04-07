#ifndef _COLLISIONVISITOR_H
#define _COLLISIONVISITOR_H
/*
 * Collision mesh "collector"
 */
#include "NodeVisitor.h"
#include "libs.h"

class CollMesh;

namespace Newmodel {

class StaticGeometry;
class Group;

class CollisionVisitor : public NodeVisitor
{
public:
	CollisionVisitor();
	virtual void ApplyStaticGeometry(StaticGeometry &g);
	//call after traversal complete
	CollMesh *CreateCollisionMesh();
private:
	std::vector<float> m_vertices;
	std::vector<int> m_indices;
	std::vector<unsigned int> m_flags; //1 per triangle
	//geomtree is not built until all nodes are visited and
	//BuildCollMesh called
	CollMesh *m_collMesh;
	int m_offset; //growing index offset for multiple surfaces
};

}
#endif
