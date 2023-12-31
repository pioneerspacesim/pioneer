// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_MODEL_H
#define _SCENEGRAPH_MODEL_H
/*
 * A new model system with a scene graph based approach.
 * Also see: https://wiki.pioneerspacesim.net/wiki/Model_system
 * Open Asset Import Library (assimp) is used as the mesh loader.
 *
 * Similar systems:
 *  - OpenSceneGraph http://www.openscenegraph.org/projects/osg has been
 *    an inspiration for naming some things and it also uses node visitors.
 *    It is a lot more complicated however
 *  - Assimp also has its own scenegraph structure (much simpler)
 *
 * A model has an internal stucture of one or (almost always several) nodes
 * For example:
 * RootNode
 *    MatrixTransformNode (applies a scale or something to child nodes)
 *        LodSwitchNode (picks 1-3)
 *            StaticGeometry_low
 *            StaticGeometry_med
 *            StaticGeometry_hi
 *
 * It's not supposed to be too complex. For example there are no "Material" nodes.
 * Geometry nodes can contain multiple separate meshes. Nodes should only be attached
 * to a single parent for consistency.
 *
 * Models are defined in a simple .model text file, which describes materials,
 * detail levels, meshes to import to each detail level and animations.
 *
 * While assimp supports a large number of formats most models are expected
 * to use Collada (.dae). The format needs to support node names since many
 * special features are based on that.
 *
 * Loading all the meshes can be quite slow, so there will be a system to
 * compile them into a more game-friendly binary format.
 *
 * Animation: position/rotation/scale keyframe animation affecting MatrixTransforms,
 * and subsequently nodes attached to them. There is no animation blending, although
 * an animated matrixtransform can be attached to another further down the chain just fine.
 * Due to format & exporter limitations, animations need to be combined into one timeline
 * and then split into new animations using frame ranges.
 *
 * Attaching models to other models (guns etc. to ships): models may specify
 * named hardpoints, known as "tags" (term from Q3).
 * Users can query tags by name or index and create a ModelNode to wrap the sub model
 *
 * Minor features:
 *  - pattern + customizable colour system (one pattern per model). Patterns can be
 *    dropped into the model directory.
 *  - dynamic textures (logos on spaceships, advertisements on stations)
 *  - 3D labels (well, 2D) on models
 *  - spaceship thrusters
 *
 * Things to optimize:
 *  - model cache
 *  - removing unnecessary nodes from the scene graph: pre-translate unanimated meshes etc.
 */
#include "CollMesh.h"
#include "ColorMap.h"
#include "DeleteEmitter.h"
#include "Group.h"
#include "JsonFwd.h"
#include "Pattern.h"
#include "graphics/Drawables.h"
#include "graphics/Material.h"
#include <stdexcept>

namespace SceneGraph {
	class Animation;
	class BaseLoader;
	class BinaryConverter;
	class MatrixTransform;
	class ModelBinarizer;
	class Tag;

	struct LoadingError : public std::runtime_error {
		LoadingError(const std::string &str) :
			std::runtime_error(str.c_str()) {}
	};

	typedef std::vector<std::pair<std::string, RefCountedPtr<Graphics::Material>>> MaterialContainer;
	typedef std::vector<Animation *> AnimationContainer;

	class Model : public DeleteEmitter {
	public:
		friend class BaseLoader;
		friend class Loader;
		friend class ModelBinarizer;
		friend class BinaryConverter;
		Model(Graphics::Renderer *r, const std::string &name);
		~Model();

		Model *MakeInstance() const;

		const std::string &GetName() const { return m_name; }

		float GetDrawClipRadius() const { return m_boundingRadius; }
		void SetDrawClipRadius(float clipRadius) { m_boundingRadius = clipRadius; }

		void Render(const matrix4x4f &trans, const RenderData *rd = 0);				 //ModelNode can override RD
		void Render(const std::vector<matrix4x4f> &trans, const RenderData *rd = 0); //ModelNode can override RD

		RefCountedPtr<CollMesh> CreateCollisionMesh();
		RefCountedPtr<CollMesh> GetCollisionMesh() const { return m_collMesh; }
		void SetCollisionMesh(RefCountedPtr<CollMesh> collMesh) { m_collMesh.Reset(collMesh.Get()); }

		RefCountedPtr<Group> GetRoot() { return m_root; }

