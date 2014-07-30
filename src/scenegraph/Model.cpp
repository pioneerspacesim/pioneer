// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Model.h"
#include "CollisionVisitor.h"
#include "NodeCopyCache.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"
#include "StringF.h"

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
, m_curPatternIndex(0)
, m_curPattern(0)
, m_debugFlags(0)
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
, m_curPatternIndex(model.m_curPatternIndex)
, m_curPattern(model.m_curPattern)
, m_debugFlags(0)
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
		std::vector<Color> colors;
		colors.push_back(Color::RED);
		colors.push_back(Color::GREEN);
		colors.push_back(Color::BLUE);
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

void Model::Render(const matrix4x4f &trans, const RenderData *rd)
{
	//update color parameters (materials are shared by model instances)
	if (m_curPattern) {
		for (MaterialContainer::const_iterator it = m_materials.begin(); it != m_materials.end(); ++it) {
			if ((*it).second->GetDescriptor().usePatterns) {
				(*it).second->texture5 = m_colorMap.GetTexture();
				(*it).second->texture4 = m_curPattern;
			}
		}
	}

	//update decals (materials and geometries are shared)
	for (unsigned int i=0; i < MAX_DECAL_MATERIALS; i++)
		if (m_decalMaterials[i])
			m_decalMaterials[i]->texture0 = m_curDecals[i];

	//Override renderdata if this model is called from ModelNode
	RenderData params = (rd != 0) ? (*rd) : m_renderData;

	m_renderer->SetTransform(trans);
	//using the entire model bounding radius for all nodes at the moment.
	//BR could also be a property of Node.
	params.boundingRadius = GetDrawClipRadius();

	//render in two passes, if this is the top-level model
	if (m_debugFlags & DEBUG_WIREFRAME)
		m_renderer->SetWireFrameMode(true);

	if (params.nodemask & MASK_IGNORE) {
		m_root->Render(trans, &params);
	} else {
		params.nodemask = NODE_SOLID;
		m_root->Render(trans, &params);
		params.nodemask = NODE_TRANSPARENT;
		m_root->Render(trans, &params);
	}

	if (!m_debugFlags)
		return;

	if (m_debugFlags & DEBUG_WIREFRAME)
		m_renderer->SetWireFrameMode(false);

	if (m_debugFlags & DEBUG_BBOX) {
		m_renderer->SetTransform(trans);
		DrawAabb();
	}

	if (m_debugFlags & DEBUG_COLLMESH) {
		m_renderer->SetTransform(trans);
		DrawCollisionMesh();
	}

	if (m_debugFlags & DEBUG_TAGS) {
		m_renderer->SetTransform(trans);
		DrawAxisIndicators(m_tagPoints);
	}

	if (m_debugFlags & DEBUG_DOCKING) {
		m_renderer->SetTransform(trans);
		DrawAxisIndicators(m_dockingPoints);
	}
}

void Model::DrawAabb()
{
	if (!m_collMesh) return;

	Aabb aabb = m_collMesh->GetAabb();

	const vector3f verts[16] = {
		vector3f(aabb.min.x, aabb.min.y, aabb.min.z),
		vector3f(aabb.max.x, aabb.min.y, aabb.min.z),
		vector3f(aabb.max.x, aabb.max.y, aabb.min.z),
		vector3f(aabb.min.x, aabb.max.y, aabb.min.z),
		vector3f(aabb.min.x, aabb.min.y, aabb.min.z),
		vector3f(aabb.min.x, aabb.min.y, aabb.max.z),
		vector3f(aabb.max.x, aabb.min.y, aabb.max.z),
		vector3f(aabb.max.x, aabb.min.y, aabb.min.z),

		vector3f(aabb.max.x, aabb.max.y, aabb.max.z),
		vector3f(aabb.min.x, aabb.max.y, aabb.max.z),
		vector3f(aabb.min.x, aabb.min.y, aabb.max.z),
		vector3f(aabb.max.x, aabb.min.y, aabb.max.z),
		vector3f(aabb.max.x, aabb.max.y, aabb.max.z),
		vector3f(aabb.max.x, aabb.max.y, aabb.min.z),
		vector3f(aabb.min.x, aabb.max.y, aabb.min.z),
		vector3f(aabb.min.x, aabb.max.y, aabb.max.z),
	};

	auto state = m_renderer->CreateRenderState(Graphics::RenderStateDesc());
	m_renderer->DrawLines(8, verts + 0, Color::GREEN, state, Graphics::LINE_STRIP);
	m_renderer->DrawLines(8, verts + 8, Color::GREEN, state, Graphics::LINE_STRIP);
}

