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
	void GotoSystem(const SystemPath &path);
	void GotoCurrentSystem() { GotoSystem(m_current); }
	void GotoSelectedSystem() { GotoSystem(m_selected); }
	void GotoHyperspaceTarget() { GotoSystem(m_hyperspaceTarget); }
	virtual void Save(Serializer::Writer &wr);
	virtual void OnSwitchTo();

	sigc::signal<void> onHyperspaceTargetChanged;

private:
	void InitDefaults();
	void InitObject();

	struct SystemLabels {
		Gui::Label *systemName;
		Gui::Label *distance;
		Gui::Label *starType;
		Gui::Label *shortDesc;
	};
	
	void DrawSector(int x, int y, int z, const vector3f &playerAbsPos, const matrix4x4f &trans);
	void PutClickableLabel(const std::string &text, const Color &labelCol, const SystemPath &path);

	void SetSelectedSystem(const SystemPath &path);
	void OnClickSystem(const SystemPath &path);

	void UpdateSystemLabels(SystemLabels &labels, const SystemPath &path);

	void UpdateHyperspaceLockLabel();

	Sector* GetCached(int sectorX, int sectorY, int sectorZ);
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
	float m_zoomMovingTo;

	SystemPath m_hyperspaceTarget;
	bool m_matchTargetToSelection;

	bool m_selectionFollowsMovement;

	Gui::Label *m_sectorLabel;
	Gui::Label *m_distanceLabel;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::ImageButton *m_galaxyButton;
	Gui::TextEntry *m_searchBox;
	Graphics::VertexArray *m_disk;
	
	Gui::LabelSet *m_clickableLabels;

	Gui::VBox *m_infoBox;
	bool m_infoBoxVisible;
	
	SystemLabels m_currentSystemLabels;
	SystemLabels m_selectedSystemLabels;
	SystemLabels m_targetSystemLabels;

	Gui::Label *m_hyperspaceLockLabel;

	sigc::connection m_onMouseButtonDown;
	sigc::connection m_onKeyPressConnection;

	std::map<SystemPath,Sector*> m_sectorCache;

	float m_playerHyperspaceRange;
	Graphics::Drawables::Line3D m_jumpLine;
};

#endif /* _SECTORVIEW_H */
