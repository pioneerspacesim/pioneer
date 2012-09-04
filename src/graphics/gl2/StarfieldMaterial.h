#ifndef _GL2_STARFIELD_MATERIAL_H
#define _GL2_STARFIELD_MATERIAL_H
/*
 * Starfield material.
 * This does nothing very special except toggle POINT_SIZE
 * The Program requires setting intensity using the generic emission parameter
 */
#include "libs.h"
#include "GL2Material.h"
#include "Program.h"

namespace Graphics {
	namespace GL2 {
		class StarfieldProgram : public Program {
			public:
			StarfieldProgram(const std::string &filename, const std::string &defines);
 
			Uniform twinkling;
			Uniform brightness;
			Uniform time;
			Uniform effect;
			Uniform starScaling;

			protected:
				virtual void InitUniforms();
		};


		class StarfieldMaterial : public Material {
		public:
			Program *CreateProgram(const MaterialDescriptor &) {
				return new StarfieldProgram("starfield", "");
			}

			virtual void Apply() {
				glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
				m_program->Use();
				SetStarfieldUniforms();
			}

			virtual void Unapply() {
				glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
				m_program->Unuse();
			}
			
		protected:
			void SetStarfieldUniforms();

		};
	}
}

#endif
