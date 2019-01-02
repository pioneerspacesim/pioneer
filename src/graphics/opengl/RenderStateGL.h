// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_RENDERSTATE_H
#define _OGL_RENDERSTATE_H
#include "graphics/RenderState.h"

namespace Graphics {
	namespace OGL {

		class RenderState : public Graphics::RenderState {
		public:
			RenderState(const RenderStateDesc &);
			void Apply();
		};

	} // namespace OGL
} // namespace Graphics
#endif
