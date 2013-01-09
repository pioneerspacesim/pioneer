// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OBJECTVIEWERVIEW_H
#define _OBJECTVIEWERVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "UIView.h"

#if WITH_OBJECTVIEWER

class Body;

class ObjectViewerView: public UIView {
public:
	ObjectViewerView();
	virtual void Update();
	virtual void Draw3D();
protected:
	virtual void OnSwitchTo();
private:
	float viewingDist;
	Gui::Label *m_infoLabel;
	Gui::VBox *m_vbox;
	const Body* lastTarget;
	matrix4x4d m_camRot;

	Gui::TextEntry *m_sbodyMass;
	Gui::TextEntry *m_sbodyRadius;
	Gui::TextEntry *m_sbodySeed;
	Gui::TextEntry *m_sbodyVolatileGas;
	Gui::TextEntry *m_sbodyVolatileLiquid;
	Gui::TextEntry *m_sbodyVolatileIces;
	Gui::TextEntry *m_sbodyLife;
	Gui::TextEntry *m_sbodyVolcanicity;
	Gui::TextEntry *m_sbodyMetallicity;
	void OnChangeTerrain();
	void OnRandomSeed();
	void OnNextSeed();
	void OnPrevSeed();
};

#endif

#endif /* _OBJECTVIEWERVIEW_H */
