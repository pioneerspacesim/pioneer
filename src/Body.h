#ifndef _BODY_H
#define _BODY_H

#include "vector3.h"
#include "matrix4x4.h"
#include "Object.h"
#include <string>

class Frame;
class ObjMesh;

class Body: public Object {
public:
	Body();
	virtual ~Body() {};
	virtual Object::Type GetType() { return Object::BODY; }
	virtual void SetPosition(vector3d p) = 0;
	virtual vector3d GetPosition() = 0; // within frame
	vector3d GetPositionRelTo(const Frame *);
	virtual void Render(const Frame *camFrame) = 0;
	virtual void TransformToModelCoords(const Frame *camFrame) = 0;
	virtual void TransformCameraTo() = 0;
	virtual void SetFrame(Frame *f) { m_frame = f; }
	Frame *GetFrame() { return m_frame; }
	void SetLabel(const char *label) { m_label = label; }
	std::string &GetLabel() { return m_label; }
	unsigned int GetFlags() { return m_flags; }
	// return true if to apply damage
	virtual bool OnCollision(Body *b) { return false; }
	void SetProjectedPos(const vector3d& projectedPos) { m_projectedPos = projectedPos; }
	// Only valid if IsOnscreen() is true.
	const vector3d& GetProjectedPos() const;
	bool IsOnscreen() const { return m_onscreen; }
	void SetOnscreen(const bool onscreen) { m_onscreen = onscreen; }
	virtual void TimeStepUpdate(const float timeStep) {}


	enum { FLAG_CAN_MOVE_FRAME = 1 };
protected:
	unsigned int m_flags;
private:
	// frame of reference
	Frame *m_frame;
	std::string m_label;
	bool m_onscreen;
	vector3d m_projectedPos;
};

#endif /* _BODY_H */
