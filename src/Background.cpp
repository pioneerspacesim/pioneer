// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Background.h"
#include "Frame.h"
#include "FileSystem.h"
#include "Game.h"
#include "perlin.h"
#include "Pi.h"
#include "Player.h"
#include "Space.h"
#include "galaxy/StarSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/StaticMesh.h"
#include "graphics/Surface.h"
#include "graphics/VertexArray.h"
#include "graphics/TextureBuilder.h"

#include <sstream>
#include <iostream>

using namespace Graphics;

namespace
{
	static Uint32 GetNumSkyboxes()
	{
		char filename[1024];
		snprintf(filename, sizeof(filename), "textures/cube");
		std::vector<FileSystem::FileInfo> fileList;
		FileSystem::gameDataFiles.ReadDirectory(filename, fileList);

		char itemMask[256];
		snprintf(itemMask, sizeof(itemMask), "ub");

		Uint32 num_matching = 0;
		for (std::vector<FileSystem::FileInfo>::const_iterator it = fileList.begin(), itEnd = fileList.end(); it!=itEnd; ++it) {
			if (starts_with((*it).GetName(), itemMask)) {
				++num_matching;
			}
		}
		return num_matching;
	}
};

namespace Background
{

void BackgroundElement::SetIntensity(float intensity)
{
	m_material->emissive = Color(intensity);
}

UniverseBox::UniverseBox(Graphics::Renderer *r)
{
	Init(r);
}

UniverseBox::~UniverseBox()
{
	delete m_cubemap;
	delete m_model;
}

void UniverseBox::Init(Graphics::Renderer *r)
{
	// Load cubemap
	TextureBuilder texture_builder = TextureBuilder::Cube("textures/cube/default.dds");
	m_cubemap = texture_builder.CreateTexture(r);

	// Create skybox geometry
	VertexArray *box = new VertexArray(ATTRIB_POSITION | ATTRIB_UV0, 36);
	const float vp = 1000.0f;
	// Top +Y
	box->Add(vector3f(-vp,  vp,  vp), vector2f(0.0f, 0.0f));
	box->Add(vector3f(-vp,  vp, -vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f( vp,  vp,  vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f( vp,  vp,  vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f(-vp,  vp, -vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f( vp,  vp, -vp), vector2f(1.0f, 1.0f));
	// Bottom -Y
	box->Add(vector3f(-vp, -vp, -vp), vector2f(0.0f, 0.0f));
	box->Add(vector3f(-vp, -vp,  vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f( vp, -vp, -vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f( vp, -vp, -vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f(-vp, -vp,  vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f( vp, -vp,  vp), vector2f(1.0f, 1.0f));
	// Front -Z
	box->Add(vector3f(-vp,  vp, -vp), vector2f(0.0f, 0.0f));
	box->Add(vector3f(-vp, -vp, -vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f( vp,  vp, -vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f( vp,  vp, -vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f(-vp, -vp, -vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f( vp, -vp, -vp), vector2f(1.0f, 1.0f));
	// Back +Z
	box->Add(vector3f( vp,  vp,  vp), vector2f(0.0f, 0.0f));
	box->Add(vector3f( vp, -vp,  vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f(-vp,  vp,  vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f(-vp,  vp,  vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f( vp, -vp,  vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f(-vp, -vp,  vp), vector2f(1.0f, 1.0f));
	// Right +X
	box->Add(vector3f( vp,  vp, -vp), vector2f(0.0f, 0.0f));
	box->Add(vector3f( vp, -vp, -vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f( vp,  vp,  vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f( vp,  vp,  vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f( vp, -vp, -vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f( vp, -vp,  vp), vector2f(1.0f, 1.0f));
	// Left -X
	box->Add(vector3f(-vp,  vp,  vp), vector2f(0.0f, 0.0f));
	box->Add(vector3f(-vp, -vp,  vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f(-vp,  vp, -vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f(-vp,  vp, -vp), vector2f(1.0f, 0.0f));
	box->Add(vector3f(-vp, -vp,  vp), vector2f(0.0f, 1.0f));
	box->Add(vector3f(-vp, -vp, -vp), vector2f(1.0f, 1.0f));
	m_model = new StaticMesh(TRIANGLES);
	Graphics::MaterialDescriptor desc;
	desc.effect = EFFECT_SKYBOX;
	m_material.Reset(r->CreateMaterial(desc));
	m_material->texture0 = m_cubemap;
	m_model->AddSurface(RefCountedPtr<Surface>(new Surface(TRIANGLES, box, m_material)));
	SetIntensity(1.0f);

	m_numCubemaps = GetNumSkyboxes();
}

void UniverseBox::Draw(Graphics::Renderer *r)
{
	if(m_cubemap) {
		r->DrawStaticMesh(m_model);
	}
}

void UniverseBox::LoadCubeMap(Graphics::Renderer *r, Random* randomizer)
{
	// Clean old texture
	delete m_cubemap;
	m_cubemap = nullptr;
	
	if(randomizer && m_numCubemaps>0) {
		int new_ubox_index = randomizer->Int32(0, m_numCubemaps);
		if(new_ubox_index > 0) {
			// Load new one
			std::ostringstream os;
			os << "textures/cube/ub" << (new_ubox_index - 1) << ".dds";
			TextureBuilder texture_builder = TextureBuilder::Cube(os.str().c_str());
			m_cubemap = texture_builder.CreateTexture(r);
			m_material->texture0 = m_cubemap;
		}
	} else {
		// Load cubemap
		TextureBuilder texture_builder = TextureBuilder::Cube("textures/cube/default.dds");
		m_cubemap = texture_builder.CreateTexture(r);
		m_material->texture0 = m_cubemap;
	}
}

Starfield::Starfield(Graphics::Renderer *r)
{
	Init(r);
	//starfield is not filled without a seed
}

Starfield::Starfield(Graphics::Renderer *r, Uint32 seed)
{
	Init(r);
	Fill(seed);
}

Starfield::~Starfield()
{
	delete m_model;
	delete[] m_hyperVtx;
	delete[] m_hyperCol;
}

void Starfield::Init(Graphics::Renderer *r)
{
	// reserve some space for positions, colours
	VertexArray *stars = new VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE, BG_STAR_MAX);
	m_model = new StaticMesh(POINTS);
	Graphics::MaterialDescriptor desc;
	desc.effect = Graphics::EFFECT_STARFIELD;
	desc.vertexColors = true;
	m_material.Reset(r->CreateMaterial(desc));
	m_material->emissive = Color::WHITE;
	m_model->AddSurface(RefCountedPtr<Surface>(new Surface(POINTS, stars, m_material)));

	m_hyperVtx = 0;
	m_hyperCol = 0;
}

void Starfield::Fill(Uint32 seed)
{
	VertexArray *va = m_model->GetSurface(0)->GetVertices();
	va->Clear(); // clear if previously filled
	// Slight colour variation to stars based on seed
	Random rand(seed);

	//fill the array
	for (int i=0; i<BG_STAR_MAX; i++) {
		float col = float(rand.Double(0.2,0.7));

		// this is proper random distribution on a sphere's surface
		const float theta = float(rand.Double(0.0, 2.0*M_PI));
		const float u = float(rand.Double(-1.0, 1.0));

		va->Add(vector3f(
				1000.0f * sqrt(1.0f - u*u) * cos(theta),
				1000.0f * u,
				1000.0f * sqrt(1.0f - u*u) * sin(theta)
			), Color(col, col, col,	1.f)
		);
	}
}

void Starfield::Draw(Graphics::Renderer *renderer)
{
	// XXX would be nice to get rid of the Pi:: stuff here
	if (!Pi::game || Pi::player->GetFlightState() != Ship::HYPERSPACE) {
		renderer->DrawStaticMesh(m_model);
	} else {
		// roughly, the multiplier gets smaller as the duration gets larger.
		// the time-looking bits in this are completely arbitrary - I figured
		// it out by tweaking the numbers until it looked sort of right
		double mult = 0.0015 / (Pi::player->GetHyperspaceDuration() / (60.0*60.0*24.0*7.0));

		double hyperspaceProgress = Pi::game->GetHyperspaceProgress();

		//XXX this is a lot of lines
		if (m_hyperVtx == 0) {
			m_hyperVtx = new vector3f[BG_STAR_MAX * 2];
			m_hyperCol = new Color[BG_STAR_MAX * 2];
		}
		VertexArray *va = m_model->GetSurface(0)->GetVertices();
		vector3d pz = Pi::player->GetOrient().VectorZ();	//back vector
		for (int i=0; i<BG_STAR_MAX; i++) {

			vector3f v(va->position[i]);
			v += vector3f(pz*hyperspaceProgress*mult);

			m_hyperVtx[i*2] = va->position[i] + v;
			m_hyperCol[i*2] = va->diffuse[i];

			m_hyperVtx[i*2+1] = v;
			m_hyperCol[i*2+1] = va->diffuse[i];
		}
		renderer->DrawLines(BG_STAR_MAX*2, m_hyperVtx, m_hyperCol);
	}
}

MilkyWay::MilkyWay(Graphics::Renderer *r)
{
	m_model = new StaticMesh(TRIANGLE_STRIP);

	//build milky way model in two strips (about 256 verts)
	//The model is built as a generic vertex array first. The renderer
	//will reprocess this into buffered format as it sees fit. The old data is
	//kept around as long as StaticMesh is alive (needed if the cache is to be regenerated)

	VertexArray *bottom = new VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE);
	VertexArray *top = new VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE);

	const Color dark(0.f);
	const Color bright(0.05f, 0.05f, 0.05f, 0.05f);

	//bottom
	float theta;
	for (theta=0.0; theta < 2.f*float(M_PI); theta+=0.1f) {
		bottom->Add(
				vector3f(100.0f*sin(theta), float(-40.0 - 30.0*noise(sin(theta),1.0,cos(theta))), 100.0f*cos(theta)),
				dark);
		bottom->Add(
			vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
			bright);
	}
	theta = 2.f*float(M_PI);
	bottom->Add(
		vector3f(100.0f*sin(theta), float(-40.0 - 30.0*noise(sin(theta),1.0,cos(theta))), 100.0f*cos(theta)),
		dark);
	bottom->Add(
		vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
		bright);
	//top
	for (theta=0; theta < 2.f*float(M_PI); theta+=0.1f) {
		top->Add(
			vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
			bright);
		top->Add(
			vector3f(100.0f*sin(theta), float(40.0 + 30.0*noise(sin(theta),-1.0,cos(theta))), 100.0f*cos(theta)),
			dark);
	}
	theta = 2.f*float(M_PI);
	top->Add(
		vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
		bright);
	top->Add(
		vector3f(100.0f*sin(theta), float(40.0 + 30.0*noise(sin(theta),-1.0,cos(theta))), 100.0f*cos(theta)),
		dark);

	Graphics::MaterialDescriptor desc;
	desc.effect = Graphics::EFFECT_STARFIELD;
	desc.vertexColors = true;
	m_material.Reset(r->CreateMaterial(desc));
	m_material->emissive = Color::WHITE;
	//This doesn't fade. Could add a generic opacity/intensity value.
	m_model->AddSurface(RefCountedPtr<Surface>(new Surface(TRIANGLE_STRIP, bottom, m_material)));
	m_model->AddSurface(RefCountedPtr<Surface>(new Surface(TRIANGLE_STRIP, top, m_material)));
}

MilkyWay::~MilkyWay()
{
	delete m_model;
}

void MilkyWay::Draw(Graphics::Renderer *renderer)
{
	assert(m_model != 0);
	renderer->DrawStaticMesh(m_model);
}

Container::Container(Graphics::Renderer *r)
: m_milkyWay(r)
, m_starField(r)
, m_universeBox(r)
, m_bLoadNewCubemap(true)
, m_uSeed(0)
, m_drawFlags( DRAW_SKYBX )
{
}

Container::Container(Graphics::Renderer *r, Uint32 seed)
: m_milkyWay(r)
, m_starField(r)
, m_universeBox(r)
, m_bLoadNewCubemap(true)
, m_uSeed(seed)
, m_drawFlags( DRAW_SKYBX )
{
	Refresh(seed);
};

void Container::Refresh(Uint32 seed)
{
	// redo starfield, milkyway stays normal for now
	m_starField.Fill(seed);
	if(m_uSeed != seed) {
		m_bLoadNewCubemap = true;
	}
	m_uSeed = seed;
}

void Container::Draw(Graphics::Renderer *renderer, const matrix4x4d &transform)
{
	PROFILE_SCOPED()
	//XXX not really const - renderer can modify the buffers
	if(m_bLoadNewCubemap) {
		m_bLoadNewCubemap = false;
		if(Pi::player == nullptr || Pi::player->GetFlightState() != Ship::HYPERSPACE) {
			if(Pi::player && Pi::game->GetSpace()->GetStarSystem()) {
				Uint32 seeds [5];
				const SystemPath& system_path = Pi::game->GetSpace()->GetStarSystem()->GetPath();
				seeds[0] = system_path.systemIndex + 41;
				seeds[1] = system_path.sectorX;
				seeds[2] = system_path.sectorY;
				seeds[3] = system_path.sectorZ;
				seeds[4] = UNIVERSE_SEED;
				Random rand(seeds, 5);
				m_universeBox.LoadCubeMap(renderer, &rand);
			} else {
				Random rand(m_uSeed);
				m_universeBox.LoadCubeMap(renderer, &rand);
			}
		} else {
			m_universeBox.LoadCubeMap(renderer);
		}
	}
	renderer->SetBlendMode(BLEND_SOLID);
	renderer->SetDepthTest(false);
	renderer->SetTransform(transform);
	if( DRAW_SKYBX & m_drawFlags ) {
		m_universeBox.Draw(renderer);
	}
	if( DRAW_MILKY & m_drawFlags ) {
		m_milkyWay.Draw(renderer);
	}
	if( DRAW_STARS & m_drawFlags ) {
		// squeeze the starfield a bit to get more density near horizon
		matrix4x4d starTrans = transform * matrix4x4d::ScaleMatrix(1.0, 0.4, 1.0);
		renderer->SetTransform(starTrans);
		const_cast<Starfield&>(m_starField).Draw(renderer);
	}
	renderer->SetDepthTest(true);
}

void Container::SetIntensity(float intensity)
{
	PROFILE_SCOPED()
	m_universeBox.SetIntensity(intensity);
	m_starField.SetIntensity(intensity);
	m_milkyWay.SetIntensity(intensity);
}

void Container::SetDrawFlags(const Uint32 flags)
{
	m_drawFlags = flags;
}

} //namespace Background
