// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GAMEMENUVIEW_H
#define _GAMEMENUVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "KeyBindings.h"
#include "View.h"
#include "graphics/Graphics.h"

//contains a slider, mute button and the necessary layout fluff
class VolumeControl : public Gui::HBox
{
	public:
		VolumeControl(const std::string& label, float volume, bool muted) :
			HBox() {
			Gui::Label *lab = new Gui::Label(label.c_str());
			Gui::Fixed *fix = new Gui::Fixed(50, 32);
			fix->Add(lab, 0, 0);
			PackEnd(fix);
			m_muteButton = new Gui::MultiStateImageButton();
			m_muteButton->AddState(0, "icons/volume_unmuted.png", "Mute");
			m_muteButton->AddState(1, "icons/volume_muted.png", "Unmute");
			m_muteButton->SetActiveState(muted ? 1 : 0);
			m_muteButton->SetRenderDimensions(32, 32);
			PackEnd(m_muteButton);
			m_adjustment = new Gui::Adjustment();
			m_adjustment->SetValue(volume);
			Gui::HScale *slider = new Gui::HScale();
			slider->SetAdjustment(m_adjustment);
			PackEnd(slider);

			//signals
			m_muteButton->onClick.connect(sigc::mem_fun(this, &VolumeControl::propagateMute));
			m_adjustment->onValueChanged.connect(sigc::mem_fun(this, &VolumeControl::propagateSlider));
		}
		virtual ~VolumeControl() {
			delete m_adjustment;
		}
		float GetValue() const {
			return m_adjustment->GetValue();
		}
		void SetValue(float v) {
			m_adjustment->SetValue(v);
		}
		bool IsMuted() const {
			return m_muteButton->GetState() == 1 ? true : false;
		}
		sigc::signal<void> onChanged;
private:
		Gui::MultiStateImageButton *m_muteButton;
		Gui::Adjustment *m_adjustment;
		//is there a better way?
		void propagateSlider() { onChanged.emit(); }
		void propagateMute(Gui::Widget *) { onChanged.emit(); }
};

class GameMenuView: public View {
public:
	GameMenuView();
	virtual ~GameMenuView();
	virtual void Update() {}
	virtual void Draw3D() {}
	virtual void ShowAll();
	virtual void HideAll();
	void OpenLoadDialog();
	void OpenSaveDialog();

protected:
	virtual void OnSwitchTo();

private:
	void BuildControlBindingList(const KeyBindings::BindingPrototype *protos, Gui::VBox *box1, Gui::VBox *box2);

	void OnChangeKeyBinding(const KeyBindings::KeyBinding &kb, const char *fnName);
	void OnChangeAxisBinding(const KeyBindings::AxisBinding &ab, const char *function);
	void OnChangeVolume();
	void OnChangePlanetDetail(int level);
	void OnChangePlanetTextures(int level);
	void OnChangeFractalMultiple(int level);
	void OnChangeCityDetail(int level);
	void OnChangeLanguage(std::string &lang);
	void OnChangeVideoResolution(int res);
	void OnToggleShaders(Gui::ToggleButton *b, bool state);
	void OnToggleFullscreen(Gui::ToggleButton *b, bool state);
	void OnToggleCompressTextures(Gui::ToggleButton *b, bool state);
	void OnToggleJoystick(Gui::ToggleButton *b, bool state);
	void OnToggleMouseYInvert(Gui::ToggleButton *b, bool state);
	void OnToggleNavTunnel(Gui::ToggleButton *b, bool state);

	bool m_changedDetailLevel;
	Gui::Button *m_saveButton;
	Gui::Button *m_loadButton;
	Gui::Button *m_exitButton;
	Gui::Button *m_menuButton;
	VolumeControl *m_masterVolume;
	VolumeControl *m_sfxVolume;
	VolumeControl *m_musicVolume;
	Gui::RadioGroup *m_screenModesGroup;
	Gui::RadioGroup *m_planetDetailGroup;
	Gui::RadioGroup *m_planetTextureGroup;
	Gui::RadioGroup *m_planetFractalGroup;
	Gui::RadioGroup *m_cityDetailGroup;
	Gui::RadioGroup *m_languageGroup;
	Gui::ToggleButton *m_toggleShaders;
	Gui::ToggleButton *m_toggleFullscreen;
	Gui::ToggleButton *m_toggleCompressTextures;
	Gui::ToggleButton *m_toggleJoystick;
	Gui::ToggleButton *m_toggleMouseYInvert;
	Gui::ToggleButton *m_toggleNavTunnel;
	std::vector<Graphics::VideoMode> m_videoModes;
};

#endif /* _GAMEMENUVIEW_H */
