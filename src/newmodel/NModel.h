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
 * Animation: keyframe animation affecting MatrixTransforms, and subsequently nodes
 * attached to them. Animations can be played by name, and play commands should propagate
 * to submodels (e.g. "anim_shoot" would activate recoil animation on gun submodels). 
 * There could be some generic animation related sigc events.
 * Animation challenges:
 *  - Assimp & several formats (.dae, .x) support multiple animations per file, but the
 *    Blender exporters don't (because they suck). Might have to ask users to supply all animations
 *    in one file and define animation names & ranges in the .model.
 * 	- Skeletal animation. I guess this will have to wait. Anyway, Bones are hierarchical
 *  MatrixTransforms. Skinning will be software-based, because it must work with Legacy renderer. 
 * 	- Combining & blending animations. Might just cut corners there, spaceships shouldn't be that animated...
 *  - Animating other properties than pos/rot (material props for example)
 * 
 * Attaching models to other models (guns etc. to ships): models may specify a nunber of
 * named hardpoints, known as "tags" (term from Q3). Users can query tags by name or index.
 * Space stations might be "Scenes" consisting of multiple models. And lights and stuff.
 */
#include "libs.h"
#include "Model.h"
#include "Group.h"
#include "ColorMap.h"
#include "Animation.h"
#include "graphics/Material.h"
#include <stdexcept>

namespace Graphics { class Renderer; }

namespace Newmodel
{

struct LoadingError : public std::runtime_error {
	LoadingError() : std::runtime_error("NewModel::LoadingError") { }
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
	//Render begins the graph traversal. Only geometry nodes actually render something.
	//might be worthwhile to implement Visitors (DrawVisitor, CreateCollisionMeshVisitor)
	void Render(Graphics::Renderer *r, const matrix4x4f &trans, const LmrObjParams *params);
	CollMesh *CreateCollisionMesh(const LmrObjParams *p);
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

	void UpdateAnimations(double time); //change this to timestep or something
	int PlayAnimation(const std::string &name); //immediately play an animation (forward), if found, returns count of animations triggered
	void StopAnimations(); //stop all animation
	const std::vector<Animation *> GetAnimations() const { return m_animations; }

private:
	ColorMap m_colorMap;
	float m_boundingRadius;
	double m_animTime;
	MaterialContainer m_materials; //materials are shared throughout the model graph
	PatternContainer m_patterns;
	RefCountedPtr<Group> m_root;
	RenderData *m_renderData;
	std::string m_name;
	std::vector<Group *> m_tags; //named attachment points
	std::vector<Animation *> m_animations;
};

typedef std::vector<Group *> TagContainer;

}

#endif
