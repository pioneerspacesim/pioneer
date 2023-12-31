// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Shields.h"

#include "GameSaveError.h"
#include "JsonUtils.h"
#include "Ship.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "graphics/UniformBuffer.h"
#include "scenegraph/CollisionGeometry.h"
#include "scenegraph/FindNodeVisitor.h"
#include "scenegraph/Node.h"
#include "scenegraph/SceneGraph.h"
#include <sstream>
#include <SDL_timer.h>

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
	static RefCountedPtr<Graphics::UniformBuffer> s_matUniformBuffer;
	static const std::string s_shieldGroupName("Shields");
	static const std::string s_matrixTransformName("_accMtx4");
	static const size_t s_shieldDataName = "ShieldData"_hash;
	static const size_t s_numHitsName = "NumHits"_hash;

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

	// Create a global shield data buffer containing "nothing" for use when there isn't an active Shields class
	// attached to the model
	s_matUniformBuffer.Reset(renderer->CreateUniformBuffer(sizeof(ShieldData), Graphics::BUFFER_USAGE_STATIC));
	s_matUniformBuffer->BufferData(ShieldData{});

	s_matShield.Reset(renderer->CreateMaterial("shield", desc, rsd));
	s_matShield->diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);
	s_matShield->SetPushConstant(s_numHitsName, 0);
	s_matShield->SetBuffer(s_shieldDataName, s_matUniformBuffer->GetBufferBinding());

	s_initialised = true;
}

void Shields::ReparentShieldNodes(SceneGraph::Model *model)
{
	using namespace SceneGraph;
	assert(s_initialised);

	Graphics::Renderer *renderer = model->GetRenderer();

	//This will find all matrix transforms meant for shields.
	SceneGraph::FindNodeVisitor shieldFinder(SceneGraph::FindNodeVisitor::MATCH_NAME_ENDSWITH, "_shield");
	model->GetRoot()->Accept(shieldFinder);

	//Move shield geometry to same level as the LODs
	for (Node *node : shieldFinder.GetResults()) {
		MatrixTransform *mt = dynamic_cast<MatrixTransform *>(node);
		if (mt == nullptr)
			continue;

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

	s_matShield.Reset();

	s_initialised = false;
}

Shields::Shields(SceneGraph::Model *model) :
	m_enabled(false)
{
	using namespace SceneGraph;
	assert(s_initialised);

	// Clone the global material and use a per-model instance
	Graphics::Renderer *r = model->GetRenderer();
	Graphics::Material *globalShield = GetGlobalShieldMaterial().Get();
	m_shieldMaterial.Reset(r->CloneMaterial(globalShield, globalShield->GetDescriptor(), r->GetMaterialRenderState(globalShield)));

	//This will find all matrix transforms meant for shields.
	SceneGraph::FindNodeVisitor shieldFinder(SceneGraph::FindNodeVisitor::MATCH_NAME_ENDSWITH, s_matrixTransformName);
	model->GetRoot()->Accept(shieldFinder);

	//Store pointer to the shields for later.
	for (Node *node : shieldFinder.GetResults()) {
		MatrixTransform *mt = dynamic_cast<MatrixTransform *>(node);
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
					rMesh.material = m_shieldMaterial;
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
	for (const auto &shield : m_shields) {
		Json shieldArrayEl({}); // Create JSON object to contain shield.
		shieldArrayEl["color"] = shield.m_colour;
		shieldArrayEl["mesh_name"] = shield.m_mesh->GetName();
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
			for (auto &shield : m_shields) {
				if (shieldArrayEl["mesh_name"] == shield.m_mesh->GetName()) {
					shield.m_colour = shieldArrayEl["color"];
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
	// FIXME: this should use a game-specific clock...
	const Uint32 tickTime = SDL_GetTicks();
	{
		auto it = m_hits.begin();
		while (it != m_hits.end()) {
			if (tickTime > it->end) {
				it = m_hits.erase(it);
			} else {
				++it;
			}
		}
	}

	if (!m_enabled) {
		for (const auto &shield : m_shields) {
			shield.m_mesh->SetNodeMask(0x0);
		}
		return;
	}

	ShieldData renderData{};

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

			renderData.hits[i].hitPos = vector3f(hit.pos.x, hit.pos.y, hit.pos.z);
			renderData.hits[i].radii = dif;
		}

		renderData.shieldStrength = shieldStrength;
		renderData.shieldCooldown = coolDown;

		m_shieldMaterial->SetBufferDynamic(s_shieldDataName, &renderData);
		m_shieldMaterial->SetPushConstant(s_numHitsName, int(numHits));
	}

	// update the shield visibility
	for (const auto &shield : m_shields) {
		shield.m_mesh->SetNodeMask(shieldStrength > 0.0f ? SceneGraph::NODE_TRANSPARENT : 0x0);
	}
}

void Shields::SetColor(const Color3ub &inCol)
{
	for (auto &shield : m_shields) {
		shield.m_colour = inCol;
	}
}

void Shields::AddHit(const vector3d &hitPos)
{
	// FIXME: should use the game time
	Uint32 tickTime = SDL_GetTicks();
	m_hits.push_back(Hits(hitPos, tickTime, tickTime + 1000));
}

SceneGraph::StaticGeometry *Shields::GetFirstShieldMesh()
{
	for (const auto &shield : m_shields) {
		if (shield.m_mesh) {
			return shield.m_mesh.Get();
		}
	}

	return nullptr;
}
