// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GRAPHICS_RENDERSTATE_H
#define _GRAPHICS_RENDERSTATE_H
#include "Renderer.h"

namespace Graphics
{

struct RenderStateDesc {
	RenderStateDesc()
		: blendMode(BLEND_SOLID)
		, cullMode(CULL_BACK)
		, depthTest(true)
		, depthWrite(true)
	{
	}

	BlendMode blendMode;
	FaceCullMode cullMode;
	bool depthTest;
	bool depthWrite;
};

class RenderState
{
public:
	virtual ~RenderState() { }

	const RenderStateDesc &GetDesc() const { return m_desc; }

protected:
	RenderState(const RenderStateDesc &d) : m_desc(d) { }
	RenderStateDesc m_desc;
};

}

#endif
