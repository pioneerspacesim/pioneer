#ifndef _NEWMODEL_H
#define _NEWMODEL_H

#include "Model.h"
#include "Group.h"

namespace Graphics { class Renderer; }

namespace Newmodel
{

class NModel : public Model
{
public:
	NModel();
	~NModel();
	float GetDrawClipRadius() const;
	void Render(Graphics::Renderer *r, const matrix4x4f &trans, const LmrObjParams *params);
	CollMesh *CreateCollisionMesh(const LmrObjParams *p) { return 0; }
	RefCountedPtr<Group> m_root;
};

}

#endif
