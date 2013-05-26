// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Model.h"
#include "CollisionVisitor.h"
#include "NodeCopyCache.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"

namespace SceneGraph {

class LabelUpdateVisitor : public NodeVisitor {
public:
	virtual void ApplyLabel(Label3D &l) {
		l.SetText(label);
	}

	std::string label;
};

Model::Model(Graphics::Renderer *r, const std::string &name)
: m_boundingRadius(10.f)
, m_renderer(r)
, m_name(name)
, m_curPattern(0)
{
	m_root.Reset(new Group(m_renderer));
	m_root->SetName(name);
	ClearDecals();
}

Model::Model(const Model &model)
: m_boundingRadius(model.m_boundingRadius)
, m_materials(model.m_materials)
, m_patterns(model.m_patterns)
, m_collMesh(model.m_collMesh) //might have to make this per-instance at some point
, m_renderer(model.m_renderer)
, m_name(model.m_name)
, m_curPattern(model.m_curPattern)
{
	//selective copying of node structure
	NodeCopyCache cache;
	m_root.Reset(dynamic_cast<Group*>(model.m_root->Clone(&cache)));

	//materials are shared by meshes
	for (unsigned int i=0; i<MAX_DECAL_MATERIALS; i++)
		m_decalMaterials[i] = model.m_decalMaterials[i];
	ClearDecals();

	//create unique color texture, if used
	//patterns are shared
	if (SupportsPatterns()) {
		std::vector<Color4ub> colors;
		colors.push_back(Color4ub::RED);
		colors.push_back(Color4ub::GREEN);
		colors.push_back(Color4ub::BLUE);
		SetColors(colors);
		SetPattern(0);
	}

	//animations need to be copied and retargeted
	for (AnimationContainer::const_iterator it = model.m_animations.begin(); it != model.m_animations.end(); ++it) {
		const Animation *anim = *it;
		m_animations.push_back(new Animation(*anim));
		m_animations.back()->UpdateChannelTargets(m_root.Get());
	}

	//m_tags needs to be updated
	for (TagContainer::const_iterator it = model.m_tags.begin(); it != model.m_tags.end(); ++it) {
		MatrixTransform *t = dynamic_cast<MatrixTransform*>(m_root->FindNode((*it)->GetName()));
		assert(t != 0);
		m_tags.push_back(t);
	}
}

Model::~Model()
{
	while(!m_animations.empty()) delete m_animations.back(), m_animations.pop_back();
}

Model *Model::MakeInstance() const
{
	Model *m = new Model(*this);
	return m;
}

void Model::Render(const matrix4x4f &trans, RenderData *rd)
{
	//update color parameters (materials are shared by model instances)
	if (m_curPattern) {
		for (MaterialContainer::const_iterator it = m_materials.begin(); it != m_materials.end(); ++it) {
			if ((*it).second->GetDescriptor().usePatterns) {
				(*it).second->texture4 = m_colorMap.GetTexture();
				(*it).second->texture3 = m_curPattern;
			}
		}
	}

	//update decals (materials and geometries are shared)
	for (unsigned int i=0; i < MAX_DECAL_MATERIALS; i++)
		if (m_decalMaterials[i].Valid())
			m_decalMaterials[i]->texture0 = m_curDecals[i];

	//Override renderdata if this model is called from ModelNode
	RenderData *params = (rd != 0) ? rd : &m_renderData;

	m_renderer->SetBlendMode(Graphics::BLEND_SOLID);
	m_renderer->SetTransform(trans);
	//using the entire model bounding radius for all nodes at the moment.
	//BR could also be a property of Node.
	params->boundingRadius = GetDrawClipRadius();

	//render in two passes, if this is the top-level model
	if (params->nodemask & MASK_IGNORE) {
		m_root->Render(trans, params);
	} else {
		params->nodemask = NODE_SOLID;
		m_root->Render(trans, params);
		params->nodemask = NODE_TRANSPARENT;
		m_root->Render(trans, params);
	}
}

RefCountedPtr<CollMesh> Model::CreateCollisionMesh()
{
	CollisionVisitor cv;
	m_root->Accept(cv);
	m_collMesh = cv.CreateCollisionMesh();
	m_boundingRadius = cv.GetBoundingRadius();
	return m_collMesh;
}

RefCountedPtr<Graphics::Material> Model::GetMaterialByName(const std::string &name) const
{
	for (MaterialContainer::const_iterator it = m_materials.begin();
		it != m_materials.end();
		++it)
	{
		if ((*it).first == name) return (*it).second;
	}
	return RefCountedPtr<Graphics::Material>(); //return invalid
}

RefCountedPtr<Graphics::Material> Model::GetMaterialByIndex(const int i) const
{
	return m_materials.at(Clamp(i, 0, int(m_materials.size())-1)).second;
}

MatrixTransform * const Model::GetTagByIndex(const unsigned int i) const
{
	if (m_tags.empty() || i > m_tags.size()-1) return 0;
	return m_tags.at(i);
}

MatrixTransform * const Model::FindTagByName(const std::string &name) const
{
	for (TagContainer::const_iterator it = m_tags.begin();
		it != m_tags.end();
		++it)
	{
		assert(!(*it)->GetName().empty()); //tags must have a name
		if ((*it)->GetName() == name) return (*it);
	}
	return 0;
}

void Model::FindTagsByStartOfName(const std::string &name, TVecMT &outNameMTs) const
{
	for (TagContainer::const_iterator it = m_tags.begin();
		it != m_tags.end();
		++it)
	{
		assert(!(*it)->GetName().empty()); //tags must have a name
		if (starts_with((*it)->GetName(), name)) {
			outNameMTs.push_back((*it));
		}
	}
	return;
}

void Model::AddTag(const std::string &name, MatrixTransform *node)
{
	if (FindTagByName(name)) return;
	node->SetName(name);
	m_root->AddChild(node);
	m_tags.push_back(node);
}

void Model::SetPattern(unsigned int index)
{
	if (m_patterns.empty() || index > m_patterns.size() - 1) return;
	const Pattern &pat = m_patterns.at(index);
	m_colorMap.SetSmooth(pat.smoothColor);
	m_curPattern = pat.texture.Get();
}

void Model::SetColors(const std::vector<Color4ub> &colors)
{
	assert(colors.size() == 3); //primary, seconday, trim
	m_colorMap.Generate(GetRenderer(), colors.at(0), colors.at(1), colors.at(2));
}

void Model::SetDecalTexture(Graphics::Texture *t, unsigned int index)
{
	index = std::min(index, MAX_DECAL_MATERIALS-1);
	if (m_decalMaterials[index].Valid())
		m_curDecals[index] = t;
}

void Model::SetLabel(const std::string &text)
{
	LabelUpdateVisitor vis;
	vis.label = text;
	m_root->Accept(vis);
}

void Model::ClearDecals()
{
	Graphics::Texture *t = Graphics::TextureBuilder::GetTransparentTexture(m_renderer);
	for (unsigned int i=0; i<MAX_DECAL_MATERIALS; i++)
		m_curDecals[i] = t;
}

void Model::ClearDecal(unsigned int index)
{
	index = std::min(index, MAX_DECAL_MATERIALS-1);
	if (m_decalMaterials[index].Valid())
		m_curDecals[index] = Graphics::TextureBuilder::GetTransparentTexture(m_renderer);
}

bool Model::SupportsDecals()
{
	for (unsigned int i=0; i<MAX_DECAL_MATERIALS; i++)
		if (m_decalMaterials[i].Valid()) return true;

	return false;
}

bool Model::SupportsPatterns()
{
	for (MaterialContainer::const_iterator it = m_materials.begin();
		it != m_materials.end();
		++it)
	{
		//Set pattern only on a material that supports it
		if ((*it).second->GetDescriptor().usePatterns)
			return true;
	}

	return false;
}

Animation *Model::FindAnimation(const std::string &name) const
{
	for (AnimationContainer::const_iterator anim = m_animations.begin(); anim != m_animations.end(); ++anim) {
		if ((*anim)->GetName() == name) return (*anim);
	}
	return 0;
}

void Model::UpdateAnimations()
{
	// XXX WIP. Assuming animations are controlled manually by SetProgress.
	for (AnimationContainer::iterator anim = m_animations.begin(); anim != m_animations.end(); ++anim)
		(*anim)->Interpolate();
}

void Model::SetThrust(const vector3f &lin, const vector3f &ang)
{
	m_renderData.linthrust[0] = lin.x;
	m_renderData.linthrust[1] = lin.y;
	m_renderData.linthrust[2] = lin.z;

	m_renderData.angthrust[0] = ang.x;
	m_renderData.angthrust[1] = ang.y;
	m_renderData.angthrust[2] = ang.z;
}

class SaveVisitor : public NodeVisitor {
public:
	SaveVisitor(Serializer::Writer *wr_): wr(wr_) {}

	void ApplyMatrixTransform(MatrixTransform &node) {
		const matrix4x4f &m = node.GetTransform();
		for (int i = 0; i < 16; i++)
			wr->Float(m[i]);
	}

private:
	Serializer::Writer *wr;
};

void Model::Save(Serializer::Writer &wr) const
{
	SaveVisitor sv(&wr);
	m_root->Accept(sv);

	for (AnimationContainer::const_iterator i = m_animations.begin(); i != m_animations.end(); ++i)
		wr.Double((*i)->GetProgress());
}

class LoadVisitor : public NodeVisitor {
public:
	LoadVisitor(Serializer::Reader *rd_): rd(rd_) {}

	void ApplyMatrixTransform(MatrixTransform &node) {
		matrix4x4f m;
		for (int i = 0; i < 16; i++)
			m[i] = rd->Float();
		node.SetTransform(m);
	}

private:
	Serializer::Reader *rd;
};

void Model::Load(Serializer::Reader &rd)
{
	LoadVisitor lv(&rd);
	m_root->Accept(lv);

	for (AnimationContainer::const_iterator i = m_animations.begin(); i != m_animations.end(); ++i)
		(*i)->SetProgress(rd.Double());
	UpdateAnimations();
}

}
