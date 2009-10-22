#ifndef _CARGOBODY_H
#define _CARGOBODY_H

#include "libs.h"
#include "DynamicBody.h"
#include "sbre/sbre.h"
#include "EquipType.h"

class CargoBody: public DynamicBody {
public:
	OBJDEF(CargoBody, DynamicBody, CARGOBODY);
	CargoBody(Equip::Type t);
	CargoBody() {}
	virtual void Render(const Frame *camFrame);
	virtual bool OnDamage(Object *attacker, float kgDamage);
protected:
	virtual void Save();
	virtual void Load();
private:
	void Init();
	Equip::Type m_type;
	float m_hitpoints;
};

#endif /* _CARGOBODY_H */
