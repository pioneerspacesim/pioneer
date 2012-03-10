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
	virtual void OnSwitchTo() {}
private:
	void OnClickGalacticView();
	void PutLabels(vector3d offset);
	void MouseButtonDown(int button, int x, int y);
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::Label *m_scaleReadout;
	Gui::LabelSet *m_labels;
	float m_zoom;
	Gui::TexturedQuad m_quad;
	sigc::connection m_onMouseButtonDown;
};

#endif /* _GALACTICVIEW_H */
