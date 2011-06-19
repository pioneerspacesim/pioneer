#ifndef _GAMEMENUVIEW_H
#define _GAMEMENUVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "KeyBindings.h"

extern std::string GetFullSavefileDirPath();

class GameMenuView: public View {
public:
	GameMenuView();
	virtual void Update() {}
	virtual void Draw3D() {}
	virtual void OnSwitchTo();
	virtual void HideAll();
	void OpenLoadDialog();
	void OpenSaveDialog();
private:
	void OnChangeKeyBinding(const KeyBindings::KeyBinding &kb, const char *fnName);
	void OnChangeAxisBinding(const KeyBindings::AxisBinding &ab, const char *function);
	void OnChangeVolume();
	void OnChangePlanetDetail(int level);
	void OnChangeCityDetail(int level);
	void OnChangeVideoResolution(int res);
	void OnToggleShaders(Gui::ToggleButton *b, bool state);
	void OnToggleHDR(Gui::ToggleButton *b, bool state);
	void OnToggleFullscreen(Gui::ToggleButton *b, bool state);
	void OnToggleJoystick(Gui::ToggleButton *b, bool state);
	void OnToggleMouseYInvert(Gui::ToggleButton *b, bool state);
	bool m_changedDetailLevel;
	View *m_subview;
	Gui::Adjustment *m_masterVolume;
	Gui::Adjustment *m_sfxVolume;
	Gui::Adjustment *m_musicVolume;
	Gui::RadioGroup *m_planetDetailGroup;
	Gui::RadioGroup *m_cityDetailGroup;
	Gui::ToggleButton *m_toggleShaders;
	Gui::ToggleButton *m_toggleHDR;
	Gui::ToggleButton *m_toggleFullscreen;
	Gui::ToggleButton *m_toggleJoystick;
	Gui::ToggleButton *m_toggleMouseYInvert;
};

#endif /* _GAMEMENUVIEW_H */
