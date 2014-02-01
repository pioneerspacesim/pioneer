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
