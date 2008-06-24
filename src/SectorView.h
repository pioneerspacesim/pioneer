#ifndef _SECTORVIEW_H
#define _SECTORVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"
#include <vector>
#include <string>
#include "GenericSystemView.h"

class SectorView: public GenericSystemView {
public:
	SectorView();
	virtual ~SectorView();
	virtual void Update();
	virtual void Draw3D();
	bool GetSelectedSystem(int *sector_x, int *sector_y, int *system_idx);
private:
	void DrawSector(int x, int y);
	void PutText(std::string &text);
	void OnClickSystemInfo();

	float m_zoom;
	int m_secx, m_secy;
	int m_selected;
	float m_px, m_py;
	float m_rot_x, m_rot_z;
	Gui::Label *m_infoLabel;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	GLuint m_gluSphereDlist, m_gluDiskDlist;
};

#endif /* _SECTORVIEW_H */
