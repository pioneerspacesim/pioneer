#ifndef _ROCKETSHIPSPINNERELEMENT_H
#define _ROCKETSHIPSPINNERELEMENT_H

#include "RocketManager.h"

#include "ShipFlavour.h"
#include "LmrModel.h"

// <ship/>

class RocketShipSpinnerElement : public Rocket::Core::Element, public RocketStashConsumer<ShipFlavour> {
public:
	RocketShipSpinnerElement(const Rocket::Core::String &_tag) : Rocket::Core::Element(_tag), m_model(0), m_rotX(0), m_rotY(0) {}

	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions);
	virtual void OnRender();
	
	static void Register();

	virtual void UpdateFromStash(const ShipFlavour &flavour);
	
private:
	LmrModel *m_model;
	LmrObjParams m_params;

	// XXX m_equipment is currently not hooked up to anything,
	// it's just used to pass equipment parameters to the displayed model
	EquipSet m_equipment;

	float m_rotX, m_rotY;
};

#endif
