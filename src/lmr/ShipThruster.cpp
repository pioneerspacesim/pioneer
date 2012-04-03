#include "ShipThruster.h"
#include "graphics/VertexArray.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"

#include "Utils.h"
#include "LmrModel.h" // XXX for LmrObjParams

namespace LMR {

namespace ShipThruster {

Graphics::VertexArray *tVerts;
Graphics::VertexArray *gVerts;
Graphics::Material tMat;
Graphics::Material glowMat;
Color color(0.7f, 0.6f, 1.f, 1.f);

static const std::string thrusterTextureFilename("textures/thruster.png");
static const std::string thrusterGlowTextureFilename("textures/halo.png");

void Init(Graphics::Renderer *renderer) {
	tVerts = new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	gVerts = new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

	//set up materials
	tMat.texture0 = Graphics::TextureBuilder::Billboard(thrusterTextureFilename).GetOrCreateTexture(renderer);
	tMat.unlit = true;
	tMat.twoSided = true;
	tMat.diffuse = color;
	glowMat.texture0 = Graphics::TextureBuilder::Billboard(thrusterGlowTextureFilename).GetOrCreateTexture(renderer);
	glowMat.unlit = true;
	glowMat.twoSided = true;
	glowMat.diffuse = color;

	//zero at thruster center
	//+x down
	//+y right
	//+z backwards (or thrust direction)
	const float w = 0.5f;

	vector3f one(0.f, -w, 0.f); //top left
	vector3f two(0.f,  w, 0.f); //top right
	vector3f three(0.f,  w, 1.f); //bottom right
	vector3f four(0.f, -w, 1.f); //bottom left

	//uv coords
	const vector2f topLeft(0.f, 1.f);
	const vector2f topRight(1.f, 1.f);
	const vector2f botLeft(0.f, 0.f);
	const vector2f botRight(1.f, 0.f);

	//add four intersecting planes to create a volumetric effect
	for (int i=0; i < 4; i++) {
		tVerts->Add(one, topLeft);
		tVerts->Add(two, topRight);
		tVerts->Add(three, botRight);

		tVerts->Add(three, botRight);
		tVerts->Add(four, botLeft);
		tVerts->Add(one, topLeft);

		one.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		two.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		three.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		four.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
	}

	//create glow billboard for linear thrusters
	const float gw = 1.0f; //0.4

	const vector3f gone(-gw, -gw, 0.f); //top left
	const vector3f gtwo(-gw,  gw, 0.f); //top right
	const vector3f gthree(gw, gw, 0.f); //bottom right
	const vector3f gfour(gw, -gw, 0.f); //bottom left

	gVerts->Add(gone, topLeft);
	gVerts->Add(gtwo, topRight);
	gVerts->Add(gthree, botRight);

	gVerts->Add(gthree, botRight);
	gVerts->Add(gfour, botLeft);
	gVerts->Add(gone, topLeft);
}

void Uninit() {
	delete tVerts;
	delete gVerts;
}

void Thruster::Render(Graphics::Renderer *renderer, const RenderState *rstate, const LmrObjParams *params)
{
	const float scale = 1.0;
	// to find v(0,0,0) position of root model (when putting thrusters on sub-models)
	vector3f compos = vector3f(rstate->subTransform[12], rstate->subTransform[13], rstate->subTransform[14]);
	matrix4x4f invSubModelMat = matrix4x4f::MakeRotMatrix(
				vector3f(rstate->subTransform[0], rstate->subTransform[1], rstate->subTransform[2]),
				vector3f(rstate->subTransform[4], rstate->subTransform[5], rstate->subTransform[6]),
				vector3f(rstate->subTransform[8], rstate->subTransform[9], rstate->subTransform[10]));

	vector3f start, end, dir = m_dir;
	start = m_pos * scale;
	float power = -dir.Dot(invSubModelMat * vector3f(params->linthrust));

	if (!m_linear_only) {
		vector3f angdir, cpos;
		const vector3f at = invSubModelMat * vector3f(params->angthrust);
		cpos = compos + start;
		angdir = cpos.Cross(dir);
		float xp = angdir.x * at.x;
		float yp = angdir.y * at.y;
		float zp = angdir.z * at.z;
		if (xp+yp+zp > 0) {
			if (xp > yp && xp > zp && fabs(at.x) > power) power = fabs(at.x);
			else if (yp > xp && yp > zp && fabs(at.y) > power) power = fabs(at.y);
			else if (zp > xp && zp > yp && fabs(at.z) > power) power = fabs(at.z);
		}
	}

	if (power <= 0.001f) return;
	power *= scale;
	float width = sqrt(power)*m_power*0.6f;
	float len = power*m_power;
	end = dir * len;
	end += start;

	vector3f v1, v2, pos;
	matrix4x4f m2;
	matrix4x4f m = matrix4x4f::Identity();
	v1.x = dir.y; v1.y = dir.z; v1.z = dir.x;
	v2 = v1.Cross(dir).Normalized();
	v1 = v2.Cross(dir);
	m[0] = v1.x; m[4] = v2.x; m[8] = dir.x;
	m[1] = v1.y; m[5] = v2.y; m[9] = dir.y;
	m[2] = v1.z; m[6] = v2.z; m[10] = dir.z;
	m2 = m;

	pos = start;

	m2[12] = pos.x;
	m2[13] = pos.y;
	m2[14] = pos.z;
	
	glPushMatrix ();
	glMultMatrixf (&m2[0]);

	glScalef (width*0.5f, width*0.5f, len*0.666f);

	matrix4x4f mv;
	glGetFloatv(GL_MODELVIEW_MATRIX, &mv[0]);
	vector3f viewdir = vector3f(-mv[2], -mv[6], -mv[10]).Normalized();
	vector3f cdir(0.f, 0.f, -1.f);
	//fade thruster out, when directly facing it
	tMat.diffuse.a = 1.0 - powf(Clamp(viewdir.Dot(cdir), 0.f, 1.f), len*2);

	renderer->DrawTriangles(tVerts, &tMat);
	glPopMatrix ();

	// linear thrusters get a secondary glow billboard
	if (m_linear_only) {
		glowMat.diffuse.a = powf(Clamp(viewdir.Dot(cdir), 0.f, 1.f), len);

		glPushMatrix();
		matrix4x4f rot;
		glGetFloatv(GL_MODELVIEW_MATRIX, &rot[0]);
		rot.ClearToRotOnly();
		rot = rot.InverseOf();
		const float sz = 0.20f*width;
		const vector3f rotv1 = rot * vector3f(sz, sz, 0.0f);
		const vector3f rotv2 = rot * vector3f(sz, -sz, 0.0f);
		const vector3f rotv3 = rot * vector3f(-sz, -sz, 0.0f);
		const vector3f rotv4 = rot * vector3f(-sz, sz, 0.0f);

		//this might seem a bit confusing, but:
		//update glow billboard vertices so they face the camera
		vector3f vert = start+rotv4;

		gVerts->position[0] = vector3f(vert.x, vert.y, vert.z);
		gVerts->position[5] = vector3f(vert.x, vert.y, vert.z);

		vert = start+rotv3;
		gVerts->position[1] = vector3f(vert.x, vert.y, vert.z);

		vert = start+rotv2;
		gVerts->position[2] = vector3f(vert.x, vert.y, vert.z);
		gVerts->position[3] = vector3f(vert.x, vert.y, vert.z);

		vert = start+rotv1;
		gVerts->position[4] = vector3f(vert.x, vert.y, vert.z);

		renderer->DrawTriangles(gVerts, &glowMat);

		glPopMatrix();
	}
}

}

}
