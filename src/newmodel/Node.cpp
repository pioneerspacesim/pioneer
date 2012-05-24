#include "Node.h"
#include "NodeVisitor.h"

namespace Newmodel {

Node::Node()
: m_name("")
{
}

void Node::Accept(NodeVisitor &v)
{
	v.ApplyNode(*this);
}

}