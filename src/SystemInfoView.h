// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SYSTEMINFOVIEW_H
#define _SYSTEMINFOVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "UIView.h"
#include <vector>

class Game;
class StarSystem;
class SystemBody;
namespace Graphics { class Renderer; }

class SystemInfoView: public UIView {
public:
	SystemInfoView(Game* game);
	virtual void Update();
	virtual void Draw3D();
	void NextPage();
protected:
	virtual void OnSwitchTo();
private:
	class BodyIcon : public Gui::ImageRadioButton {
	public:
		BodyIcon(const char* img, Graphics::Renderer*);
		virtual void Draw();
		virtual void OnActivate();
		bool HasStarport() { return m_hasStarport; }
		void SetHasStarport() { m_hasStarport = true; }
		void SetSelectColor(const Color& color) { m_selectColor = color; }
	private:
		Graphics::Renderer *m_renderer;
		Graphics::RenderState *m_renderState;
		bool m_hasStarport;
		Color m_selectColor;

	};

	enum RefreshType {
		REFRESH_NONE,
		REFRESH_SELECTED_BODY,
		REFRESH_ALL
	};

	RefreshType NeedsRefresh();
	void SystemChanged(const SystemPath &path);
	void UpdateEconomyTab();
	void OnBodyViewed(SystemBody *b);
	void OnBodySelected(SystemBody *b);
	void OnClickBackground(Gui::MouseButtonEvent *e);
	void PutBodies(SystemBody *body, Gui::Fixed *container, int dir, float pos[2], int &majorBodies, int &starports, int &onSurface, float &prevSize);
	void UpdateIconSelections();

	Game* m_game;

	Gui::VBox *m_infoBox;
	Gui::Fixed *m_econInfo;
	Gui::Fixed *m_econMajImport, *m_econMinImport;
	Gui::Fixed *m_econMajExport, *m_econMinExport;
	Gui::Fixed *m_econIllegal;
	Gui::Fixed *m_sbodyInfoTab, *m_econInfoTab;

	Gui::Label *m_commodityTradeLabel;
	Gui::Tabbed *m_tabs;
	RefCountedPtr<StarSystem> m_system;
	SystemPath m_selectedBodyPath;
	RefreshType m_refresh;
	//map is not enough to associate icons as each tab has their own. First element is the body index of SystemPath (names are not unique)
	std::vector<std::pair<Uint32, BodyIcon*> > m_bodyIcons;
	bool m_unexplored;

	Graphics::RenderState *m_solidState;
};

#endif /* _SYSTEMINFOVIEW_H */
