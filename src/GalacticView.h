// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GALACTICVIEW_H
#define _GALACTICVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include <vector>
#include <string>
#include "View.h"

class GalacticView: public View {
public:
	GalacticView();
	virtual ~GalacticView();
	virtual void Update();
	virtual void Draw3D();
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
protected:
	virtual void OnSwitchTo() {}
private:
	void OnClickGalacticView();
	void PutLabels(vector3d offset);
	void MouseWheel(bool up);
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::Label *m_scaleReadout;
	Gui::LabelSet *m_labels;
	float m_zoom, m_zoomTo;
	Gui::TexturedQuad m_quad;
	sigc::connection m_onMouseWheelCon;
};

#endif /* _GALACTICVIEW_H */
