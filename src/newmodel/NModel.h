#ifndef _NEWMODEL_H
#define _NEWMODEL_H
/*
 * New internal model structure. Think of this as a mini-scenegraph.
 * For example:
 * RootNode
 *    MatrixTransformNode (applies a scale or something to child nodes)
 *        LodSwitchNode (picks 1-3)
 *            StaticGeometry_low
 *            StaticGeometry_med
 *            StaticGeometry_hi
 *
 * It's not supposed to be too complex. For example there are no "Material" nodes.
 * Geometry nodes can contain multiple separate meshes.
 *
 * Models are defined in a simple .model text file, which describes materials,
 * detail levels, meshes to import to each detail level and animations.
 *
 * Loading all the models can be quite slow, so there should be a system to
 * compile them into a more game-friendly binary format, which would be supplied
 * with game releases.
 *
 * Animation: keyframe animation affecting MatrixTransforms, and subsequently nodes
 * attached to them. Animations can be played by name, and play commands should propagate
 * to submodels (e.g. "anim_shoot" would activate recoil animation on gun submodels).
 * There could be some generic animation related sigc events.
 * Due to format & exporter limitations, animations need to be combined into one timeline
 * and then split into new animations using frame ranges.
 *
 * Attaching models to other models (guns etc. to ships): models may specify a nunber of
 * named hardpoints, known as "tags" (term from Q3). Users can query tags by name or index.
 * Space stations might be "Scenes" consisting of multiple models. And lights and stuff.
 * The actual logic to handle equipment attachments is to be done separately from new-model.
 *
 * Minor features:
 *  - pattern + customizable colour system (one pattern per model). Patterns can be
 *    dropped into the model directory.
 *  - dynamic textures (logos on spaceships, advertisements on stations)
 *
 * Other stuff to do:
 * 	- Collisions are very limited. Not entirely in scope of new-model, but someone's got to do it
 *  - scenegraph optimizer. Group untranslated geometry nodes into one etc.
 */
#include "libs.h"
#include "Model.h"
#include "Animation.h"
#include "ColorMap.h"
#include "Group.h"
#include "Label3D.h"
#include "graphics/Material.h"
#include <stdexcept>

namespace Graphics { class Renderer; }

namespace Newmodel
{

struct LoadingError : public std::runtime_error {
	LoadingError(const std::string &str) : std::runtime_error(str.c_str()) { }
};

typedef std::vector<std::pair<std::string, RefCountedPtr<Graphics::Material> > > MaterialContainer;

struct Pattern {
	std::string name;
	Graphics::Texture *texture;
	Pattern() : name(""), texture(0) { }
	Pattern(const std::string &n, Graphics::Texture *t) : name(n), texture(t) { }
};
typedef std::vector<Pattern> PatternContainer;

class NModel : public Model
{
public:
	friend class Loader;
	NModel(const std::string &name);
	~NModel();
	float GetDrawClipRadius() const { return m_boundingRadius; }
	void Render(Graphics::Renderer *r, const matrix4x4f &trans, LmrObjParams *params);
	RefCountedPtr<CollMesh> CreateCollisionMesh(const LmrObjParams *p);
	CollMesh *GetCollisionMesh() const { return m_collMesh.Get(); }
	RefCountedPtr<Group> GetRoot() { return m_root; }
	//materials used in the nodes should be accessible from here for convenience
	RefCountedPtr<Graphics::Material> GetMaterialByName(const std::string &name) const;
	RefCountedPtr<Graphics::Material> GetMaterialByIndex(int) const;

	//XXX these ignore possible ModelNodes
	int GetNumTags() const { return m_tags.size(); }
	Group * const GetTagByIndex(unsigned int index) const;
	Group * const FindTagByName(const std::string &name) const;
	void AddTag(const std::string &name, Group *node);

	void SetRenderData(RenderData *d) { m_renderData = d; }
	const PatternContainer &GetPatterns() const { return m_patterns; }
	void SetPattern(unsigned int index);
	void SetColors(Graphics::Renderer *r, const std::vector<Color4ub> &colors); //renderer needed for texture creation
	void SetDecalTexture(Graphics::Texture *t, unsigned int index = 0);
	void SetLabel(const std::string&);

	void UpdateAnimations(double time); //change this to timestep or something
	int PlayAnimation(const std::string &name, Animation::Direction = Animation::FORWARD); //immediately play an animation (forward), if found, returns count of animations triggered
	void StopAnimations(); //stop all animation
	const std::vector<Animation *> GetAnimations() const { return m_animations; }

private:
	static const unsigned int MAX_DECAL_MATERIALS = 4;
	ColorMap m_colorMap;
	double m_lastTime;
	float m_boundingRadius;
	MaterialContainer m_materials; //materials are shared throughout the model graph
	PatternContainer m_patterns;
	RefCountedPtr<CollMesh> m_collMesh;
	RefCountedPtr<Graphics::Material> m_decalMaterials[MAX_DECAL_MATERIALS]; //spaceship insignia, advertising billboards
	RefCountedPtr<Group> m_root;
	RenderData *m_renderData;
	std::string m_name;
	std::vector<Animation *> m_activeAnimations;
	std::vector<Animation *> m_animations;
	std::vector<Group *> m_tags; //named attachment points
};

typedef std::vector<Group *> TagContainer;

}

#endif
