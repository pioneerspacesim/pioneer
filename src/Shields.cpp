// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Shields.h"
#include "graphics/TextureBuilder.h"
#include "scenegraph/FindNodeVisitor.h"
#include "scenegraph/SceneGraph.h"

namespace {
	static RefCountedPtr<Graphics::Material> s_matShield;

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

void Shields::Uninit()
{
	assert(s_initialised);

	s_initialised = false;
}

Shields::Shields(SceneGraph::Model *model, float period)
	: m_enabled(false)
{
	assert(s_initialised);

	Graphics::Renderer *renderer = model->GetRenderer();

	using SceneGraph::Node;
	using SceneGraph::MatrixTransform;
	using SceneGraph::StaticGeometry;

	//This will find all matrix transforms meant for navlights.
	SceneGraph::FindNodeVisitor shieldFinder(SceneGraph::FindNodeVisitor::MATCH_NAME_ENDSWITH, "_shield");
	model->GetRoot()->Accept(shieldFinder);
	const std::vector<Node*> &results = shieldFinder.GetResults();

	//attach light billboards
	for (unsigned int i=0; i < results.size(); i++) {
		MatrixTransform *mt = dynamic_cast<MatrixTransform*>(results.at(i));
		assert(mt);

		for(Uint32 iChild=0 ; iChild<mt->GetNumChildren() ; ++iChild) {
			Node* node = mt->GetChildAt(iChild);
			if (node)
			{
				const char* nodeTypeName = node->GetTypeName();
				printf("%s\n", nodeTypeName);

				RefCountedPtr<StaticGeometry> sg(dynamic_cast<StaticGeometry*>(node));
				assert(sg.Valid());

				for (Uint32 iMesh=0; iMesh<sg->GetNumMeshes(); ++iMesh) {
					RefCountedPtr<Graphics::StaticMesh> rMesh = sg->GetMesh(iMesh);
					rMesh->GetSurface(0)->SetMaterial(GetGlobalShieldMaterial());
				}

				// find the accumulated transform from the root to our node
				// ??? model->GetRoot()->FindNode(node->GetName()); ???

				// set our nodes transformation to be the accumulated transform
				/*MatrixTransform *sg_transform_parent = new MatrixTransform(renderer, matrix4x4f::Identity());
				sg_transform_parent->AddChild(sg.Get());
			
				// dettach node from current location in the scenegraph...
				if( !mt->RemoveChild(node) ) {
					printf("%s is bloody stubborn\n", nodeTypeName);
				}

				// attach new transform node which parents the our shields mesh at the root.
				model->GetRoot()->AddChild(sg_transform_parent);*/
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

	for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
		wr.Byte(it->m_colour.r);
		wr.Byte(it->m_colour.g);
		wr.Byte(it->m_colour.b);
	}
}

void Shields::Load(Serializer::Reader &rd)
{
	m_enabled = rd.Bool();

	RefCountedPtr<Graphics::Material> mat;
	for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
		it->m_colour.r = rd.Byte();
		it->m_colour.g = rd.Byte();
		it->m_colour.b = rd.Byte();
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
