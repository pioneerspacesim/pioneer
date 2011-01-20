#ifndef _GALACTICVIEW_H
#define _GALACTICVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"
#include <vector>
#include <string>
#include "GenericSystemView.h"

class GalacticView: public GenericSystemView {
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
	float m_zoom;
	GLuint m_texture;
	sigc::connection m_onMouseButtonDown;
};

#endif /* _GALACTICVIEW_H */
