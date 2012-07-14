#ifndef _SHIPSPINNERWIDGET_H
#define _SHIPSPINNERWIDGET_H

#include "gui/Gui.h"
#include "ShipFlavour.h"
#include "LmrModel.h"
#include "ShipType.h"
#include "Light.h"
#include "EquipSet.h"

class ShipSpinnerWidget : public Gui::Widget {
public:
	ShipSpinnerWidget(const ShipFlavour &flavour, float width, float height);

	virtual void Draw();
	virtual void GetSizeRequested(float size[2]) { size[0] = m_width; size[1] = m_height; }

private:
	float m_width;
	float m_height;

	LmrModel *m_model;
	LmrObjParams m_params;
	// XXX m_equipment is currently not hooked up to anything,
	// it's just used to pass equipment parameters to the displayed model
	EquipSet m_equipment;
	//illumination in ship view
	Light m_light;
};

#endif
