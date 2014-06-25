// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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
#include "graphics/VertexArray.h"
#include "graphics/TextureBuilder.h"
#include "StringF.h"

#include <SDL_stdinc.h>
#include <sstream>
#include <iostream>

using namespace Graphics;

namespace
{
	static std::unique_ptr<Graphics::Texture> s_defaultCubeMap;

	static Uint32 GetNumSkyboxes()
	{
		char filename[1024];
		snprintf(filename, sizeof(filename), "textures/skybox");
		std::vector<FileSystem::FileInfo> fileList;
		FileSystem::gameDataFiles.ReadDirectory(filename, fileList);

		const char *itemMask = "ub";

		Uint32 num_matching = 0;
		for (std::vector<FileSystem::FileInfo>::const_iterator it = fileList.begin(), itEnd = fileList.end(); it!=itEnd; ++it) {
			if (starts_with((*it).GetName(), itemMask)) {
				++num_matching;
			}
		}
		return num_matching;
	}
}

namespace Background
{

#pragma pack(push, 4)
struct MilkyWayVert {
	vector3f pos;
	Color4ub col;
};

struct StarVert {
	vector3f pos;
	Color4ub col;
};

struct SkyboxVert {
	vector3f pos;
	vector2f uv;
};
#pragma pack(pop)

void BackgroundElement::SetIntensity(float intensity)
{
	m_material->emissive = Color(intensity*255);
}

UniverseBox::UniverseBox(Graphics::Renderer *renderer)
{
	m_renderer = renderer;
	Init();
}

UniverseBox::~UniverseBox()
{
}

void UniverseBox::Init()
{
	// Load default cubemap
	if(!s_defaultCubeMap.get()) {
		TextureBuilder texture_builder = TextureBuilder::Cube("textures/skybox/default.dds");
		s_defaultCubeMap.reset( texture_builder.CreateTexture(m_renderer) );
	}

	// Create skybox geometry
	std::unique_ptr<Graphics::VertexArray> box(new VertexArray(ATTRIB_POSITION | ATTRIB_UV0, 36));
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

	Graphics::MaterialDescriptor desc;
	desc.effect = EFFECT_SKYBOX;
	m_material.Reset(m_renderer->CreateMaterial(desc));
	m_material->texture0 = nullptr;

	//create buffer and upload data
	Graphics::VertexBufferDesc vbd;
	vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
	vbd.attrib[0].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
	vbd.attrib[1].semantic = Graphics::ATTRIB_UV0;
	vbd.attrib[1].format   = Graphics::ATTRIB_FORMAT_FLOAT2;
	vbd.numVertices = box->GetNumVerts();
	vbd.usage = Graphics::BUFFER_USAGE_STATIC;

	m_vertexBuffer.reset(m_renderer->CreateVertexBuffer(vbd));

	SkyboxVert* vtxPtr = m_vertexBuffer->Map<SkyboxVert>(Graphics::BUFFER_MAP_WRITE);
	assert(m_vertexBuffer->GetDesc().stride == sizeof(SkyboxVert));
	for (Uint32 i = 0; i < box->GetNumVerts(); i++) {
		vtxPtr[i].pos = box->position[i];
		vtxPtr[i].uv = box->uv0[i];
	}
	m_vertexBuffer->Unmap();

	SetIntensity(1.0f);

	m_numCubemaps = GetNumSkyboxes();
}

void UniverseBox::Draw(Graphics::RenderState *rs)
{
	if(m_material->texture0)
		m_renderer->DrawBuffer(m_vertexBuffer.get(), rs, m_material.Get());
}

void UniverseBox::LoadCubeMap(Random &rand)
{
	if(m_numCubemaps>0) {
		const int new_ubox_index = rand.Int32(1, m_numCubemaps);
		if(new_ubox_index > 0) {
			// Load new one
			const std::string os = stringf("textures/skybox/ub%0{d}.dds", (new_ubox_index - 1));
			TextureBuilder texture_builder = TextureBuilder::Cube(os.c_str());
			m_cubemap.reset( texture_builder.CreateTexture(m_renderer) );
			m_material->texture0 = m_cubemap.get();
		}
	} else {
		// use default cubemap
		m_cubemap.reset();
		m_material->texture0 = s_defaultCubeMap.get();
	}
}

Starfield::Starfield(Graphics::Renderer *renderer, Random &rand)
{
	m_renderer = renderer;
	Init();
	Fill(rand);
}

void Starfield::Init()
{
	Graphics::MaterialDescriptor desc;
	desc.effect = Graphics::EFFECT_STARFIELD;
	desc.vertexColors = true;
	m_material.Reset(m_renderer->CreateMaterial(desc));
	m_material->emissive = Color::WHITE;
}

void Starfield::Fill(Random &rand)
{
	Graphics::VertexBufferDesc vbd;
	vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
	vbd.attrib[0].format = Graphics::ATTRIB_FORMAT_FLOAT3;
	vbd.attrib[1].semantic = Graphics::ATTRIB_DIFFUSE;
	vbd.attrib[1].format = Graphics::ATTRIB_FORMAT_UBYTE4;
	vbd.usage = Graphics::BUFFER_USAGE_STATIC;
	vbd.numVertices = BG_STAR_MAX;
	m_vertexBuffer.reset(m_renderer->CreateVertexBuffer(vbd));

	assert(sizeof(StarVert) == 16);
	assert(m_vertexBuffer->GetDesc().stride == sizeof(StarVert));
	auto vtxPtr = m_vertexBuffer->Map<StarVert>(Graphics::BUFFER_MAP_WRITE);
	//fill the array
	for (int i=0; i<BG_STAR_MAX; i++) {
		const Uint8 col = rand.Double(0.2,0.7)*255;

		// this is proper random distribution on a sphere's surface
		const float theta = float(rand.Double(0.0, 2.0*M_PI));
		const float u = float(rand.Double(-1.0, 1.0));

		vtxPtr->pos = vector3f(
			1000.0f * sqrt(1.0f - u*u) * cos(theta),
			1000.0f * u,
			1000.0f * sqrt(1.0f - u*u) * sin(theta));
		vtxPtr->col = Color(col, col, col,	255);

		//need to keep data around for HS anim - this is stupid
		m_hyperVtx[BG_STAR_MAX * 2 + i] = vtxPtr->pos;
		m_hyperCol[BG_STAR_MAX * 2 + i] = vtxPtr->col;

		vtxPtr++;
	}
	m_vertexBuffer->Unmap();
}

void Starfield::Draw(Graphics::RenderState *rs)
{
	// XXX would be nice to get rid of the Pi:: stuff here
	if (!Pi::game || Pi::player->GetFlightState() != Ship::HYPERSPACE) {
		m_renderer->DrawBuffer(m_vertexBuffer.get(), rs, m_material.Get(), Graphics::POINTS);
	} else {
		// roughly, the multiplier gets smaller as the duration gets larger.
		// the time-looking bits in this are completely arbitrary - I figured
		// it out by tweaking the numbers until it looked sort of right
		double mult = 0.0015 / (Pi::player->GetHyperspaceDuration() / (60.0*60.0*24.0*7.0));

		double hyperspaceProgress = Pi::game->GetHyperspaceProgress();

		const vector3d pz = Pi::player->GetOrient().VectorZ();	//back vector
		for (int i=0; i<BG_STAR_MAX; i++) {
			vector3f v = m_hyperVtx[BG_STAR_MAX * 2 + i] + vector3f(pz*hyperspaceProgress*mult);
			const Color &c = m_hyperCol[BG_STAR_MAX * 2 + i];

			m_hyperVtx[i*2] = m_hyperVtx[BG_STAR_MAX * 2 + i] + v;
			m_hyperCol[i*2] = c;

			m_hyperVtx[i*2+1] = v;
			m_hyperCol[i*2+1] = c;
		}
		m_renderer->DrawLines(BG_STAR_MAX*2, m_hyperVtx, m_hyperCol, rs);
	}
}

MilkyWay::MilkyWay(Graphics::Renderer *renderer)
{
	m_renderer = renderer;

	//build milky way model in two strips (about 256 verts)
	std::unique_ptr<Graphics::VertexArray> bottom(new VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE));
	std::unique_ptr<Graphics::VertexArray> top(new VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE));

	const Color dark(0);
	const Color bright(13, 13, 13, 13);

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
	m_material.Reset(m_renderer->CreateMaterial(desc));
	m_material->emissive = Color::WHITE;

	Graphics::VertexBufferDesc vbd;
	vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
	vbd.attrib[0].format = Graphics::ATTRIB_FORMAT_FLOAT3;
	vbd.attrib[1].semantic = Graphics::ATTRIB_DIFFUSE;
	vbd.attrib[1].format = Graphics::ATTRIB_FORMAT_UBYTE4;
	vbd.numVertices = bottom->GetNumVerts() + top->GetNumVerts();
	vbd.usage = Graphics::BUFFER_USAGE_STATIC;

	//two strips in one buffer, but seems to work ok without degenerate triangles
	m_vertexBuffer.reset(renderer->CreateVertexBuffer(vbd));
	assert(m_vertexBuffer->GetDesc().stride == sizeof(MilkyWayVert));
	auto vtxPtr = m_vertexBuffer->Map<MilkyWayVert>(Graphics::BUFFER_MAP_WRITE);
	for (Uint32 i = 0; i < top->GetNumVerts(); i++) {
		vtxPtr->pos = top->position[i];
		vtxPtr->col = top->diffuse[i];
		vtxPtr++;
	}
	for (Uint32 i = 0; i < bottom->GetNumVerts(); i++) {
		vtxPtr->pos = bottom->position[i];
		vtxPtr->col = bottom->diffuse[i];
		vtxPtr++;
	}
	m_vertexBuffer->Unmap();
}

