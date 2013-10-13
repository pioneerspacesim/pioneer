// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Shields.h"
#include "graphics/TextureBuilder.h"
#include "scenegraph/FindNodeVisitor.h"
#include "scenegraph/SceneGraph.h"
#include "Ship.h"
#include <sstream>

namespace {
	static RefCountedPtr<Graphics::Material> s_matShield;
	static ShieldRenderParameters s_renderParams;
	static const std::string s_shieldGroupName("Shields");
	static const std::string s_matrixTransformName("_accMtx4");

	static RefCountedPtr<Graphics::Material> GetGlobalShieldMaterial()
	{
		return s_matShield;
	}
};

typedef std::vector<Shields::Shield>::iterator ShieldIterator;

//static 
bool Shields::s_initialised = false;

Shields::Shield::Shield(Color3ub _colour, SceneGraph::StaticGeometry *_sg)
	: m_colour(_colour), m_mesh(_sg)
{
}

void Shields::Init(Graphics::Renderer *renderer)
{
	assert(!s_initialised);
	
	// create our global shield material
	Graphics::MaterialDescriptor desc;
	desc.textures = 0;
	desc.lighting = true;
	desc.twoSided = false;
	desc.alphaTest = false;
	desc.effect = Graphics::EffectType::EFFECT_SHIELD;
	s_matShield.Reset(renderer->CreateMaterial(desc));
	s_matShield->diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);

	s_initialised = true;
}

void Shields::ReparentShieldNodes(SceneGraph::Model* model)
{
	assert(s_initialised);

	Graphics::Renderer *renderer = model->GetRenderer();

	using SceneGraph::Node;
	using SceneGraph::Group;
	using SceneGraph::MatrixTransform;
	using SceneGraph::StaticGeometry;

	//This will find all matrix transforms meant for navlights.
	SceneGraph::FindNodeVisitor shieldFinder(SceneGraph::FindNodeVisitor::MATCH_NAME_ENDSWITH, "_shield");
	model->GetRoot()->Accept(shieldFinder);
	const std::vector<Node*> &results = shieldFinder.GetResults();

	//Move shield geometry to same level as the LODs
	for (unsigned int i = 0; i < results.size(); i++) {
		MatrixTransform *mt = dynamic_cast<MatrixTransform*>(results.at(i));
		assert(mt);

		const Uint32 NumChildren = mt->GetNumChildren();
		if (NumChildren>0)
		{
			// Group to contain all of the shields we might find
			Group *shieldGroup = new Group(renderer);
			shieldGroup->SetName(s_shieldGroupName);

			// go through all of this MatrixTransforms children to extract all of the shield meshes
			for (Uint32 iChild = 0; iChild < NumChildren; ++iChild) {
				Node* node = mt->GetChildAt(iChild);
				assert(node);
				if (node)
				{
					RefCountedPtr<StaticGeometry> sg(dynamic_cast<StaticGeometry*>(node));
					assert(sg.Valid());
					sg->SetNodeMask(SceneGraph::NODE_TRANSPARENT);
					sg->DisableDepthWrite();

					// We can early-out if we've already processed this models scenegraph.
					if (Graphics::BLEND_ALPHA == sg->m_blendMode) {
						assert(false);
					}

					// force the blend mode
					sg->m_blendMode = Graphics::BLEND_ALPHA;

					for (Uint32 iMesh = 0; iMesh < sg->GetNumMeshes(); ++iMesh) {
						RefCountedPtr<Graphics::StaticMesh> rMesh = sg->GetMesh(iMesh);

						for (Sint32 surfIdx = 0, endSurf = rMesh->GetNumSurfaces(); surfIdx < endSurf; surfIdx++) {
							RefCountedPtr<Graphics::Surface> surf = rMesh->GetSurface(surfIdx);
							if (surf.Valid()) {
								surf->SetMaterial(GetGlobalShieldMaterial());
							}
						}
					}

					// find the accumulated transform from the root to our node
					matrix4x4f accum(matrix4x4f::Identity());
					matrix4x4f outMat(matrix4x4f::Identity());
					const Node* foundNode = model->GetRoot()->GatherTransforms(mt->GetName(), accum, outMat);

					// set our nodes transformation to be the accumulated transform
					MatrixTransform *sg_transform_parent = new MatrixTransform(renderer, outMat);
					std::stringstream nodeStream;
					nodeStream << iChild << s_matrixTransformName;
					sg_transform_parent->SetName(nodeStream.str());
					sg_transform_parent->AddChild(sg.Get());

					// dettach node from current location in the scenegraph...
					mt->RemoveChild(node);

					// attach new transform node which parents the our shields mesh to the shield group.
					shieldGroup->AddChild(sg_transform_parent);
				}
			}
			
			model->GetRoot()->AddChild(shieldGroup);
		}
	}
}

