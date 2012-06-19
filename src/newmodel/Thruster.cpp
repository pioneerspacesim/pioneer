#include "Thruster.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/Material.h"
#include "graphics/TextureBuilder.h"

namespace Newmodel {

static const std::string thrusterTextureFilename("textures/thruster.png");
static const std::string thrusterGlowTextureFilename("textures/halo.png");
static Color color(0.7f, 0.6f, 1.f, 1.f);

Thruster::Thruster(Graphics::Renderer *r)
: Node(NODE_TRANSPARENT)
{
	m_tVerts.Reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0));

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
		m_tVerts->Add(one, topLeft);
		m_tVerts->Add(two, topRight);
		m_tVerts->Add(three, botRight);

		m_tVerts->Add(three, botRight);
		m_tVerts->Add(four, botLeft);
		m_tVerts->Add(one, topLeft);

		one.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		two.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		three.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		four.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
	}

	//set up materials
	m_tMat.Reset(new Graphics::Material());
	m_tMat->texture0 = Graphics::TextureBuilder::Billboard(thrusterTextureFilename).GetOrCreateTexture(r, "model");
	m_tMat->unlit = true;
	m_tMat->twoSided = true;
	m_tMat->diffuse = color;
}

void Thruster::Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd)
{
	vector3f linthrust(rd->linthrust);
	const float power = -dir.Dot(linthrust);
	if (power < 0.001f) return;

	//TODO: a horrible mess of operations
	r->SetBlendMode(Graphics::BLEND_ADDITIVE);
	r->SetDepthWrite(false);
	r->SetTransform(trans);

	m_tMat->diffuse.a = power;
	/*vector3f cdir(0.f, 0.f, -1.f);
	vector3f vdir(-trans[2], -trans[6], -trans[10]);
	m_tMat->diffuse.a = 1.f - Clamp(vdir.Dot(cdir), 0.f, 1.f);*/
	r->DrawTriangles(m_tVerts.Get(), m_tMat.Get());
	r->SetBlendMode(Graphics::BLEND_SOLID);
}

}