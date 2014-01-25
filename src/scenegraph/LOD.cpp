// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LOD.h"
#include "NodeVisitor.h"
#include "NodeCopyCache.h"
#include "StringF.h"
#include "graphics/Graphics.h"

namespace SceneGraph {

LOD::LOD(Graphics::Renderer *r) : Group(r)
{
}

LOD::LOD(const LOD &lod, NodeCopyCache *cache)
: Group(lod, cache)
, m_pixelSizes(lod.m_pixelSizes)
{
}

Node* LOD::Clone(NodeCopyCache *cache)
{
	return cache->Copy<LOD>(this);
}

void LOD::Accept(NodeVisitor &nv)
{
	nv.ApplyLOD(*this);
}

void LOD::AddLevel(float pixelSize, Node *nod)
{
	m_pixelSizes.push_back(pixelSize);
	if (nod->GetName().empty()) {
		nod->SetName(stringf("%0{f.0}", pixelSize));
	}
	AddChild(nod);
}

void LOD::Render(const matrix4x4f &trans, const RenderData *rd)
{
	//figure out approximate pixel size of object's bounding radius
	//on screen and pick a child to render
	const vector3f cameraPos(-trans[12], -trans[13], -trans[14]);
	//fov is vertical, so using screen height
	const float pixrad = Graphics::GetScreenHeight() * rd->boundingRadius / (cameraPos.Length() * Graphics::GetFovFactor());
	if (m_pixelSizes.empty()) return;
	unsigned int lod = m_children.size() - 1;
	for (unsigned int i=m_pixelSizes.size(); i > 0; i--) {
		if (pixrad < m_pixelSizes[i-1]) lod = i-1;
	}
	m_children[lod]->Render(trans, rd);
}

void LOD::Save(NodeDatabase &db)
{
    Group::Save(db);
    //same number as children
    db.wr->Int32(m_pixelSizes.size());
    for (auto i : m_pixelSizes)
		db.wr->Int32(i);
}

LOD* LOD::Load(NodeDatabase &db)
{
    LOD* lod = new LOD(db.renderer);
	const Uint32 numLevels = db.rd->Int32();
	for (Uint32 i = 0; i < numLevels; i++)
		lod->m_pixelSizes.push_back(db.rd->Int32());
    return lod;
}

}
