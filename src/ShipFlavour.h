#ifndef _SHIPFLAVOUR_H
#define _SHIPFLAVOUR_H

#include "ShipType.h"

struct ObjParams;

class ShipFlavour {
public:
	ShipType::Type type;
	char regid[16];
	int price;

	ShipFlavour();
	ShipFlavour(ShipType::Type type);
	void Save();
	void Load();
	void ApplyTo(ObjParams *p) const;
	static void MakeTrulyRandom(ShipFlavour &v);
};


#endif /* _SHIPFLAVOUR_H */
