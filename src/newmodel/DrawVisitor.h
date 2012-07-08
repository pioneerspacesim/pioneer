#ifndef _DRAWVISITOR_H
#define _DRAWVISITOR_H
#include "NodeVisitor.h"

namespace Graphics { class Renderer; }
namespace Newmodel {

class StaticGeometry;

class DrawVisitor : public NodeVisitor
{
public:
	DrawVisitor(Graphics::Renderer *r) : m_renderer(r) { }
	virtual void ApplyStaticGeometry(StaticGeometry &g);
private:
	Graphics::Renderer *m_renderer;
};

}

#endif
