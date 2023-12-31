// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Model.h"

#include "CollisionVisitor.h"
#include "FindNodeVisitor.h"
#include "GameSaveError.h"
#include "JsonUtils.h"
#include "MathUtil.h"
#include "NodeCopyCache.h"
#include "StringF.h"
#include "Thruster.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"
#include "matrix4x4.h"
#include "scenegraph/Animation.h"
#include "scenegraph/Label3D.h"
#include "scenegraph/MatrixTransform.h"
#include "scenegraph/NodeVisitor.h"
#include "scenegraph/StaticGeometry.h"
#include "scenegraph/Tag.h"
#include "utils.h"

namespace SceneGraph {

	class LabelUpdateVisitor : public NodeVisitor {
	public:
		virtual void ApplyLabel(Label3D &l)
		{
			l.SetText(label);
		}

		std::string label;
	};

	Model::Model(Graphics::Renderer *r, const std::string &name) :
		m_boundingRadius(10.f),
		m_renderer(r),
		m_name(name),
		m_activeAnimations(0),
		m_curPatternIndex(0),
		m_curPattern(0),
		m_debugFlags(0)
	{
		m_root.Reset(new Group(m_renderer));
		m_root->SetName(name);
		ClearDecals();
	}

	Model::Model(const Model &model) :
		DeleteEmitter(),
		m_boundingRadius(model.m_boundingRadius),
		m_materials(model.m_materials),
		m_patterns(model.m_patterns),
		m_collMesh(model.m_collMesh), //might have to make this per-instance at some point
		m_renderer(model.m_renderer),
		m_name(model.m_name),
		m_activeAnimations(0),
		m_curPatternIndex(model.m_curPatternIndex),
		m_curPattern(model.m_curPattern),
		m_debugFlags(0)
	{
		//selective copying of node structure
		NodeCopyCache cache;
		m_root.Reset(dynamic_cast<Group *>(model.m_root->Clone(&cache)));

		//materials are shared by meshes
		for (unsigned int i = 0; i < MAX_DECAL_MATERIALS; i++)
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
		for (const Tag *tag : model.m_tags) {
			Node *node = m_root->FindNode(tag->GetName());
			assert(node->GetNodeFlags() & NODE_TAG);
			m_tags.push_back(static_cast<Tag *>(node));
		}

		UpdateTagTransforms();
	}

	Model::~Model()
	{
		while (!m_animations.empty())
			delete m_animations.back(), m_animations.pop_back();
	}

	Model *Model::MakeInstance() const
	{
		Model *m = new Model(*this);
		return m;
	}

	void Model::Render(const matrix4x4f &trans, const RenderData *rd)
	{
		PROFILE_SCOPED()
		//update color parameters (materials are shared by model instances)
		if (m_curPattern) {
			for (auto &mat : m_materials) {
				if (mat.second->GetDescriptor().usePatterns) {
					mat.second->SetTexture("texture4"_hash, m_curPattern);
					mat.second->SetTexture("texture5"_hash, m_colorMap.GetTexture());
				}
			}
		}

		//update decals (materials and geometries are shared)
		for (unsigned int i = 0; i < MAX_DECAL_MATERIALS; i++)
			if (m_decalMaterials[i])
				m_decalMaterials[i]->SetTexture("texture0"_hash, m_curDecals[i]);

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

		if (m_debugMesh) {
			if (!m_debugLineMat) {
				Graphics::MaterialDescriptor desc;
				Graphics::RenderStateDesc rsd;
				rsd.depthWrite = false;
				rsd.primitiveType = Graphics::LINE_SINGLE;

				m_debugLineMat.reset(m_renderer->CreateMaterial("vtxColor", desc, rsd));
			}

			m_renderer->SetTransform(trans);
			m_renderer->DrawMesh(m_debugMesh.get(), m_debugLineMat.get());
		}
	}

	void Model::Render(const std::vector<matrix4x4f> &trans, const RenderData *rd)
	{
		PROFILE_SCOPED();

		//update color parameters (materials are shared by model instances)
		if (m_curPattern) {
			for (auto &mat : m_materials) {
				if (mat.second->GetDescriptor().usePatterns) {
					mat.second->SetTexture("texture4"_hash, m_curPattern);
					mat.second->SetTexture("texture5"_hash, m_colorMap.GetTexture());
				}
			}
		}

		//update decals (materials and geometries are shared)
		for (unsigned int i = 0; i < MAX_DECAL_MATERIALS; i++)
			if (m_decalMaterials[i])
				m_decalMaterials[i]->SetTexture("texture0"_hash, m_curDecals[i]);

		//Override renderdata if this model is called from ModelNode
		RenderData params = (rd != 0) ? (*rd) : m_renderData;

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

		if (m_debugFlags & DEBUG_WIREFRAME)
			m_renderer->SetWireFrameMode(false);
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
		for (auto it : m_materials) {
			if (it.first == name)
				return it.second;
		}
		return RefCountedPtr<Graphics::Material>(); //return invalid
	}

