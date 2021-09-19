// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Background.h"

#include "FileSystem.h"
#include "Game.h"
#include "GameConfig.h"
#include "MathUtil.h"
#include "Pi.h"
#include "Player.h"
#include "Space.h"
#include "StringF.h"
#include "galaxy/StarSystem.h"
#include "graphics/Graphics.h"
#include "graphics/RenderState.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "perlin.h"
#include "profiler/Profiler.h"

#include <SDL_stdinc.h>
#include <iostream>
#include <sstream>

using namespace Graphics;

namespace {
	constexpr Uint32 BG_STAR_MAX = 500000;
	constexpr Uint32 BG_STAR_MIN = 1;
	constexpr Sint32 BG_STAR_RADIUS_MAX = 500;
	constexpr Uint32 NUM_HYPERSPACE_STARS = 8000;
	static RefCountedPtr<Graphics::Texture> s_defaultCubeMap;

	static Uint32 GetNumSkyboxes()
	{
		char filename[1024];
		snprintf(filename, sizeof(filename), "textures/skybox");
		std::vector<FileSystem::FileInfo> fileList;
		FileSystem::gameDataFiles.ReadDirectory(filename, fileList);

		const char *itemMask = "ub";

		Uint32 num_matching = 0;
		for (std::vector<FileSystem::FileInfo>::const_iterator it = fileList.begin(), itEnd = fileList.end(); it != itEnd; ++it) {
			if (starts_with((*it).GetName(), itemMask)) {
				++num_matching;
			}
		}
		return num_matching;
	}
} // namespace

