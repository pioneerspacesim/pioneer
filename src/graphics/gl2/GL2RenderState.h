// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL2_RENDERSTATE_H
#define _GL2_RENDERSTATE_H
#include "graphics/RenderState.h"

namespace Graphics {
namespace GL2 {

class RenderState : public Graphics::RenderState {
public:
	RenderState(const RenderStateDesc&);
	void Apply();
};

}
}
#endif
