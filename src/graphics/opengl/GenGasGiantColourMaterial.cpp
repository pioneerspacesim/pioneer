// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GenGasGiantColourMaterial.h"
#include "TextureGL.h"
#include "graphics/Graphics.h"
#include "RendererGL.h"
#include <sstream>
#include "StringF.h"
#include "Ship.h"
#include "galaxy/StarSystem.h"

namespace Graphics {
namespace OGL {


GenGasGiantColourProgram::GenGasGiantColourProgram(const MaterialDescriptor &desc)
{
	//build some defines
	std::stringstream ss;
	if (desc.textures > 0)
		ss << "#define TEXTURE0\n";

	// this masking hack is because I also need to encode data in the UPPER 16-bits
	const Uint32 quality = desc.quality & 0x0000FFFF;
	switch( quality )
	{
	default:
	case GEN_JUPITER_TEXTURE:
		ss << "#define GEN_JUPITER_ESQUE\n";
		break;
	case GEN_SATURN_TEXTURE:
		ss << "#define GEN_SATURN_ESQUE\n";
		break;
	case GEN_SATURN2_TEXTURE:
		ss << "#define GEN_SATURN2_ESQUE\n";
		break;
		// technically Ice Giants not Gas Giants...
	case GEN_NEPTUNE_TEXTURE:
		ss << "#define GEN_NEPTUNE_ESQUE\n";
		break;
	case GEN_NEPTUNE2_TEXTURE:
		ss << "#define GEN_NEPTUNE2_ESQUE\n";
		break;
	case GEN_URANUS_TEXTURE:
		ss << "#define GEN_URANUS_ESQUE\n";
		break;
	}
	// extract the top 16-bits to get how many octaves we can use
	const Uint32 octaves = (desc.quality & 0xFFFF0000) >> 16;
	ss << "#define FBM_OCTAVES " << std::to_string(octaves) << std::endl;

	// No lights
	ss << "#define NUM_LIGHTS 0\n";

	m_name = "gen-gas-giant-colour";
	m_defines = ss.str();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

void GenGasGiantColourProgram::InitUniforms()
{
	Program::InitUniforms();

	v0.Init("v0", m_program);
	v1.Init("v1", m_program);
	v2.Init("v2", m_program);
	v3.Init("v3", m_program);
	fracStep.Init("fracStep", m_program);

	permTexture.Init("permTexture", m_program);
	gradTexture.Init("gradTexture", m_program);
	time.Init("time", m_program);

	frequency.Init("frequency", m_program);
	hueAdjust.Init("hueAdjust", m_program);
}

Program *GenGasGiantColourMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	assert(desc.effect == EFFECT_GEN_GASGIANT_TEXTURE);
	assert(desc.dirLights < 5);
	return new GenGasGiantColourProgram(desc);
}

void GenGasGiantColourMaterial::Apply()
{
	PROFILE_SCOPED()
	OGL::Material::Apply();

	GenGasGiantColourProgram *p = static_cast<GenGasGiantColourProgram*>(m_program);

	const Graphics::GenGasGiantColourMaterialParameters params = *static_cast<Graphics::GenGasGiantColourMaterialParameters*>(this->specialParameter0);
	assert(params.v);
	p->v0.Set( params.v[0] );
	p->v1.Set( params.v[1] );
	p->v2.Set( params.v[2] );
	p->v3.Set( params.v[3] );
	p->fracStep.Set( params.fracStep );

	// Quad generation specific paramters
	p->permTexture.Set(this->texture0, 0);
	p->gradTexture.Set(this->texture1, 1);
	p->time.Set(params.time);
	p->frequency.Set(params.frequency);
	p->hueAdjust.Set(params.hueAdjust);

	//Light uniform parameters
	for (Uint32 i = 0; i<m_renderer->GetNumLights(); i++) {
		const Light& Light = m_renderer->GetLight(i);
		p->lights[i].diffuse.Set(Light.GetDiffuse());
		p->lights[i].specular.Set(Light.GetSpecular());
		const vector3f& pos = Light.GetPosition();
		p->lights[i].position.Set(pos.x, pos.y, pos.z, (Light.GetType() == Light::LIGHT_DIRECTIONAL ? 0.f : 1.f));
	}

	p->diffuse.Set(this->diffuse);

	if( this->texture2 )
	{
		p->texture2.Set(this->texture2, 2);
	}
}

void GenGasGiantColourMaterial::Unapply()
{
	PROFILE_SCOPED()
	// Might not be necessary to unbind textures, but let's not old graphics code (eg, old-UI)
	if (texture4) {
		static_cast<TextureGL*>(texture4)->Unbind();
		glActiveTexture(GL_TEXTURE3);
	}
	if (texture3) {
		static_cast<TextureGL*>(texture3)->Unbind();
		glActiveTexture(GL_TEXTURE2);
	}
	if (texture2) {
		static_cast<TextureGL*>(texture2)->Unbind();
		glActiveTexture(GL_TEXTURE1);
	}
	if (texture1) {
		static_cast<TextureGL*>(texture1)->Unbind();
		glActiveTexture(GL_TEXTURE0);
	}
	if (texture0) {
		static_cast<TextureGL*>(texture0)->Unbind();
	}
}

}
}