namespace Background {

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
		m_material->emissive = Color(intensity * 255, intensity * 255, intensity * 255);
	}

	UniverseBox::UniverseBox(Graphics::Renderer *renderer)
	{
		m_renderer = renderer;
		Init();
	}

	UniverseBox::~UniverseBox()
	{
	}

	static size_t s_texture0Name = Graphics::Renderer::GetName("texture0");
	void UniverseBox::Init()
	{
		// Load default cubemap
		if (!s_defaultCubeMap.Valid()) {
			TextureBuilder texture_builder = TextureBuilder::Cube("textures/skybox/skybox.dds");
			s_defaultCubeMap.Reset(texture_builder.GetOrCreateTexture(m_renderer, std::string("cube")));
		}

		// Create skybox geometry
		std::unique_ptr<Graphics::VertexArray> box(new VertexArray(ATTRIB_POSITION | ATTRIB_UV0, 36));
		const float vp = 1000.0f;
		// Top +Y
		box->Add(vector3f(-vp, vp, vp), vector2f(0.0f, 0.0f));
		box->Add(vector3f(-vp, vp, -vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(vp, vp, vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(vp, vp, vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(-vp, vp, -vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(vp, vp, -vp), vector2f(1.0f, 1.0f));
		// Bottom -Y
		box->Add(vector3f(-vp, -vp, -vp), vector2f(0.0f, 0.0f));
		box->Add(vector3f(-vp, -vp, vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(vp, -vp, -vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(vp, -vp, -vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(-vp, -vp, vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(vp, -vp, vp), vector2f(1.0f, 1.0f));
		// Front -Z
		box->Add(vector3f(-vp, vp, -vp), vector2f(0.0f, 0.0f));
		box->Add(vector3f(-vp, -vp, -vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(vp, vp, -vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(vp, vp, -vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(-vp, -vp, -vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(vp, -vp, -vp), vector2f(1.0f, 1.0f));
		// Back +Z
		box->Add(vector3f(vp, vp, vp), vector2f(0.0f, 0.0f));
		box->Add(vector3f(vp, -vp, vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(-vp, vp, vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(-vp, vp, vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(vp, -vp, vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(-vp, -vp, vp), vector2f(1.0f, 1.0f));
		// Right +X
		box->Add(vector3f(vp, vp, -vp), vector2f(0.0f, 0.0f));
		box->Add(vector3f(vp, -vp, -vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(vp, vp, vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(vp, vp, vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(vp, -vp, -vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(vp, -vp, vp), vector2f(1.0f, 1.0f));
		// Left -X
		box->Add(vector3f(-vp, vp, vp), vector2f(0.0f, 0.0f));
		box->Add(vector3f(-vp, -vp, vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(-vp, vp, -vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(-vp, vp, -vp), vector2f(1.0f, 0.0f));
		box->Add(vector3f(-vp, -vp, vp), vector2f(0.0f, 1.0f));
		box->Add(vector3f(-vp, -vp, -vp), vector2f(1.0f, 1.0f));

		Graphics::MaterialDescriptor desc;
		Graphics::RenderStateDesc stateDesc;
		stateDesc.depthTest = false;
		stateDesc.depthWrite = false;

		m_material.Reset(m_renderer->CreateMaterial("skybox", desc, stateDesc));
		m_material->diffuse = Color4f(0.8, 0.8, 0.8, 1.0);

		//create buffer and upload data
		Graphics::VertexBufferDesc vbd = Graphics::VertexBufferDesc::FromAttribSet(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
		vbd.numVertices = box->GetNumVerts();
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;

		Graphics::VertexBuffer *vertexBuf = m_renderer->CreateVertexBuffer(vbd);

		SkyboxVert *vtxPtr = vertexBuf->Map<SkyboxVert>(Graphics::BUFFER_MAP_WRITE);
		assert(vertexBuf->GetDesc().stride == sizeof(SkyboxVert));
		for (Uint32 i = 0; i < box->GetNumVerts(); i++) {
			vtxPtr[i].pos = box->position[i];
			vtxPtr[i].uv = box->uv0[i];
		}
		vertexBuf->Unmap();

		SetIntensity(1.0f);

		m_universeBox.reset(m_renderer->CreateMeshObject(vertexBuf));
		m_numCubemaps = GetNumSkyboxes();
	}

	void UniverseBox::Draw()
	{
		if (m_cubemap.Valid())
			m_renderer->DrawMesh(m_universeBox.get(), m_material.Get());
	}

	void UniverseBox::LoadCubeMap(Random &rand)
	{
		if (m_numCubemaps > 0) {
			const int new_ubox_index = rand.Int32(1, m_numCubemaps);
			if (new_ubox_index > 0) {
				// Load new one
				const std::string os = stringf("textures/skybox/ub%0{d}.dds", (new_ubox_index - 1));
				TextureBuilder texture_builder = TextureBuilder::Cube(os.c_str());
				m_cubemap.Reset(texture_builder.GetOrCreateTexture(m_renderer, std::string("cube")));
			}
		} else {
			// use default cubemap
			m_cubemap.Reset(s_defaultCubeMap.Get());
		}

		m_material->SetTexture(s_texture0Name, m_cubemap.Get());
	}

	Starfield::Starfield(Graphics::Renderer *renderer, Random &rand, const SystemPath *const systemPath, RefCountedPtr<Galaxy> galaxy)
	{
		m_renderer = renderer;
		Init();
		Fill(rand, systemPath, galaxy);
	}

	void Starfield::Init()
	{
		PROFILE_SCOPED()

		// Create material to be used with starfield points
		Graphics::MaterialDescriptor desc;

		Graphics::RenderStateDesc stateDesc;
		stateDesc.depthTest = false;
		stateDesc.depthWrite = false;
		stateDesc.blendMode = Graphics::BLEND_ALPHA;
		stateDesc.primitiveType = Graphics::POINTS;

		m_material.Reset(m_renderer->CreateMaterial("starfield", desc, stateDesc));
		Graphics::Texture *texture = Graphics::TextureBuilder::Billboard("textures/star_point.png").GetOrCreateTexture(m_renderer, "billboard");
		m_material->SetTexture(Graphics::Renderer::GetName("texture0"), texture);
		m_material->emissive = Color::WHITE;

		// Create material to be used with hyperjump 'star streaks'
		Graphics::MaterialDescriptor descStreaks;
		Graphics::RenderStateDesc stateDescStreaks = stateDesc;
		stateDescStreaks.primitiveType = Graphics::LINE_SINGLE;
		m_materialStreaks.Reset(m_renderer->CreateMaterial("vtxColor", descStreaks, stateDescStreaks));
		m_materialStreaks->emissive = Color::WHITE;

		IniConfig cfg;
		cfg.Read(FileSystem::gameDataFiles, "configs/Starfield.ini");
		// NB: limit the ranges of all values loaded from the file
		m_rMin = Clamp(cfg.Float("rMin", 0.6), 0.2f, 1.0f);
		m_rMax = Clamp(cfg.Float("rMax", 1.0), 0.2f, 1.0f);
		m_gMin = Clamp(cfg.Float("gMin", 0.6), 0.2f, 1.0f);
		m_gMax = Clamp(cfg.Float("gMax", 1.0), 0.2f, 1.0f);
		m_bMin = Clamp(cfg.Float("bMin", 0.6), 0.2f, 1.0f);
		m_bMax = Clamp(cfg.Float("bMax", 1.0), 0.2f, 1.0f);
	}

	void Starfield::Fill(Random &rand, const SystemPath *const systemPath, RefCountedPtr<Galaxy> galaxy)
	{
		PROFILE_SCOPED()
		const Uint32 NUM_BG_STARS = MathUtil::mix(BG_STAR_MIN, BG_STAR_MAX, Pi::GetAmountBackgroundStars());
		const float brightnessApparentSizeFactor = Pi::GetStarFieldStarSizeFactor() / 7.0;
		// dividing by 7 to make sure that 100% star size isn't too big to clash with UI elements

		m_hyperVtx.reset(new vector3f[NUM_HYPERSPACE_STARS * 3]);
		m_hyperCol.reset(new Color[NUM_HYPERSPACE_STARS * 3]);
		{
			Graphics::VertexBufferDesc vbd = VertexBufferDesc::FromAttribSet(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE);
			vbd.usage = Graphics::BUFFER_USAGE_DYNAMIC;
			vbd.numVertices = NUM_HYPERSPACE_STARS * 2;
			// this vertex buffer will be owned by the animMesh object
			Graphics::VertexBuffer *vtxBuffer = m_renderer->CreateVertexBuffer(vbd);
			m_animMesh.reset(m_renderer->CreateMeshObject(vtxBuffer));
		}

		m_pointSprites.reset(new Graphics::Drawables::PointSprites);

		assert(sizeof(StarVert) == 16);
		std::unique_ptr<vector3f[]> stars(new vector3f[NUM_BG_STARS]);
		std::unique_ptr<Color[]> colors(new Color[NUM_BG_STARS]);
		std::unique_ptr<float[]> sizes(new float[NUM_BG_STARS]);
		std::unique_ptr<float[]> brightness(new float[NUM_BG_STARS]);

		//fill the array
		Uint32 num = 0;
		if (systemPath && galaxy.Valid()) {
			PROFILE_SCOPED_DESC("Pick Stars from Galaxy")

			/* the number of visible systems is in a cubic relationship with the visible radius,
			i.e. visibleRadius = x * numberSystems^(1/3)
			I experimentally determined that x is approximately 3.89
			and that stays probably the same as long as the galaxy has the same system density */
			const Sint32 visibleRadius = std::min<Sint32>(BG_STAR_RADIUS_MAX, 3.89 * pow((float)NUM_BG_STARS, 1.0 / 3.0));

			const Sint32 visibleRadiusSqr = (visibleRadius * visibleRadius);
			const Sint32 sectorMin = -(visibleRadius / Sector::SIZE); // lyrs_radius / sector_size_in_lyrs
			const Sint32 sectorMax = visibleRadius / Sector::SIZE;	  // lyrs_radius / sector_size_in_lyrs
			// fill star array
			for (Sint32 x = sectorMin; x <= sectorMax; ++x) {
				for (Sint32 y = sectorMin; y <= sectorMax; ++y) {
					for (Sint32 z = sectorMin; z <= sectorMax; ++z) {
						SystemPath sys(systemPath->sectorX + x, systemPath->sectorY + y, systemPath->sectorZ + z);
						if (SystemPath::SectorDistanceSqr(sys, *systemPath) * Sector::SIZE >= visibleRadiusSqr)
							continue; // early out

						// this is fairly expensive
						RefCountedPtr<const Sector> sec = galaxy->GetSector(sys);

						// add as many systems as we can
						const size_t numSystems = std::min(sec->m_systems.size(), (size_t)(NUM_BG_STARS - num));
						for (size_t systemIndex = 0; systemIndex < numSystems; systemIndex++) {
							const Sector::System *ss = &(sec->m_systems[systemIndex]);
							const vector3f distance = Sector::SIZE * vector3f(systemPath->sectorX, systemPath->sectorY, systemPath->sectorZ) - ss->GetFullPosition();
							if (distance.LengthSqr() >= visibleRadiusSqr)
								continue; // too far

							// add the colors and luminosities of all stars in a system together
							float luminositySystemSum = 0.0f;
							vector3f colorSystemSum(0.0f, 0.0f, 0.0f);
							for (size_t i = 0; i < ss->GetNumStars(); ++i) {
								luminositySystemSum += StarSystem::starLuminosities[ss->GetStarType(i)];
								Color col = StarSystem::starRealColors[ss->GetStarType(i)];
								colorSystemSum += vector3f(col.r, col.g, col.b) * StarSystem::starLuminosities[ss->GetStarType(i)];
							}
							colorSystemSum /= luminositySystemSum;

							Color col(colorSystemSum.x, colorSystemSum.y, colorSystemSum.z);
							col.r = Clamp(col.r, (Uint8)(m_rMin * 255), (Uint8)(m_rMax * 255));
							col.g = Clamp(col.g, (Uint8)(m_gMin * 255), (Uint8)(m_gMax * 255));
							col.b = Clamp(col.b, (Uint8)(m_bMin * 255), (Uint8)(m_bMax * 255));
							//const Color col(Color::PINK); // debug pink

							// copy the data
							stars[num] = distance.Normalized() * 1000.0f;
							colors[num] = col;
							brightness[num] = luminositySystemSum / (4 * M_PI * distance.Length() * distance.Length());

							num++;
						}
						if (num >= NUM_BG_STARS) {
							x = sectorMax;
							y = sectorMax;
							z = sectorMax;
							break;
						}
					}
				}
			}
		}
		Output("Stars picked from galaxy: %d\n", num);

		// use a logarithmic scala for brightness since this looks more natural to the human eye
		for (uint32_t i = 0; i < num; ++i) {
			brightness[i] = log(brightness[i]);
		}

		// find the median brightness of all visible stars
		std::vector<int> sortedBrightnessIndex;
		for (Uint32 i = 0; i < num; ++i) {
			sortedBrightnessIndex.push_back(i);
		}
		std::sort(sortedBrightnessIndex.begin(), sortedBrightnessIndex.end(), [&](const int a, const int b) {
			return brightness[a] > brightness[b];
		});
		double medianBrightness = 0.0;
		constexpr float medianPosition = 0.7;
		if (num > 0) {
			medianBrightness = brightness[sortedBrightnessIndex[Clamp<int>(medianPosition * num, 0, num - 1)]];
		}

		for (size_t j = 0; j < num; ++j) {
			size_t i = sortedBrightnessIndex[j]; // just for debugging purposes

			// dividing through the median helps bringing the logarithmic brightnesses to a scala that is easier to work with
			brightness[i] /= medianBrightness;
			// the exponentiation helps to emphasize very bright stars
			constexpr float brightnessPower = 7.5;
			brightness[i] = std::pow(brightness[i], brightnessPower);

			sizes[i] = std::max(brightnessApparentSizeFactor * brightness[i], 0.0f);

			// convert temporarily to floats to prevent narrowing errors
			float colorR = colors[i].r;
			float colorG = colors[i].g;
			float colorB = colors[i].b;

			// find a color scaling factor that doesn't make a colored star look white
			float colorMax = std::max({ colorR, colorG, colorB });
			float scaledColorMax = colorMax * brightness[i];
			const float colorFactor = std::min(scaledColorMax, 255.0f) / colorMax;

			colorR *= colorFactor;
			colorG *= colorFactor;
			colorB *= colorFactor;
			colors[i].r = Clamp<int>(colorR, 0, 255);
			colors[i].g = Clamp<int>(colorG, 0, 255);
			colors[i].b = Clamp<int>(colorB, 0, 255);
		}

		// fill out the remaining target count with generated points and also fill hyperspace stars
		for (Uint32 i = 0; i < std::max(NUM_BG_STARS, NUM_HYPERSPACE_STARS); i++) {
			const double size = rand.Double(0.2, 0.9);
			const Uint8 colScale = size * 255;

			const Color col(
				rand.Double(m_rMin, m_rMax) * colScale,
				rand.Double(m_gMin, m_gMax) * colScale,
				rand.Double(m_bMin, m_bMax) * colScale,
				255);

			// this is proper random distribution on a sphere's surface
			const float theta = float(rand.Double(0.0, 2.0 * M_PI));
			const float u = float(rand.Double(-1.0, 1.0));

			// squeeze the starfield a bit to get more density near horizon using matrix3x3f::Scale
			const auto star = matrix3x3f::Scale(1.0, 0.4, 1.0) * (vector3f(sqrt(1.0f - u * u) * cos(theta), u, sqrt(1.0f - u * u) * sin(theta)).Normalized() * 1000.0f);

			if (i >= num && i < NUM_BG_STARS) {
				sizes[i] = size;
				stars[i] = star;
				colors[i] = col;
				num++;
			}
			if (i < NUM_HYPERSPACE_STARS) {
				m_hyperVtx[NUM_HYPERSPACE_STARS * 2 + i] = star;
				m_hyperCol[NUM_HYPERSPACE_STARS * 2 + i] = Color::WHITE * 0.8;
			}
		}
		Output("Final stars number: %d\n", num);

		m_pointSprites->SetData(NUM_BG_STARS, stars.get(), colors.get(), sizes.get());
	}

	void Starfield::Draw()
	{
		PROFILE_SCOPED()
		// XXX would be nice to get rid of the Pi:: stuff here
		if (!Pi::game || Pi::player->GetFlightState() != Ship::HYPERSPACE) {
			m_pointSprites->Draw(m_renderer, m_material.Get());
		} else {
			Graphics::VertexBuffer *buffer = m_animMesh->GetVertexBuffer();
			assert(sizeof(StarVert) == 16);
			assert(buffer->GetDesc().stride == sizeof(StarVert));
			auto vtxPtr = buffer->Map<StarVert>(Graphics::BUFFER_MAP_WRITE);

			// roughly, the multiplier gets smaller as the duration gets larger.
			// the time-looking bits in this are completely arbitrary - I figured
			// it out by tweaking the numbers until it looked sort of right
			const double mult = 0.001 / (Pi::player->GetHyperspaceDuration() / (60.0 * 60.0 * 24.0 * 7.0));

			const double hyperspaceProgress = Pi::game->GetHyperspaceProgress();

			const Sint32 numStars = buffer->GetDesc().numVertices / 2;

			const vector3d pz = Pi::player->GetOrient().VectorZ(); //back vector
			for (int i = 0; i < numStars; i++) {
				vector3f v = m_hyperVtx[numStars * 2 + i] + vector3f(pz * hyperspaceProgress * mult);
				const Color &c = m_hyperCol[numStars * 2 + i];

				vtxPtr[i * 2].pos = m_hyperVtx[i * 2] = m_hyperVtx[numStars * 2 + i] + v;
				vtxPtr[i * 2].col = m_hyperCol[i * 2] = c;

				vtxPtr[i * 2 + 1].pos = m_hyperVtx[i * 2 + 1] = v;
				vtxPtr[i * 2 + 1].col = m_hyperCol[i * 2 + 1] = c;
			}
			buffer->Unmap();
			m_renderer->DrawMesh(m_animMesh.get(), m_materialStreaks.Get());
		}
	}

	MilkyWay::MilkyWay(Graphics::Renderer *renderer)
	{
		m_renderer = renderer;

		//build milky way model in two strips (about 256 verts)
		std::unique_ptr<Graphics::VertexArray> bottom(new VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE));
		std::unique_ptr<Graphics::VertexArray> top(new VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE));

		const Color dark(Color::BLANK);
		const Color bright(13, 13, 13, 13);

		//bottom
		float theta;
		for (theta = 0.0; theta < 2.f * float(M_PI); theta += 0.1f) {
			bottom->Add(
				vector3f(100.0f * sin(theta), float(-40.0 - 30.0 * noise(vector3d(sin(theta), 1.0, cos(theta)))), 100.0f * cos(theta)),
				dark);
			bottom->Add(
				vector3f(100.0f * sin(theta), float(5.0 * noise(vector3d(sin(theta), 0.0, cos(theta)))), 100.0f * cos(theta)),
				bright);
		}
		theta = 2.f * float(M_PI);
		bottom->Add(
			vector3f(100.0f * sin(theta), float(-40.0 - 30.0 * noise(vector3d(sin(theta), 1.0, cos(theta)))), 100.0f * cos(theta)),
			dark);
		bottom->Add(
			vector3f(100.0f * sin(theta), float(5.0 * noise(vector3d(sin(theta), 0.0, cos(theta)))), 100.0f * cos(theta)),
			bright);
		//top
		for (theta = 0; theta < 2.f * float(M_PI); theta += 0.1f) {
			top->Add(
				vector3f(100.0f * sin(theta), float(5.0 * noise(vector3d(sin(theta), 0.0, cos(theta)))), 100.0f * cos(theta)),
				bright);
			top->Add(
				vector3f(100.0f * sin(theta), float(40.0 + 30.0 * noise(vector3d(sin(theta), -1.0, cos(theta)))), 100.0f * cos(theta)),
				dark);
		}
		theta = 2.f * float(M_PI);
		top->Add(
			vector3f(100.0f * sin(theta), float(5.0 * noise(vector3d(sin(theta), 0.0, cos(theta)))), 100.0f * cos(theta)),
			bright);
		top->Add(
			vector3f(100.0f * sin(theta), float(40.0 + 30.0 * noise(vector3d(sin(theta), -1.0, cos(theta)))), 100.0f * cos(theta)),
			dark);

		Graphics::MaterialDescriptor desc;
		Graphics::RenderStateDesc stateDesc;
		stateDesc.depthTest = false;
		stateDesc.depthWrite = false;
		stateDesc.primitiveType = Graphics::TRIANGLE_STRIP;
		m_material.Reset(m_renderer->CreateMaterial("starfield", desc, stateDesc));
		m_material->emissive = Color::WHITE;

		Graphics::VertexBufferDesc vbd = VertexBufferDesc::FromAttribSet(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE);
		vbd.numVertices = bottom->GetNumVerts() + top->GetNumVerts();
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;

		//two strips in one buffer, but seems to work ok without degenerate triangles
		Graphics::VertexBuffer *vtxBuffer = renderer->CreateVertexBuffer(vbd);
		assert(vtxBuffer->GetDesc().stride == sizeof(MilkyWayVert));
		auto vtxPtr = vtxBuffer->Map<MilkyWayVert>(Graphics::BUFFER_MAP_WRITE);
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
		vtxBuffer->Unmap();

		m_meshObject.reset(m_renderer->CreateMeshObject(vtxBuffer));
	}

	void MilkyWay::Draw()
	{
		assert(m_meshObject);
		assert(m_material);
		m_renderer->DrawMesh(m_meshObject.get(), m_material.Get());
	}

	Container::Container(Graphics::Renderer *renderer, Random &rand, const Space *space, RefCountedPtr<Galaxy> galaxy, const SystemPath *const systemPath) :
		m_renderer(renderer),
		m_milkyWay(renderer),
		m_starField(renderer, rand, space && space->GetStarSystem() ? &space->GetStarSystem()->GetPath() : systemPath, galaxy),
		m_universeBox(renderer),
		m_drawFlags(DRAW_SKYBOX | DRAW_STARS)
	{
		m_universeBox.LoadCubeMap(rand);
	}

	void Container::Draw(const matrix4x4d &transform)
	{
		PROFILE_SCOPED()
		m_renderer->SetTransform(matrix4x4f(transform));
		if (DRAW_SKYBOX & m_drawFlags) {
			m_universeBox.Draw();
		}
		if (DRAW_MILKY & m_drawFlags) {
			m_milkyWay.Draw();
		}
		if (DRAW_STARS & m_drawFlags) {
			m_renderer->SetTransform(matrix4x4f(transform));
			m_starField.Draw();
		}
	}

	void Container::SetIntensity(float intensity)
	{
		PROFILE_SCOPED()
		intensity = Clamp(intensity, 0.0f, 1.0f);
		m_universeBox.SetIntensity(intensity);
		m_starField.SetIntensity(intensity);
		m_milkyWay.SetIntensity(intensity);
	}

	void Container::SetDrawFlags(const Uint32 flags)
	{
		m_drawFlags = flags;
	}

} //namespace Background
