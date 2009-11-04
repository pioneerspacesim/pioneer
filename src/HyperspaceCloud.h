#ifndef _HYPERSPACECLOUD_H
#define _HYPERSPACECLOUD_H

#include "Body.h"

class Frame;
class Ship;

class HyperspaceCloud: public Body {
public:
	OBJDEF(HyperspaceCloud, Body, HYPERSPACECLOUD);
	HyperspaceCloud(Ship *, double dateDue);
	HyperspaceCloud();
	virtual ~HyperspaceCloud();
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition() const;
	virtual double GetRadius() const { return 10.0; }
	virtual void Render(const Frame *camFrame);
	virtual void PostLoadFixup();
	Ship *GetShip() { return m_ship; }
	double GetDueDate() const { return m_due; }
protected:
	virtual void Save();
	virtual void Load();
private:
	Ship *m_ship;
	vector3d m_pos;
	double m_birthdate;
	double m_due;
};

#endif /* _HYPERSPACECLOUD_H */
