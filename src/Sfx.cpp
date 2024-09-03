// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Sfx.h"

#include "Body.h"
#include "FileSystem.h"
#include "Frame.h"
#include "GameSaveError.h"
#include "Json.h"
#include "JsonUtils.h"
#include "ModelBody.h"
#include "Pi.h"
#include "StringF.h"
#include "core/IniConfig.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "matrix4x4.h"

using namespace Graphics;

namespace {
	float SizeToPixels(int screenHeight, const vector3f &trans, const float size)
	{
		//some hand-tweaked scaling, to make the lights seem larger from distance (final size is in pixels)
		// gl_PointSize = pixels_per_radian * point_diameter / distance( camera, pointcenter );
		// FIXME: this should reference a camera object instead of querying the window height
		const float pixrad = Clamp(screenHeight / trans.Length(), 0.1f, 50.0f);
		return (size * Graphics::GetFovFactor()) * pixrad;
	}

	float GetParticleSpeed(int screenHeight, SFX_TYPE t, const vector3f &pos, const Sfx &inst)
	{
		switch (t) {
		case TYPE_NONE: assert(false);
		case TYPE_EXPLOSION:
			return SizeToPixels(screenHeight, pos, inst.m_speed);
		case TYPE_DAMAGE:
			return SizeToPixels(screenHeight, pos, 20.f);
		case TYPE_SMOKE:
			return Clamp(SizeToPixels(screenHeight, pos, (inst.m_speed * inst.m_age)), 0.1f, 50.0f);
		default:
			return 0.f;
		}
	}
} // namespace

std::unique_ptr<Graphics::Material> SfxManager::damageParticle;
std::unique_ptr<Graphics::Material> SfxManager::ecmParticle;
std::unique_ptr<Graphics::Material> SfxManager::smokeParticle;
std::unique_ptr<Graphics::Material> SfxManager::explosionParticle;
SfxManager::MaterialData SfxManager::m_materialData[TYPE_NONE];

Sfx::Sfx(const vector3d &pos, const vector3d &vel, const float speed, const SFX_TYPE type) :
	m_pos(pos),
	m_vel(vel),
	m_age(0.0f),
	m_speed(speed),
	m_type(type)
{
}

