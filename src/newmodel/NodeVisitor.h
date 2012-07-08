#ifndef _NODEVISITOR_H
#define _NODEVISITOR_H
/*
 * Node visitor.
 * Start traversal with node->Accept(visitor)!
 */
namespace Newmodel {

class Node;
class Group;
class StaticGeometry;
class Label3D;

class NodeVisitor
{
public:
	virtual void ApplyNode(Node&);
	virtual void ApplyGroup(Group&);
	virtual void ApplyStaticGeometry(StaticGeometry&);
	virtual void ApplyLabel(Label3D&);
};

}
#endif
