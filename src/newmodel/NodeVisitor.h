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

class NodeVisitor
{
public:
	virtual void ApplyNode(Node &n);
	virtual void ApplyGroup(Group &g);
	virtual void ApplyStaticGeometry(StaticGeometry &g);
};

}
#endif