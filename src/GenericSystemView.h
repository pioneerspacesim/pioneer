#ifndef _GENERICSYSTEMVIEW_H
#define _GENERICSYSTEMVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"
#include "StarSystem.h"

class SystemInfoScannerText;

class GenericSystemView: public View {
public:
	enum MapView { MAP_SECTOR, MAP_SYSTEM, MAP_INFO, MAP_GALACTIC };
	GenericSystemView(enum MapView whichView);
	virtual void Draw3D();
	virtual void ShowAll();
	virtual void HideAll();
	sigc::signal<void,StarSystem*> onSelectedSystemChanged;
	static void SwitchToCurrentView();
private:
	static void OnClickView(enum MapView v);
	static enum MapView currentView;
	Gui::Fixed *m_scannerLayout;
	Gui::Label *m_systemName;
	Gui::Label *m_distance;
	Gui::Label *m_starType;
	Gui::Label *m_shortDesc;
	int m_px, m_py, m_pidx;
};


#endif /* _GENERICSYSTEMVIEW_H */
