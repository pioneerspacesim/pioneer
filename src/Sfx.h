// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SFX_H
#define _SFX_H

#include "Body.h"
#include "Serializer.h"

class Frame;
namespace Graphics {
	class Renderer;
	class Material;
	namespace Drawables {
		class Sphere3D;
	}
}

class Sfx {
public:
	enum TYPE { TYPE_NONE, TYPE_EXPLOSION, TYPE_DAMAGE };

	static void Add(const Body *, TYPE);
	static void TimeStepAll(const float timeStep, Frame *f);
	static void RenderAll(Graphics::Renderer *r, Frame *f, const Frame *camFrame);
	static void Serialize(Serializer::Writer &wr, const Frame *f);
	static void Unserialize(Serializer::Reader &rd, Frame *f);

	Sfx();
	void SetPosition(const vector3d &p);
	vector3d GetPosition() const { return m_pos; }

	//create shared models
	static void Init(Graphics::Renderer *r);
	static void Uninit();
	static Graphics::Drawables::Sphere3D *shieldEffect;
	static Graphics::Drawables::Sphere3D *explosionEffect;
	static Graphics::Material *damageParticle;
	static Graphics::Material *ecmParticle;

private:
	static Sfx *AllocSfxInFrame(Frame *f);

	void Render(Graphics::Renderer *r, const matrix4x4d &transform);
	void TimeStepUpdate(const float timeStep);
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);

	vector3d m_pos;
	vector3d m_vel;
	float m_age;
	enum TYPE m_type;
};

#endif /* _SFX_H */