	RefCountedPtr<Graphics::Material> Model::GetMaterialByIndex(const int i) const
	{
		return m_materials.at(Clamp(i, 0, int(m_materials.size()) - 1)).second;
	}

	Tag *Model::GetTagByIndex(size_t i) const
	{
		if (m_tags.empty() || m_tags.size() <= i)
			return nullptr;

		return m_tags.at(i);
	}

	Tag *Model::FindTagByName(std::string_view name) const
	{
		for (Tag *tag : m_tags) {
			assert(!tag->GetName().empty()); //tags must have a name
			if (tag->GetName() == name)
				return tag;
		}
		return nullptr;
	}

	void Model::FindTagsByStartOfName(std::string_view name, std::vector<Tag *> &outTags) const
	{
		for (Tag *tag : m_tags) {
			assert(!tag->GetName().empty()); //tags must have a name
			if (starts_with(tag->GetName(), name)) {
				outTags.push_back(tag);
			}
		}
		return;
	}

	void Model::AddTag(std::string_view name, Group *parent, Tag *node)
	{
		if (FindTagByName(name)) return;

		node->SetName(std::string(name));
		node->SetNodeFlags(node->GetNodeFlags() | NODE_TAG);
		parent->AddChild(node);
		m_tags.push_back(node);
	}

	void Model::UpdateTagTransforms()
	{
		PROFILE_SCOPED();

		for (Tag *tag : m_tags) {
			tag->UpdateGlobalTransform();
		}
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
		index = std::min(index, MAX_DECAL_MATERIALS - 1);
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
		for (unsigned int i = 0; i < MAX_DECAL_MATERIALS; i++)
			m_curDecals[i] = t;
	}

	void Model::ClearDecal(unsigned int index)
	{
		index = std::min(index, MAX_DECAL_MATERIALS - 1);
		if (m_decalMaterials[index])
			m_curDecals[index] = Graphics::TextureBuilder::GetTransparentTexture(m_renderer);
	}

	bool Model::SupportsDecals()
	{
		for (unsigned int i = 0; i < MAX_DECAL_MATERIALS; i++)
			if (m_decalMaterials[i].Valid()) return true;

		return false;
	}

