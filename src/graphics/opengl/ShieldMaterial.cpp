// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShieldMaterial.h"
#include "RendererGL.h"
#include "Shields.h"
#include "StringF.h"
#include "TextureGL.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include <sstream>

namespace Graphics {
	namespace OGL {

		ShieldProgram::ShieldProgram(const MaterialDescriptor &desc)
		{
			//build some defines
			std::stringstream ss;
			ss << stringf("#define MAX_SHIELD_HITS %0{d}\n", MAX_SHIELD_HITS);

			m_name = "shield";
			m_defines = ss.str();

			LoadShaders(m_name, m_defines);
			InitUniforms();
			hitInfoBlock.InitBlock("ShieldData", m_program, 2);
		}

		Program *ShieldMaterial::CreateProgram(const MaterialDescriptor &desc)
		{
			return new ShieldProgram(desc);
		}

		struct ShieldData {
			struct ShieldHitInfo {
				vector3f hitPos;
				float radii;
			} hits[MAX_SHIELD_HITS];

			alignas(16)
			float shieldStrength;
			float shieldCooldown;
			int numHits;
		};

		void ShieldMaterial::Apply()
		{
			OGL::Material::Apply();

			ShieldProgram *p = static_cast<ShieldProgram *>(m_program);

			auto buffer = m_renderer->GetDrawUniformBuffer(sizeof(ShieldData));
			{
				auto dataBlock = buffer->Allocate<ShieldData>(p->hitInfoBlock.Location());
				if (this->specialParameter0) {
					const ShieldRenderParameters srp = *static_cast<ShieldRenderParameters *>(this->specialParameter0);
					dataBlock->shieldStrength = srp.strength;
					dataBlock->shieldCooldown = srp.coolDown;
					dataBlock->numHits = srp.numHits;
					for (Sint32 i = 0; i < srp.numHits && i < MAX_SHIELD_HITS; i++) {
						dataBlock->hits[i].hitPos = srp.hitPos[i];
						dataBlock->hits[i].radii = srp.radii[i];
					}
				} else {
					dataBlock->shieldStrength = 0.f;
					dataBlock->shieldCooldown = 0.f;
					dataBlock->numHits = 0.f;

					for (Sint32 i = 0; i < MAX_SHIELD_HITS; i++) {
						dataBlock->hits[i].hitPos = vector3f();
						dataBlock->hits[i].radii = 0.f;
					}
				}
			}


		}

	} // namespace OGL
} // namespace Graphics
