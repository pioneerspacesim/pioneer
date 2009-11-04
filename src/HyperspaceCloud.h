#ifndef _HYPERSPACECLOUD_H
#define _HYPERSPACECLOUD_H

#include "Body.h"

class Frame;
class Ship;

class HyperspaceCloud: public Body {
public:
	OBJDEF(HyperspaceCloud, Body, HYPERSPACECLOUD);
	HyperspaceCloud(Ship *, double dateDue, bool isArrival);
	HyperspaceCloud();
	virtual ~HyperspaceCloud();
	virtual void SetPosition(vector3d p);
	virtual void SetVelocity(vector3d v) { m_vel = v; }
	virtual vector3d GetPosition() const;
	virtual double GetRadius() const { return 10.0; }
	virtual void Render(const Frame *camFrame);
	virtual void PostLoadFixup();
	virtual void TimeStepUpdate(const float timeStep);
	Ship *GetShip() { return m_ship; }
	double GetDueDate() const { return m_due; }
	void SetIsArrival(bool isArrival) { m_isArrival = isArrival; }
	bool IsArrival() const { return m_isArrival; }
protected:
	virtual void Save();
	virtual void Load();
private:
	Ship *m_ship;
	vector3d m_pos;
	vector3d m_vel;
	double m_birthdate;
	double m_due;
	bool m_isArrival;
};

#endif /* _HYPERSPACECLOUD_H */
