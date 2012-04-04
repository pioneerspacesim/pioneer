#include "Node.h"
#include "NodeVisitor.h"

namespace Newmodel {

void Node::Accept(NodeVisitor &v)
{
	v.ApplyNode(*this);
}

}