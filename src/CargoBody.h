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
protected:
	virtual void Save();
	virtual void Load();
private:
	void Init();
	Equip::Type m_type;
};

#endif /* _CARGOBODY_H */
