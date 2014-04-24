// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShieldMaterial.h"
#include "graphics/Material.h"
#include "graphics/TextureGL.h"
#include "graphics/Graphics.h"
#include "graphics/RendererGL2.h"
#include <sstream>
#include "StringF.h"
#include "Shields.h"

namespace Graphics {
namespace GL2 {

ShieldProgram::ShieldProgram(const MaterialDescriptor &desc)
{
	//build some defines
	std::stringstream ss;

	m_name = "shield";
	m_defines = ss.str();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

void ShieldProgram::InitUniforms()
{
	Program::InitUniforms();

	shieldStrength.Init("shieldStrength", m_program);
	shieldCooldown.Init("shieldCooldown", m_program);

	char whatsInAName[256];
	for (Sint32 i = 0; i < MAX_SHIELD_HITS; i++) {
		sprintf(whatsInAName, "hitPos[%d]", i);
		hitPos[i].Init(whatsInAName, m_program);
	}
	for (Sint32 i = 0; i < MAX_SHIELD_HITS; i++) {
		sprintf(whatsInAName, "radii[%d]", i);
		radii[i].Init(whatsInAName, m_program);
	}
	numHits.Init("numHits", m_program);
}

Program *ShieldMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	return new ShieldProgram(desc);
}

void ShieldMaterial::Apply()
{
	ShieldProgram *p = static_cast<ShieldProgram*>(m_program);
	p->Use();
	p->invLogZfarPlus1.Set(m_renderer->m_invLogZfarPlus1);

	p->diffuse.Set(this->diffuse);

	if (this->specialParameter0) {
		const ShieldRenderParameters srp = *static_cast<ShieldRenderParameters*>(this->specialParameter0);
		p->shieldStrength.Set(srp.strength);
		p->shieldCooldown.Set(srp.coolDown);
		for (Sint32 i = 0; i < srp.numHits && i < MAX_SHIELD_HITS; i++) {
			p->hitPos[i].Set( srp.hitPos[i] );
		}
		for (Sint32 i = 0; i < srp.numHits && i < MAX_SHIELD_HITS; i++) {
			p->radii[i].Set( srp.radii[i] );
		}
		p->numHits.Set( int(std::min(srp.numHits, MAX_SHIELD_HITS)) );
	} else {
		p->shieldStrength.Set(0.0f);
		p->shieldCooldown.Set(0.0f);
		p->numHits.Set( 0 );
	}
}

}
}