	bool Model::SupportsPatterns()
	{
		for (MaterialContainer::const_iterator it = m_materials.begin(), itEnd = m_materials.end();
			 it != itEnd;
			 ++it) {
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
		return nullptr;
	}

	void Model::InitAnimations()
	{
		for (AnimationContainer::iterator anim = m_animations.begin(); anim != m_animations.end(); ++anim)
			(*anim)->Interpolate();
	}

	void Model::UpdateAnimations()
	{
		for (size_t i = 0; i < m_animations.size(); i++) {
			if (m_activeAnimations & (1 << i))
				m_animations[i]->Interpolate();
		}

		// Assume if we're ticking an active animation, our tags most likely need to be updated.
		// This can be optimized slightly by walking the node hierarchy and looking for a "dirty"
		// flag to determine if the tag needs to be updated, but at current it's not a significant
		// performance issue compared to animation interpolation.
		if (m_activeAnimations)
			UpdateTagTransforms();
	}

	uint32_t Model::FindAnimationIndex(Animation *anim) const
	{
		for (size_t i = 0; i < m_animations.size(); i++) {
			if (anim == m_animations[i]) return uint32_t(i);
		}

		return UINT32_MAX;
	}

	void Model::SetAnimationActive(uint32_t index, bool active)
	{
		if (index >= m_animations.size()) return;
		if (active)
			m_activeAnimations |= (1 << index);
		else
			m_activeAnimations &= ~(1 << index);
	}

	bool Model::GetAnimationActive(uint32_t index) const
	{
		if (index >= m_animations.size()) return false;
		return m_activeAnimations & (1 << index);
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

	void Model::SetThrusterColor(const vector3f &dir, const Color &color)
	{
		assert(m_root != nullptr);

		FindNodeVisitor thrusterFinder(FindNodeVisitor::MATCH_NAME_FULL, "thrusters");
		m_root->Accept(thrusterFinder);
		const std::vector<Node *> &results = thrusterFinder.GetResults();
		Group *thrusters = static_cast<Group *>(results.at(0));

		for (unsigned int i = 0; i < thrusters->GetNumChildren(); i++) {
			MatrixTransform *mt = static_cast<MatrixTransform *>(thrusters->GetChildAt(i));
			Thruster *my_thruster = static_cast<Thruster *>(mt->GetChildAt(0));
			if (my_thruster == nullptr) continue;
			float dot = my_thruster->GetDirection().Dot(dir);
			if (dot > 0.99) my_thruster->SetColor(color);
		}
	}

	void Model::SetThrusterColor(const std::string &name, const Color &color)
	{
		assert(m_root != nullptr);

		FindNodeVisitor thrusterFinder(FindNodeVisitor::MATCH_NAME_FULL, name);
		m_root->Accept(thrusterFinder);
		const std::vector<Node *> &results = thrusterFinder.GetResults();

		//Hope there's only 1 result...
		Thruster *my_thruster = static_cast<Thruster *>(results.at(0));
		if (my_thruster != nullptr) my_thruster->SetColor(color);
	}

	void Model::SetThrusterColor(const Color &color)
	{
		assert(m_root != nullptr);

		FindNodeVisitor thrusterFinder(FindNodeVisitor::MATCH_NAME_FULL, "thrusters");
		m_root->Accept(thrusterFinder);
		const std::vector<Node *> &results = thrusterFinder.GetResults();
		Group *thrusters = static_cast<Group *>(results.at(0));

		for (unsigned int i = 0; i < thrusters->GetNumChildren(); i++) {
			MatrixTransform *mt = static_cast<MatrixTransform *>(thrusters->GetChildAt(i));
			Thruster *my_thruster = static_cast<Thruster *>(mt->GetChildAt(0));
			assert(my_thruster != nullptr);
			my_thruster->SetColor(color);
		}
	}

	void Model::SaveToJson(Json &jsonObj) const
	{
		Json modelObj({}); // Create JSON object to contain model data.

		Json animationArray = Json::array(); // Create JSON array to contain animation data.
		Json activeArray = Json::array();	 // Create JSON array to contain animation data.
		for (size_t i = 0; i < m_animations.size(); i++) {
			animationArray.push_back(m_animations[i]->GetProgress());
			activeArray.push_back(GetAnimationActive(i));
		}
		modelObj["animations"] = animationArray; // Add animation array to model object.
		modelObj["activeAnimations"] = activeArray;

		modelObj["cur_pattern_index"] = m_curPatternIndex;

		jsonObj["model"] = modelObj; // Add model object to supplied object.
	}

	void Model::LoadFromJson(const Json &jsonObj)
	{
		try {
			Json modelObj = jsonObj["model"];

			Json animationArray = modelObj["animations"].get<Json::array_t>();
			Json activeArray = modelObj["activeAnimations"];
			if (m_animations.size() == animationArray.size()) {
				unsigned int arrayIndex = 0;
				bool hasActive = activeArray.is_array();
				for (auto i : m_animations) {
					i->SetProgress(animationArray[arrayIndex]);
					SetAnimationActive(arrayIndex, hasActive ? activeArray[arrayIndex].get<bool>() : true);
					++arrayIndex;
				}

			} else {
				Log::Info("Saved model '{}' has invalid animation data. The model file may have changed on disk.\n", m_name);
			}
			InitAnimations();

			SetPattern(modelObj["cur_pattern_index"]);
		} catch (Json::type_error &) {
			throw SavedGameCorruptException();
		}
	}

	std::string Model::GetNameForMaterial(Graphics::Material *mat) const
	{
		for (auto it : m_materials) {
			Graphics::Material *modelMat = it.second.Get();
			if (modelMat == mat) return it.first;
		}

		//check decal materials
		for (Uint32 i = 0; i < MAX_DECAL_MATERIALS; i++) {
			if (m_decalMaterials[i].Valid() && m_decalMaterials[i].Get() == mat)
				return stringf("decal_%0{u}", i + 1);
		}

		return "unknown";
	}

	// Debug Visualization Handling
	// ========================================================================

	static void AddAxisIndicators(const std::vector<Tag *> &mts, Graphics::VertexArray &lines)
	{
		for (const Tag *tag : mts) {
			const matrix4x4f &trans = tag->GetGlobalTransform();
			const vector3f pos = trans.GetTranslate();
			const matrix3x3f &orient = trans.GetOrient();
			const vector3f x = orient.VectorX().Normalized();
			const vector3f y = orient.VectorY().Normalized();
			const vector3f z = orient.VectorZ().Normalized();

			lines.Add(pos, Color::RED);
			lines.Add(pos + x, Color::RED * 0.5);

			lines.Add(pos, Color::GREEN);
			lines.Add(pos + y, Color::GREEN * 0.5);

			lines.Add(pos, Color::BLUE);
			lines.Add(pos + z, Color::BLUE * 0.5);
		}
	}

	static void AddCollMeshVisualizer(const CollMesh *collMesh, Graphics::VertexArray &lines)
	{
		const std::vector<vector3f> &vertices = collMesh->GetGeomTreeVertices();
		const Uint32 *indices = collMesh->GetGeomTreeIndices();
		const unsigned int *triFlags = collMesh->GetGeomTreeTriFlags();

		for (unsigned int i = 0; i < collMesh->GetGeomTreeNumTris(); i++) {
			//show special geomflags in red
			Color4ub color = triFlags[i] > 0 ? Color::RED : Color::WHITE;

			uint32_t idx = i * 3;
			// draw one line for each edge of the triangle;
			// this may be wasteful with shared triangle edges but avoids the need for a separate drawcall
			lines.Add(vertices[indices[idx]], color);
			lines.Add(vertices[indices[idx + 1]], color);

			lines.Add(vertices[indices[idx + 1]], color);
			lines.Add(vertices[indices[idx + 2]], color);

			lines.Add(vertices[indices[idx + 2]], color);
			lines.Add(vertices[indices[idx]], color);
		}
	}

	static void AddAABBVisualizer(const Aabb &aabb, Color color, Graphics::VertexArray &lines, const matrix4x4f &transform = matrix4x4fIdentity)
	{
		PROFILE_SCOPED()

		Graphics::Drawables::AABB::DrawVertices(lines, transform, aabb, color);
	}

	static void AddClipSphereVisualizer(float radius, Color color, Graphics::VertexArray &lines)
	{
		constexpr float STEP = float(M_PI) / 72;
		// XY plane
		for (float theta = 0; theta < float(2 * M_PI); theta += STEP) {
			lines.Add(vector3f(radius * sin(theta), radius * cos(theta), 0), color);
			lines.Add(vector3f(radius * sin(theta + STEP), radius * cos(theta + STEP), 0), color);
		}

		// XZ plane
		for (float theta = 0; theta < float(2 * M_PI); theta += STEP) {
			lines.Add(vector3f(radius * sin(theta), 0, radius * cos(theta)), color);
			lines.Add(vector3f(radius * sin(theta + STEP), 0, radius * cos(theta + STEP)), color);
		}

		// YZ plane
		for (float theta = 0; theta < float(2 * M_PI); theta += STEP) {
			lines.Add(vector3f(0, radius * sin(theta), radius * cos(theta)), color);
			lines.Add(vector3f(0, radius * sin(theta + STEP), radius * cos(theta + STEP)), color);
		}
	}

	class ModelAABBVisitor final : public SceneGraph::NodeVisitor {
	public:
		ModelAABBVisitor(Graphics::VertexArray &lines) :
			lines(lines)
		{
			matrixStack.push_back(matrix4x4fIdentity);
		}

		void ApplyMatrixTransform(MatrixTransform &mt) override
		{
			matrixStack.push_back(matrixStack.back() * mt.GetTransform());
			mt.Traverse(*this);
			matrixStack.pop_back();
		}

		void ApplyStaticGeometry(StaticGeometry &sg) override
		{
			AddAABBVisualizer(sg.m_boundingBox, Color::YELLOW, lines, matrixStack.back());
		}

	private:
		std::vector<matrix4x4f> matrixStack;
		Graphics::VertexArray &lines;
	};

	void Model::SetDebugFlags(Uint32 flags)
	{
		m_debugFlags = flags;

		// reserve a decent amount of space if we're going to be drawing something
		Graphics::VertexArray debugLines(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE, m_debugFlags ? 256 : 0);

		if (m_debugFlags & Model::DEBUG_TAGS) {
			std::vector<Tag *> tags;
			FindTagsByStartOfName("tag_", tags);
			AddAxisIndicators(tags, debugLines);
		}

		if (m_debugFlags & Model::DEBUG_DOCKING) {
			std::vector<Tag *> tags;
			FindTagsByStartOfName("entrance_", tags);
			AddAxisIndicators(tags, debugLines);

			tags.clear();
			FindTagsByStartOfName("loc_", tags);
			AddAxisIndicators(tags, debugLines);

			tags.clear();
			FindTagsByStartOfName("exit_", tags);
			AddAxisIndicators(tags, debugLines);
		}

		if (m_debugFlags & Model::DEBUG_COLLMESH && m_collMesh) {
			AddCollMeshVisualizer(m_collMesh.Get(), debugLines);
		}

		if (m_debugFlags & Model::DEBUG_BBOX && m_collMesh) {
			AddAABBVisualizer(m_collMesh->GetAabb(), Color::GREEN, debugLines);
			AddClipSphereVisualizer(m_boundingRadius, Color::STEELBLUE, debugLines);
		}

		if (m_debugFlags & Model::DEBUG_GEOMBBOX) {
			ModelAABBVisitor visitor(debugLines);
			m_root->Accept(visitor);
		}

		// Create the debug mesh if we have something to display.
		if (!debugLines.IsEmpty()) {
			m_debugMesh.reset(m_renderer->CreateMeshObjectFromArray(&debugLines));
		} else {
			m_debugMesh.reset();
		}
	}

} // namespace SceneGraph
