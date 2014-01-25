// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Easing.h"
#include "Thruster.h"
#include "NodeVisitor.h"
#include "BaseLoader.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/Material.h"
#include "graphics/TextureBuilder.h"

namespace SceneGraph {

static const std::string thrusterTextureFilename("textures/thruster.png");
static const std::string thrusterGlowTextureFilename("textures/halo.png");
static Color baseColor(178, 153, 255, 255);

Thruster::Thruster(Graphics::Renderer *r, bool _linear, const vector3f &_pos, const vector3f &_dir)
: Node(r, NODE_TRANSPARENT)
, linearOnly(_linear)
, dir(_dir)
, pos(_pos)
{
	m_tVerts.reset(CreateThrusterGeometry());
	m_glowVerts.reset(CreateGlowGeometry());

	//set up materials
	Graphics::MaterialDescriptor desc;
	desc.textures = 1;
	desc.twoSided = true;

	m_tMat.Reset(r->CreateMaterial(desc));
	m_tMat->texture0 = Graphics::TextureBuilder::Billboard(thrusterTextureFilename).GetOrCreateTexture(r, "billboard");
	m_tMat->diffuse = baseColor;

	m_glowMat.Reset(r->CreateMaterial(desc));
	m_glowMat->texture0 = Graphics::TextureBuilder::Billboard(thrusterGlowTextureFilename).GetOrCreateTexture(r, "billboard");
	m_glowMat->diffuse = baseColor;
}

Thruster::Thruster(const Thruster &thruster, NodeCopyCache *cache)
: Node(thruster, cache)
, m_tMat(thruster.m_tMat)
, linearOnly(thruster.linearOnly)
, dir(thruster.dir)
, pos(thruster.pos)
{
	m_tVerts.reset(CreateThrusterGeometry());
	m_glowVerts.reset(CreateGlowGeometry());
}

Node* Thruster::Clone(NodeCopyCache *cache)
{
	return this; //thrusters are shared
}

void Thruster::Accept(NodeVisitor &nv)
{
	nv.ApplyThruster(*this);
}

void Thruster::Render(const matrix4x4f &trans, const RenderData *rd)
{
	float power = 0.f;
	power = -dir.Dot(vector3f(rd->linthrust));

	if (!linearOnly) {
		// pitch X
		// yaw   Y
		// roll  Z
		//model center is at 0,0,0, no need for invSubModelMat stuff
		const vector3f at = vector3f(rd->angthrust);
		const vector3f angdir = pos.Cross(dir);

		const float xp = angdir.x * at.x;
		const float yp = angdir.y * at.y;
		const float zp = angdir.z * at.z;

		if (xp+yp+zp > 0) {
			if (xp > yp && xp > zp && fabs(at.x) > power) power = fabs(at.x);
			else if (yp > xp && yp > zp && fabs(at.y) > power) power = fabs(at.y);
			else if (zp > xp && zp > yp && fabs(at.z) > power) power = fabs(at.z);
		}
	}
	if (power < 0.001f) return;

	Graphics::Renderer *r = GetRenderer();
	r->SetTransform(trans);

	r->SetBlendMode(Graphics::BLEND_ALPHA_ONE);
	r->SetDepthWrite(false);

	m_tMat->diffuse = m_glowMat->diffuse = baseColor * power;

	//directional fade
	vector3f cdir = vector3f(trans * -dir).Normalized();
	vector3f vdir = vector3f(trans[2], trans[6], -trans[10]).Normalized();
	// XXX check this for transition to new colors.
	m_glowMat->diffuse.a = Easing::Circ::EaseIn(Clamp(vdir.Dot(cdir), 0.f, 1.f), 0.f, 1.f, 1.f) * 255;
	m_tMat->diffuse.a = 255 - m_glowMat->diffuse.a;

	r->DrawTriangles(m_tVerts.get(), m_tMat.Get());
	r->DrawTriangles(m_glowVerts.get(), m_glowMat.Get());

	r->SetBlendMode(Graphics::BLEND_SOLID);
	r->SetDepthWrite(true);
}

void Thruster::Save(NodeDatabase &db)
{
	Node::Save(db);
	db.wr->Bool(linearOnly);
	db.wr->Vector3f(dir);
	db.wr->Vector3f(pos);
}

Thruster *Thruster::Load(NodeDatabase &db)
{
	const bool linear = db.rd->Bool();
	const vector3f dir = db.rd->Vector3f();
	const vector3f pos = db.rd->Vector3f();
	Thruster *t = new Thruster(db.loader->GetRenderer(), linear, pos, dir);
	return t;
}

Graphics::VertexArray *Thruster::CreateThrusterGeometry()
{
	Graphics::VertexArray *verts =
		new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

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
		verts->Add(one, topLeft);
		verts->Add(two, topRight);
		verts->Add(three, botRight);

		verts->Add(three, botRight);
		verts->Add(four, botLeft);
		verts->Add(one, topLeft);

		one.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		two.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		three.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		four.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
	}

	return verts;
}

Graphics::VertexArray *Thruster::CreateGlowGeometry()
{
	Graphics::VertexArray *verts =
		new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

	//create glow billboard for linear thrusters
	const float w = 0.2;

	vector3f one(-w, -w, 0.f); //top left
	vector3f two(-w,  w, 0.f); //top right
	vector3f three(w, w, 0.f); //bottom right
	vector3f four(w, -w, 0.f); //bottom left

	//uv coords
	const vector2f topLeft(0.f, 1.f);
	const vector2f topRight(1.f, 1.f);
	const vector2f botLeft(0.f, 0.f);
	const vector2f botRight(1.f, 0.f);

	for (int i = 0; i < 5; i++) {
		verts->Add(one, topLeft);
		verts->Add(two, topRight);
		verts->Add(three, botRight);

		verts->Add(three, botRight);
		verts->Add(four, botLeft);
		verts->Add(one, topLeft);

		one.z += .1f;
		two.z = three.z = four.z = one.z;
	}

	return verts;
}

}