void MilkyWay::Draw(Graphics::RenderState *rs)
{
	assert(m_vertexBuffer);
	assert(m_material);
	m_renderer->DrawBuffer(m_vertexBuffer.get(), rs, m_material.Get(), Graphics::TRIANGLE_STRIP);
}

Container::Container(Graphics::Renderer *renderer, Random &rand)
: m_renderer(renderer)
, m_milkyWay(renderer)
, m_starField(renderer, rand)
, m_universeBox(renderer)
, m_drawFlags( DRAW_SKYBOX | DRAW_STARS )
{
	Graphics::RenderStateDesc rsd;
	rsd.depthTest  = false;
	rsd.depthWrite = false;
	m_renderState = renderer->CreateRenderState(rsd);
	Refresh(rand);
}

void Container::Refresh(Random &rand)
{
	// always redo starfield, milkyway stays normal for now
	m_starField.Fill(rand);
	m_universeBox.LoadCubeMap(rand);
}

void Container::Draw(const matrix4x4d &transform)
{
	PROFILE_SCOPED()
	m_renderer->SetTransform(transform);
	if( DRAW_SKYBOX & m_drawFlags ) {
		m_universeBox.Draw(m_renderState);
	}
	if( DRAW_MILKY & m_drawFlags ) {
		m_milkyWay.Draw(m_renderState);
	}
	if( DRAW_STARS & m_drawFlags ) {
		// squeeze the starfield a bit to get more density near horizon
		matrix4x4d starTrans = transform * matrix4x4d::ScaleMatrix(1.0, 0.4, 1.0);
		m_renderer->SetTransform(starTrans);
		m_starField.Draw(m_renderState);
	}
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
