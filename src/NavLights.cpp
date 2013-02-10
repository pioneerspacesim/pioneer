// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "NavLights.h"
#include "graphics/TextureBuilder.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/FindNodeVisitor.h"

NavLights::NavLights(SceneGraph::Model *model)
: m_time(0.f)
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
		m_allLights.push_back(bblight);
		if (mt->GetName().substr(9, 3) == "red") {
			bblight->SetMaterial(matRed);
			m_groupRed.push_back(bblight);
		} else if (mt->GetName().substr(9, 5) == "green") {
			bblight->SetMaterial(matGreen);
			m_groupGreen.push_back(bblight);
		} else {
			m_groupBlue.push_back(bblight);
		}
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
			m_allLights[i]->SetNodeMask(0x0);
		return;
	}

	m_time += time;

	int phase = int(fmod(m_time, 2.f));

	for (unsigned int i = 0; i < m_groupRed.size(); i++) {
		m_groupRed[i]->SetNodeMask(phase == 0 ? SceneGraph::NODE_TRANSPARENT : 0x0);
	}

	for (unsigned int i = 0; i < m_groupGreen.size(); i++) {
		m_groupGreen[i]->SetNodeMask(phase == 1 ? SceneGraph::NODE_TRANSPARENT : 0x0);
	}

	for (unsigned int i = 0; i < m_groupBlue.size(); i++) {
		m_groupBlue[i]->SetNodeMask(SceneGraph::NODE_TRANSPARENT);
	}
}