void Shields::Uninit()
{
	assert(s_initialised);

	s_initialised = false;
}

Shields::Shields(SceneGraph::Model *model)
	: m_enabled(false)
{
	assert(s_initialised);

	using SceneGraph::Node;
	using SceneGraph::MatrixTransform;
	using SceneGraph::StaticGeometry;

	//This will find all matrix transforms meant for navlights.
	SceneGraph::FindNodeVisitor shieldFinder(SceneGraph::FindNodeVisitor::MATCH_NAME_ENDSWITH, s_matrixTransformName);
	model->GetRoot()->Accept(shieldFinder);
	const std::vector<Node*> &results = shieldFinder.GetResults();

	//Move shield geometry to same level as the LODs
	for (unsigned int i=0; i < results.size(); i++) {
		MatrixTransform *mt = dynamic_cast<MatrixTransform*>(results.at(i));
		assert(mt);

		for(Uint32 iChild=0 ; iChild<mt->GetNumChildren() ; ++iChild) {
			Node* node = mt->GetChildAt(iChild);
			if (node)
			{
				RefCountedPtr<StaticGeometry> sg(dynamic_cast<StaticGeometry*>(node));
				assert(sg.Valid());
				sg->SetNodeMask(SceneGraph::NODE_TRANSPARENT);
				m_shields.push_back(Shield(Color3ub(255), sg.Get()));
			}
		}
	}
}

Shields::~Shields()
{
}

void Shields::Save(Serializer::Writer &wr)
{
	wr.Bool(m_enabled);

	wr.Int32(m_shields.size());
	for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
		wr.Byte(it->m_colour.r);
		wr.Byte(it->m_colour.g);
		wr.Byte(it->m_colour.b);
		wr.String(it->m_mesh->GetName());
	}
}

void Shields::Load(Serializer::Reader &rd)
{
	m_enabled = rd.Bool();

	const Uint32 NumShields = rd.Int32();
	assert(NumShields == m_shields.size());
	for (Uint32 iRead = 0; iRead < NumShields; iRead++ ) {
		const Uint8 r = rd.Byte();
		const Uint8 g = rd.Byte();
		const Uint8 b = rd.Byte();
		const std::string name = rd.String();
		for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
			if(name==it->m_mesh->GetName()) {
				it->m_colour = Color3ub(r, g, b);
				break;
			}
		}
	}
}

void Shields::Update(const float shieldStrength /* 0.0f to 1.0f */)
{
	if (!m_enabled) {
		for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
			it->m_mesh->SetNodeMask(0x0);
		}
		return;
	}

	for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
		if (shieldStrength>0.0f) {
			it->m_mesh->SetNodeMask(SceneGraph::NODE_TRANSPARENT);
			it->m_mesh->DisableDepthWrite();
			s_renderParams.strength = shieldStrength;
			GetGlobalShieldMaterial()->specialParameter0 = &s_renderParams;
		} else {
			it->m_mesh->SetNodeMask(0x0);
		}
	}
}

void Shields::SetColor(const Color3ub inCol)
{
	for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
		it->m_colour = inCol;
	}
}
