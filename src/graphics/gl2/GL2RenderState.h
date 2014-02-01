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
