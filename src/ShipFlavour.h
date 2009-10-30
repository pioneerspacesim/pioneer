#ifndef _SHIPFLAVOUR_H
#define _SHIPFLAVOUR_H

#include "ShipType.h"
#include "sbre/sbre.h"

struct ObjParams;

class ShipFlavour {
public:
	ShipType::Type type;
	char regid[16];
	int price;
	// struct Material: sbre.h
	Material primaryColor;
	Material secondaryColor;

	ShipFlavour();
	ShipFlavour(ShipType::Type type);
	void Save();
	void Load();
	void ApplyTo(ObjParams *p) const;
	static void MakeTrulyRandom(ShipFlavour &v);
private:
	void MakeRandomColor(Material &m);
};


#endif /* _SHIPFLAVOUR_H */
