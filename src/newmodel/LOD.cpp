#include "LOD.h"

namespace Newmodel {

LOD::LOD() : Group()
{
}

void LOD::AddLevel(float pixelRadius, Node *nod)
{
	m_pixelSizes.push_back(pixelRadius);
	AddChild(nod);
}

void LOD::Render(Graphics::Renderer *renderer, const matrix4x4f &trans, RenderData *rd)
{
	//figure out approximate pixel size on screen and pick a child to render
	const vector3f cameraPos(-trans[12], -trans[13], -trans[14]);
	const float pixrad = 0.5f * rd->scrWidth * rd->boundingRadius / cameraPos.Length();
	assert(m_children.size() == m_pixelSizes.size());
	if (m_pixelSizes.empty()) return;
	unsigned int lod = m_children.size() - 1;
	for (unsigned int i=0; i != m_pixelSizes.size(); i++) {
		if (pixrad < m_pixelSizes[i]) lod = i;
	}
	m_children[lod]->Render(renderer, trans, rd);
}

}