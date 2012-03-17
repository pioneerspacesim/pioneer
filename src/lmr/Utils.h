#ifndef _LMRUTILS_H
#define _LMRUTILS_H

#include "graphics/Shader.h"

// XXX small shared classes that are used in a few places. probably need to be
// rethought

namespace LMR {

// XXX replace with Graphics::VertexArray
struct Vertex {
	Vertex() : v(0.0), n(0.0), tex_u(0.0), tex_v(0.0) {}		// zero this shit to stop denormal-copying on resize
	Vertex(const vector3f &v_, const vector3f &n_, const GLfloat tex_u_, const GLfloat tex_v_): v(v_), n(n_), tex_u(tex_u_), tex_v(tex_v_) {}
	vector3f v, n;
	GLfloat tex_u, tex_v;
};

struct RenderState {
	/* For the root model this will be identity matrix.
	 * For sub-models called with call_model() then this will be the
	 * transform from sub-model coords to root-model coords.
	 * It is needed by the RenderThruster stuff so we know the centre of
	 * the root model and orientation when rendering thrusters on
	 * sub-models */
	matrix4x4f subTransform;
	// combination of model scale, call_model scale, and all parent scalings
	float combinedScale;
};

struct LmrUnknownMaterial {};

}

#endif
