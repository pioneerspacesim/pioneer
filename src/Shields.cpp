// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Shields.h"

#include "GameSaveError.h"
#include "JsonUtils.h"
#include "Ship.h"
#include "graphics/RenderState.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "scenegraph/CollisionGeometry.h"
#include "scenegraph/FindNodeVisitor.h"
#include "scenegraph/Node.h"
#include "scenegraph/SceneGraph.h"
#include <sstream>

namespace {
	static constexpr size_t MAX_SHIELD_HITS = 8;
	struct ShieldData {
		struct ShieldHitInfo {
			vector3f hitPos;
			float radii;
		} hits[MAX_SHIELD_HITS];

		alignas(16) float shieldStrength;
		float shieldCooldown;
	};

	static RefCountedPtr<Graphics::Material> s_matShield;
	static ShieldData s_shieldRenderData;
	static const std::string s_shieldGroupName("Shields");
	static const std::string s_matrixTransformName("_accMtx4");
	static const size_t s_shieldDataName = Graphics::Renderer::GetName("ShieldData");
	static const size_t s_numHitsName = Graphics::Renderer::GetName("NumHits");

	static RefCountedPtr<Graphics::Material> GetGlobalShieldMaterial()
	{
		return s_matShield;
	}
} // namespace

//used to find the accumulated transform of a MatrixTransform
class MatrixAccumVisitor : public SceneGraph::NodeVisitor {
public:
	MatrixAccumVisitor(const std::string &name_) :
		outMat(matrix4x4f::Identity()),
		m_accumMat(matrix4x4f::Identity()),
		m_name(name_)
	{
	}

	virtual void ApplyMatrixTransform(SceneGraph::MatrixTransform &mt) override
	{
		if (mt.GetName() == m_name) {
			outMat = m_accumMat * mt.GetTransform();
		} else {
			const matrix4x4f prevAcc = m_accumMat;
			m_accumMat = m_accumMat * mt.GetTransform();
			mt.Traverse(*this);
			m_accumMat = prevAcc;
		}
	}

	matrix4x4f outMat;

private:
	matrix4x4f m_accumMat;
	std::string m_name;
};

typedef std::vector<Shields::Shield>::iterator ShieldIterator;

//static
bool Shields::s_initialised = false;

Shields::Shield::Shield(const Color3ub &_colour, const matrix4x4f &matrix, SceneGraph::StaticGeometry *_sg) :
	m_colour(_colour),
	m_matrix(matrix),
	m_mesh(_sg)
{}

Shields::Hits::Hits(const vector3d &_pos, const Uint32 _start, const Uint32 _end) :
	pos(_pos),
	start(_start),
	end(_end)
{}

void Shields::Init(Graphics::Renderer *renderer)
{
	PROFILE_SCOPED()
	assert(!s_initialised);

	// create our global shield material
	Graphics::MaterialDescriptor desc;
	desc.textures = 0;
	desc.lighting = true;
	desc.alphaTest = false;

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA;
	rsd.depthWrite = false;
	rsd.cullMode = Graphics::CULL_NONE;

	s_matShield.Reset(renderer->CreateMaterial("shield", desc, rsd));
	s_matShield->diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);

	s_initialised = true;
}

