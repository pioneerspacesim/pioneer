// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _DUMMY_RENDERSTATE_H
#define _DUMMY_RENDERSTATE_H

#include "graphics/RenderState.h"

namespace Graphics {
namespace Dummy {

class RenderState : public Graphics::RenderState {
public:
	RenderState(const RenderStateDesc &d) : Graphics::RenderState(d) {}
	void Apply() {}
};

}
}
#endif
