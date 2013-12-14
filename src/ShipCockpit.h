// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIP_COCKPIT_H_
#define _SHIP_COCKPIT_H_

#include "libs.h"
#include "ModelBody.h"

struct ShipType;

class ShipCockpit : public ModelBody
{
public:
	explicit ShipCockpit(const ShipType& ship_type);
	virtual ~ShipCockpit();

	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;

protected:
	void Init();

private:
	ShipCockpit(const ShipCockpit&);
	ShipCockpit& operator=(const ShipCockpit&);
	
	ShipType m_type;
};

#endif