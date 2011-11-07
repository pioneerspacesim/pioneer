#ifndef _CAMERAELEMENT_H
#define _CAMERAELEMENT_H

#include "ui/UIManager.h"

// <camera/>

class Body;
class Camera;

struct CameraElementData {
	CameraElementData() : body(0), pos(0.0), orient(matrix4x4d::Identity()) {}
	Body       *body;
	vector3d   pos;
	matrix4x4d orient;
};

class CameraElement : public Rocket::Core::Element, public UI::StashConsumer<CameraElementData> {
public:
	CameraElement(const Rocket::Core::String &_tag) : Rocket::Core::Element(_tag), m_camera(0) {}
    ~CameraElement();

	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions);

	virtual void OnUpdate();
	virtual void OnRender();

	virtual void UpdateFromStash(const CameraElementData &camera);

private:
    Camera *m_camera;
};

#endif
