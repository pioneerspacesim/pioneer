// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_SHIELDMATERIAL_H
#define _OGL_SHIELDMATERIAL_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 *
 */
#include "MaterialGL.h"
#include "Program.h"

namespace Graphics {

	namespace OGL {
		static const Sint32 MAX_SHIELD_HITS = 5; // Also defined in Ship.h

		class ShieldProgram : public Program {
		public:
			ShieldProgram(const MaterialDescriptor &);
			Uniform shieldStrength;
			Uniform shieldCooldown;
			Uniform hitPos[MAX_SHIELD_HITS];
			Uniform radii[MAX_SHIELD_HITS];
			Uniform numHits;
		protected:
			virtual void InitUniforms();
		};

		class ShieldMaterial : public Material { //unlit
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &);
			virtual void Apply();
		};
	}
}

#endif
