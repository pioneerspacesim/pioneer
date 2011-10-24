#ifndef _ROCKETCAMERAELEMENT_H
#define _ROCKETCAMERAELEMENT_H

#include "RocketManager.h"

#include "Body.h"
#include "vector3.h"
#include "matrix4x4.h"

#include "Background.h"

// <camera/>

class Body;
class Frame;

struct RocketCamera {
	RocketCamera() : body(0), pos(0.0), orient(matrix4x4d::Identity()) {}
	const Body *body;
	vector3d    pos;
	matrix4x4d  orient;
};

class RocketCameraElement : public Rocket::Core::Element, public RocketStashConsumer<RocketCamera> {
public:
	RocketCameraElement(const Rocket::Core::String &_tag) : Rocket::Core::Element(_tag), m_camFrame(0) {}

	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions);

	virtual void OnUpdate();
	virtual void OnRender();

	static void Register();

	virtual void UpdateFromStash(const RocketCamera &camera);

private:
	RocketCamera m_camera;

	Frame *m_camFrame;
};

#endif
