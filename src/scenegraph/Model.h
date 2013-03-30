// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_MODEL_H
#define _SCENEGRAPH_MODEL_H
/*
 * A new model system with a scene graph based approach.
 * Also see: http://pioneerwiki.com/wiki/New_Model_System
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
 * Geometry nodes can contain multiple separate meshes. One node can be attached to
 * multiple parents to achieve a simple form of instancing, although the support for
 * this is dependanant on tools.
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
#include "libs.h"
#include "Animation.h"
#include "ColorMap.h"
#include "Group.h"
#include "Label3D.h"
#include "Pattern.h"
#include "CollMesh.h"
#include "graphics/Material.h"
#include "Serializer.h"
#include <stdexcept>

namespace Graphics { class Renderer; }

namespace SceneGraph
{

struct LoadingError : public std::runtime_error {
	LoadingError(const std::string &str) : std::runtime_error(str.c_str()) { }
};

typedef std::vector<std::pair<std::string, RefCountedPtr<Graphics::Material> > > MaterialContainer;
typedef std::vector<Animation*> AnimationContainer;
typedef std::vector<MatrixTransform *> TagContainer;

class Model
{
public:
	friend class Loader;
	Model(Graphics::Renderer *r, const std::string &name);
	~Model();
	Model *MakeInstance() const;
	float GetDrawClipRadius() const { return m_boundingRadius; }
	void Render(const matrix4x4f &trans, RenderData *params = 0); //ModelNode can override RD
	RefCountedPtr<CollMesh> CreateCollisionMesh();
	CollMesh *GetCollisionMesh() const { return m_collMesh.Get(); }
	RefCountedPtr<Group> GetRoot() { return m_root; }
	//materials used in the nodes should be accessible from here for convenience
	RefCountedPtr<Graphics::Material> GetMaterialByName(const std::string &name) const;
	RefCountedPtr<Graphics::Material> GetMaterialByIndex(int) const;

	int GetNumTags() const { return m_tags.size(); }
	MatrixTransform * const GetTagByIndex(unsigned int index) const;
	MatrixTransform * const FindTagByName(const std::string &name) const;
	typedef std::vector<MatrixTransform *> TVecMT;
	void FindTagsByStartOfName(const std::string &name, TVecMT &outNameMTs) const;
	void AddTag(const std::string &name, MatrixTransform *node);

	const PatternContainer &GetPatterns() const { return m_patterns; }
	unsigned int GetNumPatterns() const { return m_patterns.size(); }
	void SetPattern(unsigned int index);
	void SetColors(const std::vector<Color4ub> &colors);
	void SetDecalTexture(Graphics::Texture *t, unsigned int index = 0);
	void ClearDecal(unsigned int index = 0);
	void ClearDecals();
	void SetLabel(const std::string&);

	//for modelviewer, at least
	bool SupportsDecals();
	bool SupportsPatterns();

	Animation *FindAnimation(const std::string&); //0 if not found
	const std::vector<Animation *> GetAnimations() const { return m_animations; }
	void UpdateAnimations();

	Graphics::Renderer *GetRenderer() const { return m_renderer; }

	//special for ship model use
	void SetThrust(const vector3f& linear, const vector3f &angular);

	void Save(Serializer::Writer &wr) const;
	void Load(Serializer::Reader &rd);

private:
	Model(const Model&);
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
	TagContainer m_tags; //named attachment points
	RenderData m_renderData;

	//per-instance flavour data
	Graphics::Texture *m_curPattern;
	Graphics::Texture *m_curDecals[MAX_DECAL_MATERIALS];
};

}

#endif
