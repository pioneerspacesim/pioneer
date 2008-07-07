#ifndef _SPACE_H
#define _SPACE_H

#include <list>
#include "vector3.h"
#include "StarSystem.h"

class Body;
class Frame;

// The place all the 'Body's exist in
class Space {
public:
	static void Init();
	static void Clear();
	static void BuildSystem(StarSystem *s);
	static void GenBody(StarSystem *s, StarSystem::SBody *b, Frame *f);
	static void TimeStep(float step);
	static void AddBody(Body *);
	static void KillBody(Body *);
	static void Render(const Frame *cam_frame);
	static Frame *GetRootFrame() { return rootFrame; }

	static dWorldID world;
	static std::list<Body*> bodies;
	typedef std::list<Body*>::iterator bodiesIter_t;
private:
	static void UpdateFramesOfReference();
	static void CollideFrame(Frame *f);
	static void PruneCorpses();

	static Frame *rootFrame;
//	static std::list<Frame*> rootFrames;
	static std::list<Body*> corpses;
};


#endif /* _SPACE_H */