		//materials used in the nodes should be accessible from here for convenience
		RefCountedPtr<Graphics::Material> GetMaterialByName(const std::string &name) const;
		RefCountedPtr<Graphics::Material> GetMaterialByIndex(const int) const;
		unsigned int GetNumMaterials() const { return static_cast<Uint32>(m_materials.size()); }

		// Utilities for iterating the array of model tags
		// Tag indicies should not be considered stable
		size_t GetNumTags() const { return m_tags.size(); }
		Tag *GetTagByIndex(size_t index) const;

		// Find the given tag in the model
		Tag *FindTagByName(std::string_view name) const;
		void FindTagsByStartOfName(std::string_view name, std::vector<Tag *> &outTags) const;

		// Add a tag to this model in the given parent
		void AddTag(std::string_view name, Group *parent, Tag *node);
		// Recalculate tag global transforms after e.g. animation changes
		void UpdateTagTransforms();

		const PatternContainer &GetPatterns() const { return m_patterns; }
		unsigned int GetNumPatterns() const { return static_cast<Uint32>(m_patterns.size()); }
		void SetPattern(unsigned int index);
		unsigned int GetPattern() const { return m_curPatternIndex; }
		void SetColors(const std::vector<Color> &colors);
		void SetDecalTexture(Graphics::Texture *t, unsigned int index = 0);
		void ClearDecal(unsigned int index = 0);
		void ClearDecals();
		void SetLabel(const std::string &);

		//for modelviewer, at least
		bool SupportsDecals();
		bool SupportsPatterns();

		// update all animations once to ensure all transforms are correctly positioned
		void InitAnimations();
		// Get an animation matching the given name or return nullptr.
		Animation *FindAnimation(const std::string &) const;
		// Get the index of an animation in this container. If there is no such animation, returns UINT32_MAX.
		uint32_t FindAnimationIndex(Animation *) const;
		// Return a reference to all animations defined on this model.
		const std::vector<Animation *> GetAnimations() const { return m_animations; }
		// Mark an animation as actively updating. A maximum of 64 active animations are supported.
		void SetAnimationActive(uint32_t index, bool active);
		bool GetAnimationActive(uint32_t index) const;
		// Update all active animations.
		void UpdateAnimations();

		Graphics::Renderer *GetRenderer() const { return m_renderer; }

		//special for ship model use
		void SetThrust(const vector3f &linear, const vector3f &angular);

		void SetThrusterColor(const vector3f &dir, const Color &color);
		void SetThrusterColor(const std::string &name, const Color &color);
		void SetThrusterColor(const Color &color);

		void SaveToJson(Json &jsonObj) const;
		void LoadFromJson(const Json &jsonObj);

		//serialization aid
		std::string GetNameForMaterial(Graphics::Material *) const;

		enum DebugFlags { // <enum scope='SceneGraph::Model' name=ModelDebugFlags prefix=DEBUG_ public>
			DEBUG_NONE = 0x0,
			DEBUG_BBOX = 0x1,
			DEBUG_COLLMESH = 0x2,
			DEBUG_WIREFRAME = 0x4,
			DEBUG_TAGS = 0x8,
			DEBUG_DOCKING = 0x10,
			DEBUG_GEOMBBOX = 0x20
		};
		void SetDebugFlags(Uint32 flags);

	private:
		Model(const Model &);

		static const unsigned int MAX_DECAL_MATERIALS = 4;
		ColorMap m_colorMap;
		float m_boundingRadius;
		MaterialContainer m_materials; //materials are shared throughout the model graph
		PatternContainer m_patterns;
		RefCountedPtr<CollMesh> m_collMesh;
		RefCountedPtr<Graphics::Material> m_decalMaterials[MAX_DECAL_MATERIALS]; //spaceship insignia, advertising billboards
		RefCountedPtr<Group> m_root;
		Graphics::Renderer *m_renderer;
		std::string m_name;

		std::vector<Animation *> m_animations;
		uint64_t m_activeAnimations; // bitmask of actively ticking animations

		std::vector<Tag *> m_tags;		 //named attachment points

		RenderData m_renderData;

		//per-instance flavour data
		unsigned int m_curPatternIndex;
		Graphics::Texture *m_curPattern;
		Graphics::Texture *m_curDecals[MAX_DECAL_MATERIALS];

		Uint32 m_debugFlags;
		bool m_tagsDirty;

		std::unique_ptr<Graphics::MeshObject> m_debugMesh;
		std::unique_ptr<Graphics::Material> m_debugLineMat;
	};

} // namespace SceneGraph

#endif
