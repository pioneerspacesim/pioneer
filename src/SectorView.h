// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SECTORVIEW_H
#define _SECTORVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "UIView.h"
#include <vector>
#include <set>
#include <string>
#include "View.h"
#include "galaxy/Sector.h"
#include "galaxy/SystemPath.h"
#include "graphics/Drawables.h"
#include "graphics/RenderState.h"
#include <set>

class SectorView: public UIView {
public:
	SectorView();
	SectorView(Serializer::Reader &rd);
	virtual ~SectorView();

	virtual void Update();
	virtual void ShowAll();
	virtual void Draw3D();
	vector3f GetPosition() const { return m_pos; }
	SystemPath GetSelected() const { return m_selected; }
	void SetSelected(const SystemPath &path);
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

	struct DistanceIndicator {
		Gui::Label *label;
		Graphics::Drawables::Line3D *line;
		Color okayColor;
		Color unsuffFuelColor;
		Color outOfRangeColor;
	};

	struct SystemLabels {
		Gui::Label *systemName;
		Gui::Label *sector;
		DistanceIndicator distance;
		Gui::Label *starType;
		Gui::Label *shortDesc;
	};

	void DrawNearSectors(const matrix4x4f& modelview);
	void DrawNearSector(const int sx, const int sy, const int sz, const vector3f &playerAbsPos, const matrix4x4f &trans);
	void PutSystemLabels(RefCountedPtr<Sector> sec, const vector3f &origin, int drawRadius);

	void DrawFarSectors(const matrix4x4f& modelview);
	void BuildFarSector(RefCountedPtr<Sector> sec, const vector3f &origin, std::vector<vector3f> &points, std::vector<Color> &colors);
	void PutFactionLabels(const vector3f &secPos);
	void AddStarBillboard(const matrix4x4f &modelview, const vector3f &pos, const Color &col, float size);

	void OnClickSystem(const SystemPath &path);

	void UpdateDistanceLabelAndLine(DistanceIndicator &distance, const SystemPath &src, const SystemPath &dest);
	void UpdateSystemLabels(SystemLabels &labels, const SystemPath &path);
	void UpdateFactionToggles();
	void RefreshDetailBoxVisibility();

	void UpdateHyperspaceLockLabel();

	RefCountedPtr<Sector> GetCached(const SystemPath& loc) { return m_sectorCache->GetCached(loc); }
	void ShrinkCache();

	void MouseWheel(bool up);
	void OnKeyPressed(SDL_Keysym *keysym);
	void OnSearchBoxKeyPress(const SDL_Keysym *keysym);

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
	bool m_automaticSystemSelection;

	Gui::Label *m_sectorLabel;
	Gui::Label *m_distanceLabel;
	Gui::Label *m_zoomLevelLabel;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::ImageButton *m_galaxyButton;
	Gui::TextEntry *m_searchBox;
	Gui::Label *m_statusLabel;
	Gui::ToggleButton *m_drawOutRangeLabelButton;
	Gui::ToggleButton *m_drawUninhabitedLabelButton;
	Gui::ToggleButton *m_drawSystemLegButton;
	Gui::ToggleButton *m_automaticSystemSelectionButton;
	void OnAutomaticSystemSelectionChange(Gui::ToggleButton *b, bool pressed);

	std::unique_ptr<Graphics::Drawables::Disk> m_disk;

	Gui::LabelSet *m_clickableLabels;

	Gui::VBox *m_infoBox;

	SystemLabels m_currentSystemLabels;
	SystemLabels m_selectedSystemLabels;
	SystemLabels m_targetSystemLabels;
	DistanceIndicator m_secondDistance;
	Gui::Label *m_hyperspaceLockLabel;

	Gui::VBox *m_factionBox;
	std::set<const Faction*>        m_visibleFactions;
	std::set<const Faction*>        m_hiddenFactions;
	std::vector<Gui::Label*>        m_visibleFactionLabels;
	std::vector<Gui::HBox*>         m_visibleFactionRows;
	std::vector<Gui::ToggleButton*> m_visibleFactionToggles;

	Uint8 m_detailBoxVisible;

	void OnToggleFaction(Gui::ToggleButton* button, bool pressed, const Faction* faction);

	sigc::connection m_onMouseWheelCon;
	sigc::connection m_onKeyPressConnection;

	RefCountedPtr<SectorCache::Slave> m_sectorCache;
	std::string m_previousSearch;

	float m_playerHyperspaceRange;
	Graphics::Drawables::Line3D m_selectedLine;
	Graphics::Drawables::Line3D m_secondLine;
	Graphics::Drawables::Line3D m_jumpLine;

	Graphics::RenderState *m_solidState;
	Graphics::RenderState *m_alphaBlendState;
	Graphics::RenderState *m_jumpSphereState;
	RefCountedPtr<Graphics::Material> m_material; //flat colour
	RefCountedPtr<Graphics::Material> m_starMaterial;

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

	std::unique_ptr<Graphics::VertexArray> m_lineVerts;
	std::unique_ptr<Graphics::VertexArray> m_secLineVerts;
	std::unique_ptr<Graphics::Drawables::Sphere3D> m_jumpSphere;
	std::unique_ptr<Graphics::Drawables::Disk> m_jumpDisk;
	std::unique_ptr<Graphics::VertexArray> m_starVerts;
};

#endif /* _SECTORVIEW_H */
