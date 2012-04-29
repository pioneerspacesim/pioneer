#include "NModel.h"
#include "CollisionVisitor.h"
#include "DrawVisitor.h"
#include "graphics/Renderer.h"

namespace Newmodel {

NModel::NModel(const std::string &name) : Model(),
	m_name(name),
	m_boundingRadius(10.f)
{
	m_root.Reset(new Group());
}

NModel::~NModel()
{
}

void NModel::Render(Graphics::Renderer *renderer, const matrix4x4f &trans, const LmrObjParams *params)
{
	renderer->SetBlendMode(Graphics::BLEND_SOLID);
	renderer->SetTransform(trans);
	/*DrawVisitor vis(renderer);
	m_root->Accept(vis);*/
	//using the entire model bounding radius for all nodes at the moment. 
	//BR could also be a property of Node.
	RenderData renderData;
	renderData.scrWidth = params->screenWidth;
	renderData.boundingRadius = GetDrawClipRadius();
	m_root->Render(renderer, trans, &renderData);
}

CollMesh *NModel::CreateCollisionMesh(const LmrObjParams *p)
{
	CollisionVisitor cv;
	m_root->Accept(cv);
	CollMesh *m = cv.CreateCollisionMesh();
	m_boundingRadius = cv.m_boundingRadius;
	return m;
}

RefCountedPtr<Graphics::Material> NModel::GetMaterialByName(const std::string &name) const
{
	for (MaterialContainer::const_iterator it = m_materials.begin();
		it != m_materials.end();
		++it)
	{
		if ((*it).first == name) return (*it).second;
	}
	return RefCountedPtr<Graphics::Material>(); //return invalid
}

RefCountedPtr<Graphics::Material> NModel::GetMaterialByIndex(const int i) const
{
	return m_materials.at(Clamp(i, 0, int(m_materials.size())-1)).second;
}

Group * const NModel::GetTagByIndex(const unsigned int i) const
{
	if (i > m_tags.size()-1) return 0;
	return m_tags.at(i);
}

Group * const NModel::FindTagByName(const std::string &name) const
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

void NModel::AddTag(const std::string &name, Group *node)
{
	if (FindTagByName(name)) return;
	node->SetName(name);
	m_root->AddChild(node);
	m_tags.push_back(node);
}

}
