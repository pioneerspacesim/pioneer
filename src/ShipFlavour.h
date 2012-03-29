#ifndef _SHIPFLAVOUR_H
#define _SHIPFLAVOUR_H

#include "ShipType.h"
#include "LmrModel.h"
#include "Serializer.h"

struct LmrObjParams;

class ShipFlavour {
public:
	ShipType::Type type;
	std::string regid;
	int price;
	LmrMaterial primaryColor;
	LmrMaterial secondaryColor;

	ShipFlavour();
	ShipFlavour(ShipType::Type type);
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);
	void ApplyTo(LmrObjParams *p) const;
	static void MakeTrulyRandom(ShipFlavour &v);
private:
	void SaveLmrMaterial(Serializer::Writer &wr, LmrMaterial *m);
	void LoadLmrMaterial(Serializer::Reader &rd, LmrMaterial *m);
	void MakeRandomColor(LmrMaterial &m);
};


#endif /* _SHIPFLAVOUR_H */
