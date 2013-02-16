// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LOD_H
#define _LOD_H
/*
 * Level of detail switch node
 */
#include "Group.h"

namespace SceneGraph {

class LOD : public Group {
public:
	LOD(Graphics::Renderer *r);
	LOD(const LOD&);
	virtual Node *Clone();
	virtual const char *GetTypeName() { return "LOD"; }
	virtual void Accept(NodeVisitor &v);
	void AddLevel(float pixelRadius, Node *child);
	virtual void Render(const matrix4x4f &trans, RenderData *rd);
protected:
	virtual ~LOD() { }
	std::vector<unsigned int> m_pixelSizes; //same amount as children
};

}

#endif
