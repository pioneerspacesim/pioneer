// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _OGL_BILLBOARDMATERIAL_H
#define _OGL_BILLBOARDMATERIAL_H
/*
 * point sprite (aka billboard) material
 */

#include "MaterialGL.h"
#include "Program.h"
namespace Graphics {

	namespace OGL {

		class BillboardProgram : public Program {
		public:
			BillboardProgram(const MaterialDescriptor &);
			Uniform coordDownScale;

		protected:
			virtual void InitUniforms() override;
		};

		class BillboardMaterial : public Material {
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &) override;
			virtual void Apply() override;
			virtual void Unapply() override;
		};
	} // namespace OGL
} // namespace Graphics
#endif