void Shields::ReparentShieldNodes(SceneGraph::Model *model)
{
	assert(s_initialised);

	Graphics::Renderer *renderer = model->GetRenderer();

	using SceneGraph::Group;
	using SceneGraph::MatrixTransform;
	using SceneGraph::Node;
	using SceneGraph::StaticGeometry;

	//This will find all matrix transforms meant for shields.
	SceneGraph::FindNodeVisitor shieldFinder(SceneGraph::FindNodeVisitor::MATCH_NAME_ENDSWITH, "_shield");
	model->GetRoot()->Accept(shieldFinder);
	const std::vector<Node *> &results = shieldFinder.GetResults();

	//Move shield geometry to same level as the LODs
	for (unsigned int i = 0; i < results.size(); i++) {
		MatrixTransform *mt = dynamic_cast<MatrixTransform *>(results.at(i));
		assert(mt);

		const Uint32 NumChildren = mt->GetNumChildren();
		if (NumChildren > 0) {
			// Group to contain all of the shields we might find
			Group *shieldGroup = new Group(renderer);
			shieldGroup->SetName(s_shieldGroupName);

			// go through all of this MatrixTransforms children to extract all of the shield meshes
			for (Uint32 iChild = 0; iChild < NumChildren; ++iChild) {
				Node *node = mt->GetChildAt(iChild);
				assert(node);
				if (node) {
					RefCountedPtr<StaticGeometry> sg(dynamic_cast<StaticGeometry *>(node));
					assert(sg.Valid());
					sg->SetNodeMask(SceneGraph::NODE_TRANSPARENT);

					for (Uint32 iMesh = 0; iMesh < sg->GetNumMeshes(); ++iMesh) {
						StaticGeometry::Mesh &rMesh = sg->GetMeshAt(iMesh);
						rMesh.material = GetGlobalShieldMaterial();
					}

					// find the accumulated transform from the root to our node
					MatrixAccumVisitor mav(mt->GetName());
					model->GetRoot()->Accept(mav);

					// set our nodes transformation to be the accumulated transform
					MatrixTransform *sg_transform_parent = new MatrixTransform(renderer, mav.outMat);
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

Shields::Shields(SceneGraph::Model *model) :
	m_enabled(false)
{
	assert(s_initialised);

	using SceneGraph::CollisionGeometry;
	using SceneGraph::MatrixTransform;
	using SceneGraph::Node;
	using SceneGraph::StaticGeometry;

	//This will find all matrix transforms meant for shields.
	SceneGraph::FindNodeVisitor shieldFinder(SceneGraph::FindNodeVisitor::MATCH_NAME_ENDSWITH, s_matrixTransformName);
	model->GetRoot()->Accept(shieldFinder);
	const std::vector<Node *> &results = shieldFinder.GetResults();

	//Store pointer to the shields for later.
	for (unsigned int i = 0; i < results.size(); i++) {
		MatrixTransform *mt = dynamic_cast<MatrixTransform *>(results.at(i));
		assert(mt);

		for (Uint32 iChild = 0; iChild < mt->GetNumChildren(); ++iChild) {
			Node *node = mt->GetChildAt(iChild);
			if (node) {
				RefCountedPtr<StaticGeometry> sg(dynamic_cast<StaticGeometry *>(node));
				assert(sg.Valid());
				sg->SetNodeMask(SceneGraph::NODE_TRANSPARENT);

				// set the material
				for (Uint32 iMesh = 0; iMesh < sg->GetNumMeshes(); ++iMesh) {
					StaticGeometry::Mesh &rMesh = sg->GetMeshAt(iMesh);
					rMesh.material = GetGlobalShieldMaterial();
				}

				m_shields.push_back(Shield(Color3ub(255), mt->GetTransform(), sg.Get()));
			}
		}
	}
}

Shields::~Shields()
{
}

void Shields::SaveToJson(Json &jsonObj)
{
	Json shieldsObj({}); // Create JSON object to contain shields data.

	shieldsObj["enabled"] = m_enabled;
	shieldsObj["num_shields"] = m_shields.size();

	Json shieldArray = Json::array(); // Create JSON array to contain shield data.
	for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
		Json shieldArrayEl({}); // Create JSON object to contain shield.
		shieldArrayEl["color"] = it->m_colour;
		shieldArrayEl["mesh_name"] = it->m_mesh->GetName();
		shieldArray.push_back(shieldArrayEl); // Append shield object to array.
	}
	shieldsObj["shield_array"] = shieldArray; // Add shield array to shields object.

	jsonObj["shields"] = shieldsObj; // Add shields object to supplied object.
}

void Shields::LoadFromJson(const Json &jsonObj)
{
	try {
		Json shieldsObj = jsonObj["shields"];

		m_enabled = shieldsObj["enabled"];
		assert(shieldsObj["num_shields"].get<unsigned int>() == m_shields.size());

		Json shieldArray = shieldsObj["shield_array"].get<Json::array_t>();

		for (unsigned int i = 0; i < shieldArray.size(); ++i) {
			Json shieldArrayEl = shieldArray[i];
			for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
				if (shieldArrayEl["mesh_name"] == it->m_mesh->GetName()) {
					it->m_colour = shieldArrayEl["color"];
					break;
				}
			}
		}
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

void Shields::Update(const float coolDown, const float shieldStrength)
{
	// update hits on the shields
	const Uint32 tickTime = SDL_GetTicks();
	{
		HitIterator it = m_hits.begin();
		while (it != m_hits.end()) {
			if (tickTime > it->end) {
				it = m_hits.erase(it);
			} else {
				++it;
			}
		}
	}

	if (!m_enabled) {
		for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
			it->m_mesh->SetNodeMask(0x0);
		}
		return;
	}

	// setup the render params
	// FIXME: don't use a static variable to hold all of this
	if (shieldStrength > 0.0f) {
		Uint32 numHits = std::min(m_hits.size(), MAX_SHIELD_HITS);
		for (Uint32 i = 0; i < numHits; ++i) {
			const Hits &hit = m_hits[i];

			//Calculate the impact's radius dependant on time
			Uint32 dif1 = hit.end - hit.start;
			Uint32 dif2 = tickTime - hit.start;
			//Range from start (0.0) to end (1.0)
			float dif = float(dif2 / (dif1 * 1.0f));

			s_shieldRenderData.hits[i].hitPos = vector3f(hit.pos.x, hit.pos.y, hit.pos.z);
			s_shieldRenderData.hits[i].radii = dif;
		}

		s_shieldRenderData.shieldStrength = shieldStrength;
		s_shieldRenderData.shieldCooldown = coolDown;

		// FIXME: this is marginally doable given that we're immediately drawing and uploading the data
		// This should really use a better system like a per-model shield material or possibly a
		// way to bind per-frame buffer uploads in with the draw command
		GetGlobalShieldMaterial()->SetBuffer(s_shieldDataName,
			&s_shieldRenderData, Graphics::BUFFER_USAGE_DYNAMIC);
		GetGlobalShieldMaterial()->SetPushConstant(s_numHitsName, int(numHits));
	}

	// update the shield visibility
	for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
		it->m_mesh->SetNodeMask(shieldStrength > 0.0f ? SceneGraph::NODE_TRANSPARENT : 0x0);
	}
}

void Shields::SetColor(const Color3ub &inCol)
{
	for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
		it->m_colour = inCol;
	}
}

void Shields::AddHit(const vector3d &hitPos)
{
	Uint32 tickTime = SDL_GetTicks();
	m_hits.push_back(Hits(hitPos, tickTime, tickTime + 1000));
}

SceneGraph::StaticGeometry *Shields::GetFirstShieldMesh()
{
	for (ShieldIterator it = m_shields.begin(); it != m_shields.end(); ++it) {
		if (it->m_mesh) {
			return it->m_mesh.Get();
		}
	}

	return nullptr;
}
