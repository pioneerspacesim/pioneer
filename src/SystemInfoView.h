#ifndef _SYSTEMINFOVIEW_H
#define _SYSTEMINFOVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "View.h"
#include <vector>

class StarSystem;
class SystemBody;
namespace Graphics { class Renderer; }

class SystemInfoView: public View {
public:
	SystemInfoView();
	virtual void Update();
	virtual void Draw3D();
	virtual void OnSwitchTo();
	void NextPage();
private:
	class BodyIcon : public Gui::ImageRadioButton {
	public:
		BodyIcon(const char* img);
		virtual void Draw();
		virtual void OnActivate();
		void SetRenderer(Graphics::Renderer *r) { m_renderer = r; }
	private:
		Graphics::Renderer *m_renderer;
	};
	void SystemChanged(const SystemPath &path);
	void UpdateEconomyTab();
	void OnBodyViewed(SystemBody *b);
	void OnBodySelected(SystemBody *b);
	void OnClickBackground(Gui::MouseButtonEvent *e);
	void PutBodies(SystemBody *body, Gui::Fixed *container, int dir, float pos[2], int &majorBodies, int &starports, float &prevSize);
	void UpdateIconSelections();
	Gui::VBox *m_infoBox;
	Gui::Label *m_econInfo;
	Gui::Label *m_econMajImport, *m_econMinImport;
	Gui::Label *m_econMajExport, *m_econMinExport;
	Gui::Label *m_econIllegal;
	Gui::Fixed *m_sbodyInfoTab, *m_econInfoTab;
	Gui::Tabbed *m_tabs;
	RefCountedPtr<StarSystem> m_system;
	bool m_refresh;
	//map is not enough to associate icons as each tab has their own
	std::vector<std::pair<std::string, BodyIcon*> > m_bodyIcons;
};

#endif /* _SYSTEMINFOVIEW_H */
