#ifndef _SECTORVIEW_H
#define _SECTORVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include <vector>
#include <string>
#include "View.h"
#include "Sector.h"
#include "SystemPath.h"

class SectorView: public View {
public:
	SectorView();
	virtual ~SectorView();
	virtual void Update();
	virtual void Draw3D();
	bool GetSelectedSystem(SystemPath *path);
	void GotoSystem(int sector_x, int sector_y, int system_idx);
	void GetSector(int *outSecX, int *outSecY) const { *outSecX = m_secx; *outSecY = m_secy; }
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
	virtual void OnSwitchTo();
private:
	void DrawSector(int x, int y);
	void PutClickableLabel(std::string &text, int sx, int sy, int sys_idx);
	void OnClickSystem(int sx, int sy, int sys_idx);
	void MouseButtonDown(int button, int x, int y);
	Sector* GetCached(int sectorX, int sectorY);
	void ShrinkCache();

	float m_zoom;
	int m_secx, m_secy;
	int m_selected;
	float m_px, m_py;
	float m_rot_x, m_rot_z;
	float m_pxMovingTo, m_pyMovingTo;
	Gui::Label *m_infoLabel;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::ImageButton *m_galaxyButton;
	GLuint m_gluDiskDlist;
	
	SystemPath m_lastShown;
	Gui::Label *m_systemName;
	Gui::Label *m_distance;
	Gui::Label *m_starType;
	Gui::Label *m_shortDesc;
	Gui::LabelSet *m_clickableLabels;
	sigc::connection m_onMouseButtonDown;

	std::map<SystemPath,Sector*> m_sectorCache;
};

#endif /* _SECTORVIEW_H */
