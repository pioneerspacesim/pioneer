#ifndef _GAMEMENUVIEW_H
#define _GAMEMENUVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "KeyBindings.h"

extern std::string GetFullSavefileDirPath();

//contains sliders, mute button and the necessary layout fluff
class VolumeControl : public Gui::HBox
{
	public:
		VolumeControl(const std::string& label) :
			HBox() {
			PackEnd(new Gui::Label(label.c_str()), false);
			Gui::MultiStateImageButton *muteButton = new Gui::MultiStateImageButton();
			muteButton->AddState(1, PIONEER_DATA_DIR "/icons/labels_on.png", "Mute");
			muteButton->AddState(0, PIONEER_DATA_DIR "/icons/labels_off.png", "Unmute");
			PackEnd(muteButton);
			Gui::Adjustment *adjustment = new Gui::Adjustment();
			Gui::HScale *slider = new Gui::HScale();
			slider->SetAdjustment(adjustment);
			PackEnd(slider);
		}
		float GetValue() const {
			return 0.5f;
		}
		sigc::signal<void> onMuteToggled;
		sigc::signal<void> onValueChanged;
};

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
	VolumeControl *m_masterVolume;
	VolumeControl *m_sfxVolume;
	VolumeControl *m_musicVolume;
	Gui::RadioGroup *m_planetDetailGroup;
	Gui::RadioGroup *m_cityDetailGroup;
	Gui::ToggleButton *m_toggleShaders;
	Gui::ToggleButton *m_toggleHDR;
	Gui::ToggleButton *m_toggleFullscreen;
	Gui::ToggleButton *m_toggleJoystick;
	Gui::ToggleButton *m_toggleMouseYInvert;
};

#endif /* _GAMEMENUVIEW_H */
