#include "Node.h"
#include "NodeVisitor.h"

namespace Newmodel {

Node::Node()
: m_name("")
, m_nodeMask(NODE_SOLID)
{
}

Node::Node(unsigned int nodemask)
: m_name("")
, m_nodeMask(nodemask)
{
}

void Node::Accept(NodeVisitor &v)
{
	v.ApplyNode(*this);
}

Node* Node::FindNode(const std::string &name)
{
	if (m_name == name)
		return this;
	else
		return 0;
}

}
