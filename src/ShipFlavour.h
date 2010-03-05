#ifndef _SHIPFLAVOUR_H
#define _SHIPFLAVOUR_H

#include "ShipType.h"
#include "LmrModel.h"

struct LmrObjParams;

class ShipFlavour {
public:
	ShipType::Type type;
	char regid[16];
	int price;
	LmrMaterial primaryColor;
	LmrMaterial secondaryColor;

	ShipFlavour();
	ShipFlavour(ShipType::Type type);
	void Save();
	void Load();
	void ApplyTo(LmrObjParams *p) const;
	static void MakeTrulyRandom(ShipFlavour &v);
private:
	void SaveLmrMaterial(LmrMaterial *m);
	void LoadLmrMaterial(LmrMaterial *m);
	void MakeRandomColor(LmrMaterial &m);
};


#endif /* _SHIPFLAVOUR_H */
