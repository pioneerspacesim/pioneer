// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "NavLights.h"
#include "graphics/TextureBuilder.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/FindNodeVisitor.h"

NavLights::LightBulb::LightBulb(Uint8 _mask, SceneGraph::Billboard *bb)
: mask(_mask)
, billboard(bb)
{
}

NavLights::NavLights(SceneGraph::Model *model)
: m_time(0.f)
, m_period(2.f)
, m_enabled(false)
{
	using SceneGraph::Node;
	using SceneGraph::MatrixTransform;
	using SceneGraph::Billboard;

	Graphics::Renderer *renderer = model->GetRenderer();

	Graphics::MaterialDescriptor desc;
	desc.textures = 1;
	RefCountedPtr<Graphics::Material> matWhite(renderer->CreateMaterial(desc));
	RefCountedPtr<Graphics::Material> matRed(renderer->CreateMaterial(desc));
	RefCountedPtr<Graphics::Material> matGreen(renderer->CreateMaterial(desc));
	RefCountedPtr<Graphics::Material> matBlue(renderer->CreateMaterial(desc));

	matWhite->texture0 = Graphics::TextureBuilder::Billboard("textures/halo.png").GetOrCreateTexture(renderer, "billboard");
	matRed->texture0 = Graphics::TextureBuilder::Billboard("textures/halo_red.png").GetOrCreateTexture(renderer, "billboard");
	matGreen->texture0 = Graphics::TextureBuilder::Billboard("textures/halo_green.png").GetOrCreateTexture(renderer, "billboard");
	matBlue->texture0 = Graphics::TextureBuilder::Billboard("textures/halo_blue.png").GetOrCreateTexture(renderer, "billboard");

	//This will find all matrix transforms meant for navlights.
	SceneGraph::FindNodeVisitor lightFinder(SceneGraph::FindNodeVisitor::MATCH_NAME_STARTSWITH, "navlight_");
	model->GetRoot()->Accept(lightFinder);
	const std::vector<Node*> &results = lightFinder.GetResults();

	//attach light billboards
	for (unsigned int i=0; i < results.size(); i++) {
		MatrixTransform *mt = dynamic_cast<MatrixTransform*>(results.at(i));
		assert(mt);
		Billboard *bblight = new Billboard(renderer, matBlue, vector3f(0.f), 5.f);
		Uint8 mask = 0xff; //always on
		if (mt->GetName().substr(9, 3) == "red") {
			bblight->SetMaterial(matRed);
			mask = 0x0f;
		} else if (mt->GetName().substr(9, 5) == "green") {
			bblight->SetMaterial(matGreen);
			mask = 0xf0;
		}
		m_allLights.push_back(LightBulb(mask, bblight));
		mt->SetNodeMask(SceneGraph::NODE_TRANSPARENT);
		mt->AddChild(bblight);
	}
}

NavLights::~NavLights()
{
}

void NavLights::Update(float time)
{
	if (!m_enabled) {
		for (unsigned int i = 0; i < m_allLights.size(); i++)
			m_allLights[i].billboard->SetNodeMask(0x0);
		return;
	}

	m_time += time;

	int phase((fmod(m_time, m_period) / m_period) * 8);
	Uint8 mask = 1 << phase;

	for (unsigned int i = 0; i < m_allLights.size(); i++) {
		if (m_allLights[i].mask & mask)
			m_allLights[i].billboard->SetNodeMask(SceneGraph::NODE_TRANSPARENT);
		else
			m_allLights[i].billboard->SetNodeMask(0x0);
	}
}