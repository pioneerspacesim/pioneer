// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _NODEVISITOR_H
#define _NODEVISITOR_H
/*
 * Node visitor.
 * Start traversal with node->Accept(visitor)!
 */
#include "libs.h"

namespace Newmodel {

class Billboard;
class Group;
class Label3D;
class LOD;
class MatrixTransform;
class Node;
class StaticGeometry;
class Thruster;

class NodeVisitor
{
public:
	virtual ~NodeVisitor() { }
	virtual void ApplyNode(Node&);
	virtual void ApplyGroup(Group&);
	virtual void ApplyStaticGeometry(StaticGeometry&);
	virtual void ApplyLabel(Label3D&);
	virtual void ApplyMatrixTransform(MatrixTransform&);
	virtual void ApplyBillboard(Billboard&);
	virtual void ApplyThruster(Thruster&);
	virtual void ApplyLOD(LOD&);
};

}
#endif
