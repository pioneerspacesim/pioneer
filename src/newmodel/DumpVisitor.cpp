#include "DumpVisitor.h"
#include "Node.h"
#include "Group.h"
#include <iostream>

namespace Newmodel {

DumpVisitor::DumpVisitor()
: m_level(0)
{
}

void DumpVisitor::ApplyNode(Node &n)
{
	for(int i=0; i<m_level; i++)
		std::cout << "  ";

	const std::string &nodeName = n.GetName();
	std::cout << (!nodeName.empty() ? nodeName : "Node") << std::endl;
}

void DumpVisitor::ApplyGroup(Group &g)
{
	for(int i=0; i<m_level; i++)
		std::cout << "  ";

	const std::string &nodeName = g.GetName();
	std::cout << (!nodeName.empty() ? nodeName : "Group") << std::endl;

	m_level++;
	g.Traverse(*this);
	m_level--;
}

}