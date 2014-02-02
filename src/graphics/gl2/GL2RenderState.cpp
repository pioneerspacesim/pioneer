// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/gl2/GL2RenderState.h"

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
		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);
		break;
	case BLEND_ADDITIVE:
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		break;
	case BLEND_ALPHA:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_ALPHA_ONE:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case BLEND_ALPHA_PREMULT:
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_SET_ALPHA:
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ZERO);
		break;
	case BLEND_DEST_ALPHA:
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	default:
		break;
	}

	if (m_desc.cullMode == CULL_BACK) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	} else if (m_desc.cullMode == CULL_FRONT) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	} else {
		glDisable(GL_CULL_FACE);
	}


	if (m_desc.depthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if (m_desc.depthWrite)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);
}

}
}
