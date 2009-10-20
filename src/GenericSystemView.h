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
	static void SwitchToCurrentView();
private:
	static void OnClickView(enum MapView v);
	static enum MapView currentView;
};


#endif /* _GENERICSYSTEMVIEW_H */
