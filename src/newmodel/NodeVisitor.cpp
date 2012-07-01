#include "NodeVisitor.h"
#include "Group.h"
#include "Node.h"
#include "StaticGeometry.h"
#include "Label3D.h"

namespace Newmodel {

void NodeVisitor::ApplyNode(Node &n)
{
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

}