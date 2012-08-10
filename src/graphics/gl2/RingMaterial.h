#ifndef _GL2_RINGMATERIAL_H
#define _GL2_RINGMATERIAL_H
/*
 * Planet ring material
 */
#include "libs.h"
#include "GL2Material.h"
#include "Program.h"
namespace Graphics {

	namespace GL2 {

		class RingMaterial : public Material {
		public:
			Program *CreateProgram(const MaterialDescriptor &);
			void Apply();
			void Unapply();
		};
	}
}
#endif