Sfx::Sfx(const Json &jsonObj)
{
	try {
		Json sfxObj = jsonObj["sfx"];

		m_pos = sfxObj["pos"];
		m_vel = sfxObj["vel"];
		m_age = sfxObj["age"];
		m_type = sfxObj["type"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

void Sfx::SaveToJson(Json &jsonObj)
{
	Json sfxObj({}); // Create JSON object to contain sfx data.

	sfxObj["pos"] = m_pos;
	sfxObj["vel"] = m_vel;
	sfxObj["age"] = m_age;
	sfxObj["type"] = m_type;

	jsonObj["sfx"] = sfxObj; // Add sfx object to supplied object.
}

void Sfx::SetPosition(const vector3d &p)
{
	m_pos = p;
}

void Sfx::TimeStepUpdate(const float timeStep)
{
	PROFILE_SCOPED()
	m_age += timeStep;
	m_pos += m_vel * double(timeStep);

	switch (m_type) {
	case TYPE_EXPLOSION:
		if (m_age > 3.2) m_type = TYPE_NONE;
		break;
	case TYPE_DAMAGE:
		if (m_age > 2.0) m_type = TYPE_NONE;
		break;
	case TYPE_SMOKE:
		if (m_age > 8.0) m_type = TYPE_NONE;
		break;
	case TYPE_NONE: break;
	}
}

float Sfx::AgeBlend() const
{
	switch (m_type) {
	case TYPE_EXPLOSION: return (3.2 - m_age) / 3.2;
	case TYPE_DAMAGE: return (2.0 - m_age) / 2.0;
	case TYPE_SMOKE: return (8.0 - m_age) / 8.0;
	case TYPE_NONE: return 0.0f;
	}
	return 0.0f;
}

SfxManager::SfxManager()
{
	for (size_t t = 0; t < TYPE_NONE; t++) {
		m_instances[t].clear();
	}
}

void SfxManager::ToJson(Json &jsonObj, const FrameId fId)
{
	Frame *f = Frame::GetFrame(fId);
	Json sfxArray = Json::array(); // Create JSON array to contain sfx data.

	if (f->m_sfx) {
		for (size_t t = TYPE_EXPLOSION; t < TYPE_NONE; t++) {
			for (size_t i = 0; i < f->m_sfx->GetNumberInstances(SFX_TYPE(t)); i++) {
				Sfx &inst(f->m_sfx->GetInstanceByIndex(SFX_TYPE(t), i));
				if (inst.m_type != TYPE_NONE) {
					Json sfxArrayEl({}); // Create JSON object to contain sfx element.
					inst.SaveToJson(sfxArrayEl);
					sfxArray.push_back(sfxArrayEl); // Append sfx object to array.
				}
			}
		}
	}

	jsonObj["sfx_array"] = sfxArray; // Add sfx array to supplied object.
}

void SfxManager::FromJson(const Json &jsonObj, FrameId fId)
{
	Json sfxArray = jsonObj["sfx_array"].get<Json::array_t>();

	Frame *f = Frame::GetFrame(fId);

	if (sfxArray.size()) f->m_sfx.reset(new SfxManager);
	for (unsigned int i = 0; i < sfxArray.size(); ++i) {
		Sfx inst(sfxArray[i]);
		f->m_sfx->AddInstance(inst);
	}
}

SfxManager *SfxManager::AllocSfxInFrame(FrameId fId)
{
	Frame *f = Frame::GetFrame(fId);

	if (!f->m_sfx) {
		f->m_sfx.reset(new SfxManager);
	}

	return f->m_sfx.get();
}

void SfxManager::Add(const Body *b, SFX_TYPE t)
{
	assert(t != TYPE_NONE);
	SfxManager *sfxman = AllocSfxInFrame(b->GetFrame());
	if (!sfxman) return;
	vector3d vel(b->GetVelocity() + 200.0 * vector3d(Pi::rng.Double() - 0.5, Pi::rng.Double() - 0.5, Pi::rng.Double() - 0.5));
	Sfx sfx(b->GetPosition(), vel, 200, t);
	sfxman->AddInstance(sfx);
}

void SfxManager::AddExplosion(Body *b)
{
	SfxManager *sfxman = AllocSfxInFrame(b->GetFrame());
	if (!sfxman) return;

	float speed = 200.0f;
	if (b->IsType(ObjectType::SHIP)) {
		ModelBody *mb = static_cast<ModelBody *>(b);
		speed = mb->GetAabb().radius * 8.0;
	}
	Sfx sfx(b->GetPosition(), b->GetVelocity(), speed, TYPE_EXPLOSION);
	sfxman->AddInstance(sfx);
}

void SfxManager::AddThrustSmoke(const Body *b, const float speed, const vector3d &adjustpos)
{
	SfxManager *sfxman = AllocSfxInFrame(b->GetFrame());
	if (!sfxman) return;

	Sfx sfx(b->GetPosition() + adjustpos, vector3d(0, 0, 0), speed, TYPE_SMOKE);
	sfxman->AddInstance(sfx);
}

void SfxManager::TimeStepAll(const float timeStep, FrameId fId)
{
	PROFILE_SCOPED()

	Frame *f = Frame::GetFrame(fId);

	if (f->m_sfx) {
		for (size_t t = TYPE_EXPLOSION; t < TYPE_NONE; t++) {
			for (size_t i = 0; i < f->m_sfx->GetNumberInstances(SFX_TYPE(t)); i++) {
				Sfx &inst(f->m_sfx->GetInstanceByIndex(SFX_TYPE(t), i));
				inst.TimeStepUpdate(timeStep);
			}
		}
		f->m_sfx->Cleanup();
	}

	for (FrameId kid : f->GetChildren()) {
		TimeStepAll(timeStep, kid);
	}
}

void SfxManager::Cleanup()
{
	for (size_t t = TYPE_EXPLOSION; t < TYPE_NONE; t++) {
		const size_t numInstances = GetNumberInstances(SFX_TYPE(t));
		if (!numInstances)
			continue;

		for (Sint64 i = Sint64(numInstances - 1); i >= 0; i--) {
			Sfx &inst(GetInstanceByIndex(SFX_TYPE(t), i));
			if (inst.m_type == TYPE_NONE) {
				m_instances[t].erase(m_instances[t].begin() + i);
			}
		}
	}
}

void SfxManager::RenderAll(Renderer *renderer, FrameId fId, FrameId camFrameId)
{
	Frame *f = Frame::GetFrame(fId);
	int screenHeight = renderer->GetWindowHeight();

	PROFILE_SCOPED()
	if (f->m_sfx) {
		matrix4x4d ftran;
		Frame::GetFrameTransform(fId, camFrameId, ftran);

		for (size_t t = TYPE_EXPLOSION; t < TYPE_NONE; t++) {
			const size_t numInstances = f->m_sfx->GetNumberInstances(SFX_TYPE(t));
			if (!numInstances)
				continue;

			Graphics::Material *material = nullptr;
			switch (t) {
			case TYPE_NONE: assert(false); break;
			case TYPE_EXPLOSION:
				material = explosionParticle.get();
				break;
			case TYPE_DAMAGE:
				material = damageParticle.get();
				break;
			case TYPE_SMOKE:
				material = smokeParticle.get();
				break;
			}

			// NB - we're (ab)using the normal type to hold (uv coordinate offset value + point size)
			Graphics::VertexArray pointArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL, numInstances);

			for (size_t i = 0; i < numInstances; i++) {
				Sfx &inst(f->m_sfx->GetInstanceByIndex(SFX_TYPE(t), i));

				assert(inst.m_type == t);

				// make the particle position relative to the camera frame
				const vector3f pos(ftran * inst.m_pos);
				// pack UV offset and particle size in normal attribute
				const vector2f offset = CalculateOffset(SFX_TYPE(t), inst);
				const float speed = GetParticleSpeed(screenHeight, SFX_TYPE(t), pos, inst);

				pointArray.Add(pos, vector3f(offset, Clamp(speed, 0.1f, FLT_MAX)));
			}

			renderer->SetTransform(matrix4x4f::Identity());
			renderer->DrawBuffer(&pointArray, material);
		}
	}

	for (FrameId kid : f->GetChildren()) {
		RenderAll(renderer, kid, camFrameId);
	}
}

vector2f SfxManager::CalculateOffset(const enum SFX_TYPE type, const Sfx &inst)
{
	if (m_materialData[type].effect == Graphics::EFFECT_BILLBOARD_ATLAS) {
		const int spriteframe = inst.AgeBlend() * (m_materialData[type].num_textures - 1);
		const Sint32 numImgsWide = m_materialData[type].num_imgs_wide;
		const int u = (spriteframe % numImgsWide); // % is the "modulo operator", the remainder of i / width;
		const int v = (spriteframe / numImgsWide); // where "/" is an integer division
		return vector2f(
			float(u) / float(numImgsWide),
			float(v) / float(numImgsWide));
	}
	return vector2f(0.0f);
}

bool SfxManager::SplitMaterialData(const std::string &spec, MaterialData &output)
{
	static const std::string delim(",");

	enum dataEntries {
		eEFFECT = 0,
		eNUM_IMGS_WIDE,
		eNUM_TEXTURES,
		eCOORD_DOWNSCALE
	};

	size_t i = 0, start = 0, end = 0;
	while (end != std::string::npos) {
		// get to the first non-delim char
		start = spec.find_first_not_of(delim, end);

		// read the end, no more to do
		if (start == std::string::npos)
			break;

		// find the end - next delim or end of string
		end = spec.find_first_of(delim, start);

		// extract the fragment and remember it
		switch (i) {
		case eEFFECT:
			output.effect = (spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start) == "billboard") ? Graphics::EFFECT_BILLBOARD : Graphics::EFFECT_BILLBOARD_ATLAS;
			break;
		case eNUM_IMGS_WIDE:
			output.num_imgs_wide = atoi(spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start).c_str());
			break;
		case eNUM_TEXTURES:
			output.num_textures = atoi(spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start).c_str());
			break;
		default:
		case eCOORD_DOWNSCALE:
			assert(false);
			break;
		}
		i++;
	}

	output.coord_downscale = 1.0f / float(output.num_imgs_wide);
	return i == eCOORD_DOWNSCALE;
}

