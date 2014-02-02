// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL2_SHIELDMATERIAL_H
#define _GL2_SHIELDMATERIAL_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 *
 */
#include "GL2Material.h"
#include "Program.h"

namespace Graphics {

	namespace GL2 {
		static const Sint32 MAX_SHIELD_HITS = 5; // Also defined in Ship.h

		class ShieldProgram : public Program {
		public:
			ShieldProgram(const MaterialDescriptor &, int lights=0);
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
