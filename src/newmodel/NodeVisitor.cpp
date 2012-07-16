#include "NodeVisitor.h"
#include "Group.h"
#include "Label3D.h"
#include "MatrixTransform.h"
#include "Node.h"
#include "StaticGeometry.h"

namespace Newmodel {

void NodeVisitor::ApplyNode(Node &n)
{
	n.Traverse(*this);
}

void NodeVisitor::ApplyGroup(Group &g)
{
	ApplyNode(static_cast<Node&>(g));
}

void NodeVisitor::ApplyStaticGeometry(StaticGeometry &g)
{
	ApplyNode(static_cast<Node&>(g));
}

void NodeVisitor::ApplyLabel(Label3D &l)
{
	ApplyNode(static_cast<Node&>(l));
}

void NodeVisitor::ApplyMatrixTransform(MatrixTransform &m)
{
	ApplyGroup(static_cast<Group&>(m));
}

}
