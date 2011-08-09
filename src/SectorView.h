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
	vector3f GetPosition() const { return m_pos; }
	SystemPath GetSelectedSystem() const { return m_selected; }
	SystemPath GetHyperspaceTarget() const { return m_hyperspaceTarget; }
	void SetHyperspaceTarget(const SystemPath &path);
	void FloatHyperspaceTarget();
	void ResetHyperspaceTarget();
	void GotoSystem(const SystemPath &path);
	void WarpToSystem(const SystemPath &path);
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
	virtual void OnSwitchTo();

	sigc::signal<void> onHyperspaceTargetChanged;
private:
	void DrawSector(int x, int y, int z, const vector3f &playerAbsPos);
	void PutClickableLabel(const std::string &text, const Color &labelCol, const SystemPath &path);
	void OnClickSystem(const SystemPath &path);

	void MouseButtonDown(int button, int x, int y);
	Sector* GetCached(int sectorX, int sectorY, int sectorZ);
	void OnKeyPress(SDL_keysym *keysym);
	void OnSearchBoxValueChanged();
	void ShrinkCache();

	float m_zoom;

	bool m_firstTime;
	SystemPath m_selected;

	vector3f m_pos;
	vector3f m_posMovingTo;

	float m_rotX, m_rotZ;
	float m_rotXMovingTo, m_rotZMovingTo;

	SystemPath m_hyperspaceTarget;
	bool m_matchTargetToSelection;

	Gui::Label *m_infoLabel;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::ImageButton *m_galaxyButton;
	Gui::TextEntry *m_searchBox;
	GLuint m_gluDiskDlist;
	
	Gui::Label *m_systemName;
	Gui::Label *m_distance;
	Gui::Label *m_starType;
	Gui::Label *m_shortDesc;
	Gui::LabelSet *m_clickableLabels;

	sigc::connection m_onMouseButtonDown;
	sigc::connection m_onKeyPressConnection;

	std::map<SystemPath,Sector*> m_sectorCache;
};

#endif /* _SECTORVIEW_H */
