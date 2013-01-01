// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LOD.h"
#include "graphics/Graphics.h"
#include "StringF.h"

namespace SceneGraph {

LOD::LOD() : Group()
{
}

void LOD::AddLevel(float pixelSize, Node *nod)
{
	m_pixelSizes.push_back(pixelSize);
	if (nod->GetName().empty()) {
		nod->SetName(stringf("%0{f.0}", pixelSize));
	}
	AddChild(nod);
}

void LOD::Render(Graphics::Renderer *renderer, const matrix4x4f &trans, RenderData *rd)
{
	//figure out approximate pixel size on screen and pick a child to render
	const vector3f cameraPos(-trans[12], -trans[13], -trans[14]);
	const float pixrad = 0.5f * Graphics::GetScreenWidth() * rd->boundingRadius / cameraPos.Length();
	assert(m_children.size() == m_pixelSizes.size());
	if (m_pixelSizes.empty()) return;
	unsigned int lod = m_children.size() - 1;
	for (unsigned int i=m_pixelSizes.size(); i > 0; i--) {
		if (pixrad < m_pixelSizes[i-1]) lod = i-1;
	}
	m_children[lod]->Render(renderer, trans, rd);
}

}
