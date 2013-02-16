// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "NavLights.h"
#include "graphics/TextureBuilder.h"
#include "scenegraph/FindNodeVisitor.h"
#include "scenegraph/SceneGraph.h"

const float BILLBOARD_SIZE = 5.f;

static bool g_initted = false;

static RefCountedPtr<Graphics::Material> matWhite;
static RefCountedPtr<Graphics::Material> matRed;
static RefCountedPtr<Graphics::Material> matGreen;
static RefCountedPtr<Graphics::Material> matBlue;
static RefCountedPtr<Graphics::Material> matYellow;

typedef std::vector<NavLights::LightBulb>::iterator LightIterator;

NavLights::LightBulb::LightBulb(Uint8 _group, Uint8 _mask, SceneGraph::Billboard *bb)
: group(_group)
, mask(_mask)
, billboard(bb)
, color(NAVLIGHT_BLUE)
{
}

void NavLights::Init(Graphics::Renderer *renderer)
{
	assert(!g_initted);
	Graphics::MaterialDescriptor desc;
	desc.textures = 1;
	matWhite.Reset(renderer->CreateMaterial(desc));
	matRed.Reset(renderer->CreateMaterial(desc));
	matGreen.Reset(renderer->CreateMaterial(desc));
	matBlue.Reset(renderer->CreateMaterial(desc));
	matYellow.Reset(renderer->CreateMaterial(desc));

	//not cached because modelviewer clears everything...
	matWhite->texture0  = Graphics::TextureBuilder::Billboard("textures/halo.png").CreateTexture(renderer);
	matRed->texture0    = Graphics::TextureBuilder::Billboard("textures/halo_red.png").CreateTexture(renderer);
	matGreen->texture0  = Graphics::TextureBuilder::Billboard("textures/halo_green.png").CreateTexture(renderer);
	matBlue->texture0   = Graphics::TextureBuilder::Billboard("textures/halo_blue.png").CreateTexture(renderer);
	matYellow->texture0 = Graphics::TextureBuilder::Billboard("textures/halo_yellow.png").CreateTexture(renderer);

	g_initted = true;
}

void NavLights::Uninit()
{
	assert(g_initted);

	delete matWhite->texture0;
	delete matRed->texture0;
	delete matGreen->texture0;
	delete matBlue->texture0;

	g_initted = false;
}

NavLights::NavLights(SceneGraph::Model *model, float period)
: m_period(period)
, m_enabled(false)
, m_time(0.f)
{
	assert(g_initted);

	Graphics::Renderer *renderer = model->GetRenderer();

	using SceneGraph::Node;
	using SceneGraph::MatrixTransform;
	using SceneGraph::Billboard;

	//This will find all matrix transforms meant for navlights.
	SceneGraph::FindNodeVisitor lightFinder(SceneGraph::FindNodeVisitor::MATCH_NAME_STARTSWITH, "navlight_");
	model->GetRoot()->Accept(lightFinder);
	const std::vector<Node*> &results = lightFinder.GetResults();

	//attach light billboards
	for (unsigned int i=0; i < results.size(); i++) {
		MatrixTransform *mt = dynamic_cast<MatrixTransform*>(results.at(i));
		assert(mt);
		Billboard *bblight = new Billboard(renderer, matBlue, vector3f(0.f), BILLBOARD_SIZE);
		Uint8 group = 0;
		Uint8 mask = 0xff; //always on

		if (mt->GetName().substr(9, 3) == "red") {
			bblight->SetMaterial(matRed);
			mask = 0x0f;
		} else if (mt->GetName().substr(9, 5) == "green") {
			bblight->SetMaterial(matGreen);
			mask = 0xf0;
		} else if (mt->GetName().substr(9, 3) == "pad") {
			//group by pad number
			group = atoi(mt->GetName().substr(12, 1).c_str());
			mask = 0xf0;
		}
		//everything else is blue & static

		m_lights.push_back(LightBulb(group, mask, bblight));
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
		for (LightIterator it = m_lights.begin(); it != m_lights.end(); ++it)
			it->billboard->SetNodeMask(0x0);
		return;
	}

	m_time += time;

	int phase((fmod(m_time, m_period) / m_period) * 8);
	Uint8 mask = 1 << phase;

	for (LightIterator it = m_lights.begin(); it != m_lights.end(); ++it) {
		if (it->mask & mask)
			it->billboard->SetNodeMask(SceneGraph::NODE_TRANSPARENT);
		else
			it->billboard->SetNodeMask(0x0);
	}
}

void NavLights::SetColor(unsigned int group, LightColor c)
{
	RefCountedPtr<Graphics::Material> mat;

	for (LightIterator it = m_lights.begin(); it != m_lights.end(); ++it) {
		if (it->group != group || it->color == c) continue;
		if (c == NAVLIGHT_RED)
			mat = matRed;
		else if (c == NAVLIGHT_GREEN)
			mat = matGreen;
		else if (c == NAVLIGHT_YELLOW)
			mat = matYellow;
		else
			mat = matBlue;
		it->billboard->SetMaterial(mat);
		it->color = c;
	}
}
