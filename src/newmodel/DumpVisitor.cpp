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

	if (n.GetName().empty())
		std::cout << n.GetTypeName() << std::endl;
	else
		std::cout << n.GetTypeName() << " - " << n.GetName() << std::endl;
}

void DumpVisitor::ApplyGroup(Group &g)
{
	for(int i=0; i<m_level; i++)
		std::cout << "  ";

	if (g.GetName().empty())
		std::cout << g.GetTypeName() << std::endl;
	else
		std::cout << g.GetTypeName() << " - " << g.GetName() << std::endl;

	m_level++;
	g.Traverse(*this);
	m_level--;
}

}