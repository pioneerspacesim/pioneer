// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/opengl/RenderStateGL.h"
#include "OpenGLLibs.h"

using namespace gl3x;

namespace Graphics
{
namespace OGL
{

RenderState::RenderState(const RenderStateDesc &d)
	: Graphics::RenderState(d)
{
}

void RenderState::Apply()
{
	switch (m_desc.blendMode) {
	case BLEND_SOLID:
		gl::Disable(gl::BLEND);
		gl::BlendFunc(gl::ONE, gl::ZERO);
		break;
	case BLEND_ADDITIVE:
		gl::Enable(gl::BLEND);
		gl::BlendFunc(gl::ONE, gl::ONE);
		break;
	case BLEND_ALPHA:
		gl::Enable(gl::BLEND);
		gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_ALPHA_ONE:
		gl::Enable(gl::BLEND);
		gl::BlendFunc(gl::SRC_ALPHA, gl::ONE);
		break;
	case BLEND_ALPHA_PREMULT:
		gl::Enable(gl::BLEND);
		gl::BlendFunc(gl::ONE, gl::ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_SET_ALPHA:
		gl::Enable(gl::BLEND);
		gl::BlendFuncSeparate(gl::ZERO, gl::ONE, gl::SRC_COLOR, gl::ZERO);
		break;
	case BLEND_DEST_ALPHA:
		gl::Enable(gl::BLEND);
		gl::BlendFunc(gl::DST_ALPHA, gl::ONE_MINUS_DST_ALPHA);
	default:
		break;
	}

	if (m_desc.cullMode == CULL_BACK) {
		gl::Enable(gl::CULL_FACE);
		gl::CullFace(gl::BACK);
	} else if (m_desc.cullMode == CULL_FRONT) {
		gl::Enable(gl::CULL_FACE);
		gl::CullFace(gl::FRONT);
	} else {
		gl::Disable(gl::CULL_FACE);
	}


	if (m_desc.depthTest)
		gl::Enable(gl::DEPTH_TEST);
	else
		gl::Disable(gl::DEPTH_TEST);

	if (m_desc.depthWrite)
		gl::DepthMask(gl::TRUE_);
	else
		gl::DepthMask(gl::FALSE_);
}

}
}
