#ifndef _LOD_H
#define _LOD_H
/*
 * Level of detail switch node
 */
#include "Group.h"

namespace Newmodel {

class LOD : public Group {
public:
	LOD();
	virtual const char *GetTypeName() { return "LOD"; }
	void AddLevel(float pixelRadius, Node *child);
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd);
protected:
	virtual ~LOD() { }
	std::vector<unsigned int> m_pixelSizes; //same amount as children
};

}

#endif
