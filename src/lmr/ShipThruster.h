#ifndef _LMRSHIPTHRUSTER_H
#define _LMRSHIPTHRUSTER_H

#include "graphics/Material.h"
#include "Color.h"

namespace Graphics {
	class VertexArray;
	class Renderer;
}
class LmrObjParams;

namespace LMR {

class RenderState;

namespace ShipThruster {
	//vertices for thruster flare & glow
	extern Graphics::VertexArray *tVerts;
	extern Graphics::VertexArray *gVerts;
	extern Graphics::Material tMat;
	extern Graphics::Material glowMat;
	//cool purple-ish
	extern Color color;

	extern void Init(Graphics::Renderer *renderer);
	extern void Uninit();

	struct Thruster
	{
		Thruster() : m_pos(0.0), m_dir(0.0), m_power(0) {}	// zero this shit to stop denormal-copying on resize
		// cannot be used as an angular thruster
		bool m_linear_only;
		vector3f m_pos;
		vector3f m_dir;
		float m_power;
		void Render(Graphics::Renderer *r, const RenderState *rstate, const LmrObjParams *params);
	};
}

}

#endif
