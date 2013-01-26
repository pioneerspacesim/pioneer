// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Model.h"
#include "CollisionVisitor.h"
#include "graphics/Renderer.h"

namespace SceneGraph {

class LabelUpdateVisitor : public NodeVisitor {
public:
	virtual void ApplyLabel(Label3D &l) {
		l.SetText(label);
	}

	std::string label;
};

Model::Model(Graphics::Renderer *r, const std::string &name)
: ModelBase()
, m_boundingRadius(10.f)
, m_renderer(r)
, m_name(name)
{
	m_root.Reset(new Group(m_renderer));
	m_root->SetName(name);
}

Model::Model(const Model &model)
: ModelBase()
, m_boundingRadius(model.m_boundingRadius)
, m_renderer(model.m_renderer)
, m_name(model.m_name)
, m_materials(model.m_materials)
, m_patterns(model.m_patterns)
, m_collMesh(model.m_collMesh) //might have to make this per-instance at some point
{
	//selective copying of node structure
	Group *root = dynamic_cast<Group*>(model.m_root->Clone());
	assert(root != 0);
	m_root.Reset(root);

	//materials are shared by meshes
	for (unsigned int i=0; i<MAX_DECAL_MATERIALS; i++)
		m_decalMaterials[i] = model.m_decalMaterials[i];

	//create unique color texture, if used
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

void Model::Render(const matrix4x4f &trans, LmrObjParams *params)
{
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

RefCountedPtr<CollMesh> Model::CreateCollisionMesh(const LmrObjParams *p)
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

	//XXX don't set this yet, do it in render
	for (MaterialContainer::const_iterator it = m_materials.begin();
		it != m_materials.end();
		++it)
	{
		//Set pattern only on a material that supports it
		//XXX hacky using the descriptor
		if ((*it).second->GetDescriptor().usePatterns) {
			(*it).second->texture3 = m_patterns.at(index).texture.Get();
			m_colorMap.SetSmooth(m_patterns.at(index).smoothColor);
		}
	}
}

void Model::SetColors(const std::vector<Color4ub> &colors)
{
	assert(colors.size() == 3); //primary, seconday, trim
	m_colorMap.Generate(GetRenderer(), colors.at(0), colors.at(1), colors.at(2));

	//XXX don't set this yet, do it in render instead
	for (MaterialContainer::const_iterator it = m_materials.begin();
		it != m_materials.end();
		++it)
	{
		//Set colortexture only on a material that uses patterns
		//XXX hacky using the descriptor
		if ((*it).second->GetDescriptor().usePatterns) {
			(*it).second->texture4 = m_colorMap.GetTexture();
		}
	}
}

void Model::SetDecalTexture(Graphics::Texture *t, unsigned int index)
{
	index = std::min(index, MAX_DECAL_MATERIALS-1);
	if (m_decalMaterials[index].Valid())
		m_decalMaterials[index]->texture0 = t;
}

void Model::SetLabel(const std::string &text)
{
	LabelUpdateVisitor vis;
	vis.label = text;
	m_root->Accept(vis);
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

Animation *Model::FindAnimation(const std::string &name)
{
	for (AnimationContainer::iterator anim = m_animations.begin(); anim != m_animations.end(); ++anim) {
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

}
