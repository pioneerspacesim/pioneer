#ifndef _NODE_H
#define _NODE_H
/*
 * Generic node for the model scenegraph
 */
#include "RefCounted.h"

namespace Graphics { class Renderer; }

namespace Newmodel
{

class NodeVisitor;
struct RenderData;

class Node : public RefCounted
{
public:
	Node() { }
	Node *m_parent;

	virtual void Accept(NodeVisitor &v);
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd) { }
protected:
	//can only to be deleted using DecRefCount
	virtual ~Node() { }
};

struct RenderData {
	int scrWidth;
	float boundingRadius;
	bool drawBoundingBoxes;

	RenderData()
	: scrWidth(800)
	, boundingRadius(0.f)
	, drawBoundingBoxes(false)
	{ }
};

}

#endif