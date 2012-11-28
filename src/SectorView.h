// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SECTORVIEW_H
#define _SECTORVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include <vector>
#include <set>
#include <string>
#include "View.h"
#include "galaxy/Sector.h"
#include "galaxy/SystemPath.h"
#include "graphics/Drawables.h"

class SectorView: public View {
public:
	SectorView();
	SectorView(Serializer::Reader &rd);
	virtual ~SectorView();

	virtual void Update();
	virtual void ShowAll();
	virtual void Draw3D();
	vector3f GetPosition() const { return m_pos; }
	SystemPath GetSelectedSystem() const { return m_selected; }
	SystemPath GetHyperspaceTarget() const { return m_hyperspaceTarget; }
	void SetHyperspaceTarget(const SystemPath &path);
	void FloatHyperspaceTarget();
	void ResetHyperspaceTarget();
	void GotoSector(const SystemPath &path);
	void GotoSystem(const SystemPath &path);
	void GotoCurrentSystem() { GotoSystem(m_current); }
	void GotoSelectedSystem() { GotoSystem(m_selected); }
	void GotoHyperspaceTarget() { GotoSystem(m_hyperspaceTarget); }
	virtual void Save(Serializer::Writer &wr);

	sigc::signal<void> onHyperspaceTargetChanged;

protected:
	virtual void OnSwitchTo();
private:
	void InitDefaults();
	void InitObject();

	struct SystemLabels {
		Gui::Label *systemName;
		Gui::Label *distance;
		Gui::Label *starType;
		Gui::Label *shortDesc;
	};

	void DrawNearSectors(matrix4x4f modelview);
	void DrawNearSector(int x, int y, int z, const vector3f &playerAbsPos, const matrix4x4f &trans);
	void PutSystemLabels(Sector *sec, const vector3f &origin, int drawRadius);

	void DrawFarSectors(matrix4x4f modelview);
	void BuildFarSector(Sector *sec, const vector3f &origin, std::vector<vector3f> &points, std::vector<Color> &colors);
	void PutFactionLabels(const vector3f &secPos);

	void SetSelectedSystem(const SystemPath &path);
	void OnClickSystem(const SystemPath &path);

	void UpdateSystemLabels(SystemLabels &labels, const SystemPath &path);
	void UpdateFactionToggles();
	void RefreshDetailBoxVisibility();

	void UpdateHyperspaceLockLabel();

	Sector* GetCached(const SystemPath& loc);
	Sector* GetCached(const int sectorX, const int sectorY, const int sectorZ);
	void ShrinkCache();

	void MouseButtonDown(int button, int x, int y);
	void OnKeyPressed(SDL_keysym *keysym);
	void OnSearchBoxKeyPress(const SDL_keysym *keysym);

	bool m_inSystem;

	SystemPath m_current;
	SystemPath m_selected;

	vector3f m_pos;
	vector3f m_posMovingTo;

	float m_rotXDefault, m_rotZDefault, m_zoomDefault;

	float m_rotX, m_rotZ;
	float m_rotXMovingTo, m_rotZMovingTo;

	float m_zoom;
	float m_zoomClamped;
	float m_zoomMovingTo;

	SystemPath m_hyperspaceTarget;
	bool m_matchTargetToSelection;

	bool m_selectionFollowsMovement;

	Gui::Label *m_sectorLabel;
	Gui::Label *m_distanceLabel;
	Gui::Label *m_zoomLevelLabel;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::ImageButton *m_galaxyButton;
	Gui::TextEntry *m_searchBox;

	ScopedPtr<Graphics::Drawables::Disk> m_disk;

	Gui::LabelSet *m_clickableLabels;

	Gui::VBox *m_infoBox;

	SystemLabels m_currentSystemLabels;
	SystemLabels m_selectedSystemLabels;
	SystemLabels m_targetSystemLabels;
	Gui::Label *m_hyperspaceLockLabel;

	Gui::VBox *m_factionBox;
	std::set<Faction*>              m_visibleFactions;
	std::set<Faction*>              m_hiddenFactions;
	std::vector<Gui::Label*>        m_visibleFactionLabels;
	std::vector<Gui::HBox*>         m_visibleFactionRows;
	std::vector<Gui::ToggleButton*> m_visibleFactionToggles;

	Uint8 m_detailBoxVisible;

	void OnToggleFaction(Gui::ToggleButton* button, bool pressed, Faction* faction);

	sigc::connection m_onMouseButtonDown;
	sigc::connection m_onKeyPressConnection;

	std::map<SystemPath,Sector*> m_sectorCache;
	std::string m_previousSearch;

	float m_playerHyperspaceRange;
	Graphics::Drawables::Line3D m_jumpLine;

	RefCountedPtr<Graphics::Material> m_material;

	std::vector<vector3f> m_farstars;
	std::vector<Color>    m_farstarsColor;

	vector3f m_secPosFar;
	int      m_radiusFar;
	bool     m_toggledFaction;

	int m_cacheXMin;
	int m_cacheXMax;
	int m_cacheYMin;
	int m_cacheYMax;
	int m_cacheZMin;
	int m_cacheZMax;

};

#endif /* _SECTORVIEW_H */
