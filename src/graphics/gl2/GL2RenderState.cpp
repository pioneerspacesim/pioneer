// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "OpenGLLibs.h"
#include "GL2RenderState.h"

using namespace gl21;

namespace Graphics
{
namespace GL2
{

RenderState::RenderState(const RenderStateDesc &d)
	: Graphics::RenderState(d)
{
}

void RenderState::Apply()
{
	switch (m_desc.blendMode) {
	case BLEND_SOLID:
		gl::glDisable(gl::GL_BLEND);
		gl::glBlendFunc(gl::GL_ONE, gl::GL_ZERO);
		break;
	case BLEND_ADDITIVE:
		gl::glEnable(gl::GL_BLEND);
		gl::glBlendFunc(gl::GL_ONE, gl::GL_ONE);
		break;
	case BLEND_ALPHA:
		gl::glEnable(gl::GL_BLEND);
		gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_ALPHA_ONE:
		gl::glEnable(gl::GL_BLEND);
		gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE);
		break;
	case BLEND_ALPHA_PREMULT:
		gl::glEnable(gl::GL_BLEND);
		gl::glBlendFunc(gl::GL_ONE, gl::GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_SET_ALPHA:
		gl::glEnable(gl::GL_BLEND);
		gl::glBlendFuncSeparate(gl::GL_ZERO, gl::GL_ONE, gl::GL_SRC_COLOR, gl::GL_ZERO);
		break;
	case BLEND_DEST_ALPHA:
		gl::glEnable(gl::GL_BLEND);
		gl::glBlendFunc(gl::GL_DST_ALPHA, gl::GL_ONE_MINUS_DST_ALPHA);
	default:
		break;
	}

	if (m_desc.cullMode == CULL_BACK) {
		gl::glEnable(gl::GL_CULL_FACE);
		gl::glCullFace(gl::GL_BACK);
	} else if (m_desc.cullMode == CULL_FRONT) {
		gl::glEnable(gl::GL_CULL_FACE);
		gl::glCullFace(gl::GL_FRONT);
	} else {
		gl::glDisable(gl::GL_CULL_FACE);
	}


	if (m_desc.depthTest)
		gl::glEnable(gl::GL_DEPTH_TEST);
	else
		gl::glDisable(gl::GL_DEPTH_TEST);

	if (m_desc.depthWrite)
		gl::glDepthMask(gl::GL_TRUE);
	else
		gl::glDepthMask(gl::GL_FALSE);
}

}
}
