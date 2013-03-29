// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "NodeVisitor.h"
/*
 * Print the graph structure to console
 */
namespace SceneGraph {

class DumpVisitor : public NodeVisitor {
public:
	DumpVisitor();
	virtual void ApplyNode(Node&);
	virtual void ApplyGroup(Group&);

private:
	int m_level;
};

}
