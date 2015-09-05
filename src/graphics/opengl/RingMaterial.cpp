// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RingMaterial.h"
#include "StringF.h"
#include "graphics/Graphics.h"
#include "RendererGL.h"
#include "TextureGL.h"

namespace Graphics {
namespace OGL {

Program *RingMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	assert(desc.textures == 1);
	//pick light count and some defines
	unsigned int numLights = Clamp(desc.dirLights, 1u, 4u);
	std::string defines = stringf("#define NUM_LIGHTS %0{u}\n", numLights);
	return new Program("planetrings", defines);
}

void RingMaterial::Apply()
{
	OGL::Material::Apply();

	assert(this->texture0);
	static_cast<TextureGL*>(texture0)->Bind();
	m_program->texture0.Set(0);

	//Light uniform parameters
	for( Uint32 i=0 ; i<m_renderer->GetNumLights() ; i++ ) {
		const Light& Light = m_renderer->GetLight(i);
		m_program->lights[i].diffuse.Set( Light.GetDiffuse() );
		m_program->lights[i].specular.Set( Light.GetSpecular() );
		const vector3f& pos = Light.GetPosition();
		m_program->lights[i].position.Set( pos.x, pos.y, pos.z, (Light.GetType() == Light::LIGHT_DIRECTIONAL ? 0.f : 1.f));
	}
}

void RingMaterial::Unapply()
{
	static_cast<TextureGL*>(texture0)->Unbind();
}

}
}
