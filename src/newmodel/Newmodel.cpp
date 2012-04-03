#include "Newmodel.h"
#include "graphics/Renderer.h"

namespace Newmodel {

NModel::NModel() : Model()
{
	m_root.Reset(new Group());
}

NModel::~NModel()
{
}

void NModel::Render(Graphics::Renderer *renderer, const matrix4x4f &trans, const LmrObjParams *params)
{
	renderer->SetTransform(trans);
	m_root->Render(renderer);
}

}
