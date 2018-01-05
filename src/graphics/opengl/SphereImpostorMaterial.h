// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
#ifndef _GL2_SPHEREIMPOSTORMATERIAL_H
#define _GL2_SPHEREIMPOSTORMATERIAL_H
/*
 * Billboard sphere impostor
 */
#include "libs.h"
#include "MaterialGL.h"
#include "Program.h"
namespace Graphics {

	namespace OGL {

		class SphereImpostorMaterial : public Material {
		public:
			Program *CreateProgram(const MaterialDescriptor &) override {
				return new Program("billboard_sphereimpostor", "");
			}

			virtual void Apply() override {
				OGL::Material::Apply();
				m_program->sceneAmbient.Set(m_renderer->GetAmbientColor());

				//Light uniform parameters
				for( Uint32 i=0 ; i<m_renderer->GetNumLights() ; i++ ) {
					const Light& Light = m_renderer->GetLight(i);
					m_program->lights[i].diffuse.Set( Light.GetDiffuse() );
					m_program->lights[i].specular.Set( Light.GetSpecular() );
					const vector3f& pos = Light.GetPosition();
					m_program->lights[i].position.Set( pos.x, pos.y, pos.z, (Light.GetType() == Light::LIGHT_DIRECTIONAL ? 0.f : 1.f));
				}
			}
		};
	}
}
#endif