void SfxManager::Init(Graphics::Renderer *r)
{
	PROFILE_SCOPED()
	IniConfig cfg;
	// set defaults in case they're missing from the file
	cfg.SetString("damageFile", "textures/smoke.png");
	cfg.SetString("smokeFile", "textures/smoke.png");
	cfg.SetString("explosionFile", "textures/explosions/explosions.png");

	cfg.SetString("damagePacking", "billboard,1,1");
	cfg.SetString("smokePacking", "billboard,1,1");
	cfg.SetString("explosionPacking", "atlas,6,32");
	// load
	cfg.Read(FileSystem::gameDataFiles, "textures/Sfx.ini");

	// shared render states
	Graphics::RenderStateDesc alphaState;
	alphaState.blendMode = Graphics::BLEND_ALPHA;
	alphaState.depthWrite = false;
	alphaState.primitiveType = Graphics::POINTS;

	Graphics::RenderStateDesc additiveAlphaState;
	additiveAlphaState.blendMode = Graphics::BLEND_ALPHA_ONE;
	additiveAlphaState.depthWrite = false;
	additiveAlphaState.primitiveType = Graphics::POINTS;

	// materials
	Graphics::MaterialDescriptor desc;
	desc.textures = 1;

	// ECM effect is different, not managed by Sfx at all, should it be factored out?
	desc.effect = Graphics::EFFECT_BILLBOARD;
	ecmParticle.reset(r->CreateMaterial("billboards", desc, additiveAlphaState));
	ecmParticle->SetTexture("texture0"_hash,
		Graphics::TextureBuilder::Billboard("textures/ecm.png").GetOrCreateTexture(r, "billboard"));

	// load material definition data
	SplitMaterialData(cfg.String("explosionPacking"), m_materialData[TYPE_EXPLOSION]);
	SplitMaterialData(cfg.String("damagePacking"), m_materialData[TYPE_DAMAGE]);
	SplitMaterialData(cfg.String("smokePacking"), m_materialData[TYPE_SMOKE]);

	desc.effect = m_materialData[TYPE_DAMAGE].effect;
	damageParticle.reset(r->CreateMaterial("billboards", desc, additiveAlphaState));
	damageParticle->SetTexture("texture0"_hash,
		Graphics::TextureBuilder::Billboard(cfg.String("damageFile")).GetOrCreateTexture(r, "billboard"));
	if (desc.effect == Graphics::EFFECT_BILLBOARD_ATLAS)
		damageParticle->SetPushConstant("coordDownScale"_hash,
			m_materialData[TYPE_DAMAGE].coord_downscale);

	desc.effect = m_materialData[TYPE_SMOKE].effect;
	smokeParticle.reset(r->CreateMaterial("billboards", desc, alphaState));
	smokeParticle->SetTexture("texture0"_hash,
		Graphics::TextureBuilder::Billboard(cfg.String("smokeFile")).GetOrCreateTexture(r, "billboard"));
	if (desc.effect == Graphics::EFFECT_BILLBOARD_ATLAS)
		smokeParticle->SetPushConstant("coordDownScale"_hash,
			m_materialData[TYPE_SMOKE].coord_downscale);

	desc.effect = m_materialData[TYPE_EXPLOSION].effect;
	explosionParticle.reset(r->CreateMaterial("billboards", desc, alphaState));
	explosionParticle->SetTexture("texture0"_hash,
		Graphics::TextureBuilder::Billboard(cfg.String("explosionFile")).GetOrCreateTexture(r, "billboard"));
	if (desc.effect == Graphics::EFFECT_BILLBOARD_ATLAS)
		explosionParticle->SetPushConstant("coordDownScale"_hash,
			m_materialData[TYPE_EXPLOSION].coord_downscale);
}

void SfxManager::Uninit()
{
	damageParticle.reset();
	ecmParticle.reset();
	smokeParticle.reset();
	explosionParticle.reset();
}
