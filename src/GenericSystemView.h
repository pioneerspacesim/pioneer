#ifndef _GENERICSYSTEMVIEW_H
#define _GENERICSYSTEMVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"
#include "StarSystem.h"

class SystemInfoScannerText;

class GenericSystemView: public View {
public:
	GenericSystemView();
	virtual void Draw3D();
	virtual void ShowAll();
	virtual void HideAll();
	sigc::signal<void,StarSystem*> onSelectedSystemChanged;
private:
	Gui::Fixed *m_scannerLayout;
	Gui::Label *m_systemName;
	Gui::Label *m_distance;
	Gui::Label *m_starType;
	Gui::Label *m_shortDesc;
	int px, py, pidx;
};


#endif /* _GENERICSYSTEMVIEW_H */
