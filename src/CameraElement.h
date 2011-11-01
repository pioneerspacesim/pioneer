#ifndef _CAMERAELEMENT_H
#define _CAMERAELEMENT_H

#include "rocket/RocketManager.h"

// <camera/>

class Body;
class Camera;

struct CameraElementData {
	CameraElementData() : body(0), pos(0.0), orient(matrix4x4d::Identity()) {}
	const Body *body;
	vector3d    pos;
	matrix4x4d  orient;
};

class CameraElement : public Rocket::Core::Element, public RocketStashConsumer<CameraElementData> {
public:
	CameraElement(const Rocket::Core::String &_tag) : Rocket::Core::Element(_tag), m_camera(0) {}
    ~CameraElement();

	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions);

	virtual void OnUpdate();
	virtual void OnRender();

	static void Register();

	virtual void UpdateFromStash(const CameraElementData &camera);

private:
    Camera *m_camera;
};

#endif