// Draw collision mesh as a wireframe overlay
void Model::DrawCollisionMesh()
{
	if (!m_collMesh) return;

	const vector3f *vertices = reinterpret_cast<const vector3f*>(m_collMesh->GetGeomTree()->GetVertices());
	const Uint16 *indices = m_collMesh->GetGeomTree()->GetIndices();
	const unsigned int *triFlags = m_collMesh->GetGeomTree()->GetTriFlags();
	const unsigned int numIndices = m_collMesh->GetGeomTree()->GetNumTris() * 3;

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE, numIndices * 3);
	int trindex = -1;
	for(unsigned int i = 0; i < numIndices; i++) {
		if (i % 3 == 0)
			trindex++;
		const unsigned int flag = triFlags[trindex];
		//show special geomflags in red
		va.Add(vertices[indices[i]], flag > 0 ? Color::RED : Color::WHITE);
	}

	//might want to add some offset
	m_renderer->SetWireFrameMode(true);
	Graphics::RenderStateDesc rsd;
	rsd.cullMode = Graphics::CULL_NONE;
	m_renderer->DrawTriangles(&va, m_renderer->CreateRenderState(rsd), Graphics::vtxColorMaterial);
	m_renderer->SetWireFrameMode(false);
}

void Model::DrawAxisIndicators(std::vector<Graphics::Drawables::Line3D> &lines)
{
	for(auto i = lines.begin(); i != lines.end(); ++i)
		(*i).Draw(m_renderer, m_renderer->CreateRenderState(Graphics::RenderStateDesc()));
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
	node->SetNodeFlags(node->GetNodeFlags() | NODE_TAG);
	m_root->AddChild(node);
	m_tags.push_back(node);
}

void Model::SetPattern(unsigned int index)
{
	if (m_patterns.empty() || index > m_patterns.size() - 1) return;
	const Pattern &pat = m_patterns.at(index);
	m_colorMap.SetSmooth(pat.smoothColor);
	m_curPatternIndex = index;
	m_curPattern = pat.texture.Get();
}

void Model::SetColors(const std::vector<Color> &colors)
{
	assert(colors.size() == 3); //primary, seconday, trim
	m_colorMap.Generate(GetRenderer(), colors.at(0), colors.at(1), colors.at(2));
}

void Model::SetDecalTexture(Graphics::Texture *t, unsigned int index)
{
	index = std::min(index, MAX_DECAL_MATERIALS-1);
	if (m_decalMaterials[index])
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
	if (m_decalMaterials[index])
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

	wr.Int32(m_curPatternIndex);
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

	SetPattern(rd.Int32());
}

std::string Model::GetNameForMaterial(Graphics::Material *mat) const
{
	for (auto it : m_materials) {
		Graphics::Material* modelMat = it.second.Get();
		if (modelMat == mat) return it.first;
	}

	//check decal materials
	for (Uint32 i = 0; i < MAX_DECAL_MATERIALS; i++) {
		if (m_decalMaterials[i].Valid() && m_decalMaterials[i].Get() == mat)
			return stringf("decal_%0{u}", i + 1);
	}

	return "unknown";
}

void Model::AddAxisIndicators(const std::vector<MatrixTransform*> &mts, std::vector<Graphics::Drawables::Line3D> &lines)
{
	for (std::vector<MatrixTransform*>::const_iterator i = mts.begin(); i != mts.end(); ++i) {
		const matrix4x4f &trans = (*i)->GetTransform();
		const vector3f pos = trans.GetTranslate();
		const matrix3x3f &orient = trans.GetOrient();
		const vector3f x = orient.VectorX().Normalized();
		const vector3f y = orient.VectorY().Normalized();
		const vector3f z = orient.VectorZ().Normalized();

		Graphics::Drawables::Line3D lineX;
		lineX.SetStart(pos);
		lineX.SetEnd(pos+x);
		lineX.SetColor(Color::RED);

		Graphics::Drawables::Line3D lineY;
		lineY.SetStart(pos);
		lineY.SetEnd(pos+y);
		lineY.SetColor(Color::GREEN);

		Graphics::Drawables::Line3D lineZ;
		lineZ.SetStart(pos);
		lineZ.SetEnd(pos+z);
		lineZ.SetColor(Color::BLUE);

		lines.push_back(lineX);
		lines.push_back(lineY);
		lines.push_back(lineZ);
	}
}

void Model::SetDebugFlags(Uint32 flags) {
    m_debugFlags = flags;

    if (m_debugFlags & SceneGraph::Model::DEBUG_TAGS && m_tagPoints.empty()) {
		std::vector<MatrixTransform*> mts;
		FindTagsByStartOfName("tag_", mts);
		AddAxisIndicators(mts, m_tagPoints);
	}

	if (m_debugFlags & SceneGraph::Model::DEBUG_DOCKING && m_dockingPoints.empty()) {
		std::vector<MatrixTransform*> mts;
		FindTagsByStartOfName("approach_", mts);
		AddAxisIndicators(mts, m_dockingPoints);
		FindTagsByStartOfName("docking_", mts);
		AddAxisIndicators(mts, m_dockingPoints);
		FindTagsByStartOfName("leaving_", mts);
		AddAxisIndicators(mts, m_dockingPoints);
    }
}

}
