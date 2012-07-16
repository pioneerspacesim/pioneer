#include "NodeVisitor.h"
/*
 * Print the graph structure to console
 */
namespace Newmodel {

class DumpVisitor : public NodeVisitor {
public:
	DumpVisitor();
	virtual void ApplyNode(Node&);
	virtual void ApplyGroup(Group&);

private:
	int m_level;
};

}