#ifndef _GL2_MULTIMATERIAL_H
#define _GL2_MULTIMATERIAL_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 */
#include "Material.h"
#include "Program.h"

namespace Graphics {
	class MaterialDescriptor;

	namespace GL2 {
		class MultiProgram : public Program {
		public:
			MultiProgram(const MaterialDescriptor &);
		};

		class MultiMaterial : public Material {
		public:
			Program *CreateProgram(const MaterialDescriptor &);
			virtual void Apply();
			virtual void Unapply();
		};
	}
}

#endif
