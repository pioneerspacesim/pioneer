#ifndef _STATICGEOMETRY_H
#define _STATICGEOMETRY_H
/*
 * Geometry node containing one StaticMesh. Nothing fancy.
 */
#include "Node.h"
#include "graphics\StaticMesh.h"
#include "SmartPtr.h"

namespace Newmodel {

class StaticGeometry : public Node
{
public:
	StaticGeometry();
	void Render(Graphics::Renderer *r);

protected:
	~StaticGeometry();
	ScopedPtr<Graphics::StaticMesh> m_mesh;
};

}
#endif