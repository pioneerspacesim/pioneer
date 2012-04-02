#include "Importer.h"
#include "Newmodel.h"
#include "StaticGeometry.h"

namespace Newmodel {

NModel *Importer::CreateDummyModel()
{
	NModel *m = new NModel();
	StaticGeometry* geom = new StaticGeometry();
	m->m_root->AddChild(geom);
	return m;
}

}