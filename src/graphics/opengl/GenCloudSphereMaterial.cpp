// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GenCloudSphereMaterial.h"
#include "TextureGL.h"
#include "graphics/Graphics.h"
#include "RendererGL.h"
#include <sstream>
#include "StringF.h"
#include "Ship.h"
#include "galaxy/StarSystem.h"

namespace Graphics {
namespace OGL {


GenCloudSphereProgram::GenCloudSphereProgram(const MaterialDescriptor &desc)
{
	//build some defines
	std::stringstream ss;
	if (desc.textures > 0)
		ss << "#define TEXTURE0\n";

	// get how many octaves we can use
	const Uint32 octaves = desc.quality;
	ss << "#define FBM_OCTAVES " << std::to_string(octaves) << std::endl;
	
	// No lights
	ss << "#define NUM_LIGHTS 0\n";

	m_name = "gen-cloud-sphere";
	m_defines = ss.str();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

void GenCloudSphereProgram::InitUniforms()
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

Program *GenCloudSphereMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	assert(desc.effect == EFFECT_GEN_CLOUDSPHERE_TEXTURE);
	return new GenCloudSphereProgram(desc);
}

void GenCloudSphereMaterial::Apply()
{
	PROFILE_SCOPED()
	OGL::Material::Apply();

	GenCloudSphereProgram *p = static_cast<GenCloudSphereProgram*>(m_program);

	const Graphics::GenCloudSphereMaterialParameters params = *static_cast<Graphics::GenCloudSphereMaterialParameters*>(this->specialParameter0);
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
	p->hueAdjust.Set(params.hueAdjust);
}

void GenCloudSphereMaterial::Unapply()
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
