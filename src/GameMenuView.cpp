// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GameMenuView.h"
#include "Pi.h"
#include "Serializer.h"
#include "WorldView.h"
#include "ShipCpanel.h"
#include "Sound.h"
#include "SoundMusic.h"
#include "KeyBindings.h"
#include "Lang.h"
#include "StringF.h"
#include "Game.h"
#include "FileSystem.h"
#include "FileSelectorWidget.h"
#include "graphics/Graphics.h"

class KeyGetter: public Gui::Fixed {
public:
	KeyGetter(const char *label, const KeyBindings::KeyBinding &kb): Gui::Fixed(350, 19) {
		m_binding = kb;
		m_function = label;
		m_keyLabel = new Gui::Label(m_binding.Description());
		Gui::Button *b = new Gui::LabelButton(m_keyLabel);
		b->onClick.connect(sigc::mem_fun(this, &KeyGetter::OnClickChange));
		Add(new Gui::Label(label), 0, 0);
		Add(b, 180, 0);
		m_infoTooltip = 0;
	}
	sigc::signal<void, KeyBindings::KeyBinding> onChange;
private:
	void OnClickChange() {
		if (m_infoTooltip) return;
		std::string msg = Lang::PRESS_BUTTON_WANTED_FOR + m_function;
		Gui::ToolTip *t = new Gui::ToolTip(this, msg);
		Gui::Screen::AddBaseWidget(t, 300, 300);
		t->Show();
		t->GrabFocus();
		m_keyUpConnection = Gui::RawEvents::onKeyUp.connect(sigc::mem_fun(this, &KeyGetter::OnKeyUpPick));
		m_joyButtonUpConnection = Gui::RawEvents::onJoyButtonUp.connect(sigc::mem_fun(this, &KeyGetter::OnJoyButtonUp));
		m_joyHatConnection = Gui::RawEvents::onJoyHatMotion.connect(sigc::mem_fun(this, &KeyGetter::OnJoyHatMotion));
		m_infoTooltip = t;
	}

	void Disconnect() {
		m_keyUpConnection.disconnect();
		m_joyButtonUpConnection.disconnect();
		m_joyHatConnection.disconnect();
		Gui::Screen::RemoveBaseWidget(m_infoTooltip);
		delete m_infoTooltip;
		m_infoTooltip = 0;
	}

	void Close() {
		onChange.emit(m_binding);
		m_keyLabel->SetText(m_binding.Description());
		ResizeRequest();
	}

	void OnKeyUpPick(SDL_KeyboardEvent *e) {
		Disconnect();
		m_binding.type = KeyBindings::KEYBOARD_KEY;
		m_binding.u.keyboard.key = e->keysym.sym;
		// get rid of number lock, caps lock, etc
		m_binding.u.keyboard.mod = SDL_Keymod(e->keysym.mod & (KMOD_CTRL | KMOD_ALT | KMOD_GUI | KMOD_SHIFT));
		Close();
	}

	void OnJoyButtonUp(SDL_JoyButtonEvent *e) {
		Disconnect();
		m_binding.type = KeyBindings::JOYSTICK_BUTTON;
		m_binding.u.joystickButton.joystick = e->which;
		m_binding.u.joystickButton.button = e->button;
		Close();
	}

	void OnJoyHatMotion(SDL_JoyHatEvent *e) {
		Disconnect();
		m_binding.type = KeyBindings::JOYSTICK_HAT;
		m_binding.u.joystickHat.joystick = e->which;
		m_binding.u.joystickHat.hat = e->hat;
		m_binding.u.joystickHat.direction = e->value;
		Close();
	}

	KeyBindings::KeyBinding m_binding;
	Gui::Label *m_keyLabel;
	Gui::ToolTip *m_infoTooltip;
	std::string m_function;
	sigc::connection m_keyUpConnection;
	sigc::connection m_joyButtonUpConnection;
	sigc::connection m_joyHatConnection;
};

class AxisGetter: public Gui::Fixed {
public:
	AxisGetter(const char *label, const KeyBindings::AxisBinding &ab): Gui::Fixed(350, 19) {
		m_axisBinding = ab;
		m_function = label;
		m_keyLabel = new Gui::Label(ab.Description());
		Gui::Button *b = new Gui::LabelButton(m_keyLabel);
		b->onClick.connect(sigc::mem_fun(this, &AxisGetter::OnClickChange));
		Add(new Gui::Label(label), 0, 0);
		Add(b, 180, 0);
		m_infoTooltip = 0;
	}
	sigc::signal<void, KeyBindings::AxisBinding> onChange;
private:
	void OnClickChange() {
		if (m_infoTooltip) return;
		std::string msg = Lang::MOVE_AXIS_WANTED_FOR + m_function;
		Gui::ToolTip *t = new Gui::ToolTip(this, msg);
		Gui::Screen::AddBaseWidget(t, 300, 300);
		t->Show();
		t->GrabFocus();
		m_keyUpConnection.disconnect();
		m_keyUpConnection = Gui::RawEvents::onJoyAxisMotion.connect(sigc::mem_fun(this, &AxisGetter::OnAxisPick));
		m_infoTooltip = t;
	}

	void OnAxisPick(SDL_JoyAxisEvent *e) {
		if (e->value > -32767/3 && e->value < 32767/3)
			return;

		m_keyUpConnection.disconnect();
		Gui::Screen::RemoveBaseWidget(m_infoTooltip);
		delete m_infoTooltip;
		m_infoTooltip = 0;

		m_axisBinding.joystick = e->which;
		m_axisBinding.axis = e->axis;
		m_axisBinding.direction = (e->value < 0 ? KeyBindings::NEGATIVE : KeyBindings::POSITIVE);

		onChange.emit(m_axisBinding);
		m_keyLabel->SetText(m_axisBinding.Description());
		ResizeRequest();
	}

	KeyBindings::AxisBinding m_axisBinding;
	Gui::Label *m_keyLabel;
	Gui::ToolTip *m_infoTooltip;
	std::string m_function;
	sigc::connection m_keyUpConnection;
};


static const char *planet_detail_desc[5] = {
	Lang::LOW, Lang::MEDIUM, Lang::HIGH, Lang::VERY_HIGH, Lang::VERY_VERY_HIGH
};

static const char *planet_textures_desc[2] = {
	Lang::OFF, Lang::ON
};

static const char *planet_fractal_desc[5] = {
	Lang::VERY_LOW, Lang::LOW, Lang::MEDIUM, Lang::HIGH, Lang::VERY_HIGH
};

GameMenuView::~GameMenuView()
{
	// Have to explicitly delete RadioGroups because they're not a physical GUI element
	delete m_screenModesGroup;
	delete m_planetDetailGroup;
	delete m_planetTextureGroup;
	delete m_planetFractalGroup;
	delete m_cityDetailGroup;
	delete m_languageGroup;
}

GameMenuView::GameMenuView(): View()
{
	Gui::Tabbed *tabs = new Gui::Tabbed();
	Add(tabs, 0, 0);

	Gui::Fixed *mainTab = new Gui::Fixed(800, 600);
	tabs->AddPage(new Gui::Label(Lang::SIGHTS_SOUNDS_SAVES), mainTab);

	mainTab->Add((new Gui::Label(Lang::PIONEER))->Shadow(true), 350, 10);
	SetTransparency(false);
	Gui::Label *l = new Gui::Label(Lang::PIONEER);
	l->Color(1,.7f,0);
	m_rightRegion2->Add(l, 10, 0);

	{
		Gui::Box *hbox = new Gui::HBox();
		hbox->SetSpacing(5.0f);
		mainTab->Add(hbox, 20, 30);

		m_saveButton = new Gui::LabelButton(new Gui::Label(Lang::SAVE_THE_GAME));
		m_saveButton->SetShortcut(SDLK_s, KMOD_NONE);
		m_saveButton->onClick.connect(sigc::mem_fun(this, &GameMenuView::OpenSaveDialog));
		hbox->PackEnd(m_saveButton);
		m_loadButton = new Gui::LabelButton(new Gui::Label(Lang::LOAD_A_GAME));
		m_loadButton->onClick.connect(sigc::mem_fun(this, &GameMenuView::OpenLoadDialog));
		m_loadButton->SetShortcut(SDLK_l, KMOD_NONE);
		hbox->PackEnd(m_loadButton);
		m_exitButton = new Gui::LabelButton(new Gui::Label(Lang::EXIT_THIS_GAME));
		m_exitButton->onClick.connect(sigc::ptr_fun(&Pi::EndGame));
		hbox->PackEnd(m_exitButton);

		m_menuButton = new Gui::LabelButton(new Gui::Label(Lang::RETURN_TO_MENU));
		m_menuButton->onClick.connect(sigc::bind(sigc::ptr_fun(&Pi::SetView), static_cast<View*>(0)));
		mainTab->Add(m_menuButton, 20, 30);
	}

	Gui::Box *vbox = new Gui::VBox();
	vbox->SetSizeRequest(300, 440);
	vbox->SetSpacing(5.0);
	mainTab->Add(vbox, 20, 60);

	{
		vbox->PackEnd((new Gui::Label(Lang::WINDOW_OR_FULLSCREEN))->Color(1.0f,1.0f,0.0f));
		m_toggleFullscreen = new Gui::ToggleButton();
		m_toggleFullscreen->onChange.connect(sigc::mem_fun(this, &GameMenuView::OnToggleFullscreen));
		Gui::HBox *hbox = new Gui::HBox();
		hbox->SetSpacing(5.0f);
		hbox->PackEnd(m_toggleFullscreen);
		hbox->PackEnd(new Gui::Label(Lang::FULL_SCREEN));
		vbox->PackEnd(hbox);

		vbox->PackEnd((new Gui::Label(Lang::OTHER_GRAPHICS_SETTINGS))->Color(1.0f,1.0f,0.0f));
		m_toggleShaders = new Gui::ToggleButton();
		m_toggleShaders->onChange.connect(sigc::mem_fun(this, &GameMenuView::OnToggleShaders));
		hbox = new Gui::HBox();
		hbox->SetSpacing(5.0f);
		hbox->PackEnd(m_toggleShaders);
		hbox->PackEnd(new Gui::Label(Lang::USE_SHADERS));
		m_toggleCompressTextures = new Gui::ToggleButton();
		m_toggleCompressTextures->onChange.connect(sigc::mem_fun(this, &GameMenuView::OnToggleCompressTextures));
		hbox->PackEnd(m_toggleCompressTextures);
		hbox->PackEnd(new Gui::Label(Lang::COMPRESS_TEXTURES));
		vbox->PackEnd(hbox);

		vbox->PackEnd((new Gui::Label(Lang::SOUND_SETTINGS))->Color(1.0f,1.0f,0.0f));
		m_masterVolume = new VolumeControl(Lang::VOL_MASTER, Pi::config->Float("MasterVolume"), Pi::config->Int("MasterMuted"));
		vbox->PackEnd(m_masterVolume);
		m_sfxVolume = new VolumeControl(Lang::VOL_EFFECTS, Pi::config->Float("SfxVolume"), Pi::config->Int("SfxMuted"));
		vbox->PackEnd(m_sfxVolume);
		m_musicVolume = new VolumeControl(Lang::VOL_MUSIC, Pi::config->Float("MusicVolume"), Pi::config->Int("MusicMuted"));
		vbox->PackEnd(m_musicVolume);

		m_masterVolume->onChanged.connect(sigc::mem_fun(this, &GameMenuView::OnChangeVolume));
		m_sfxVolume->onChanged.connect(sigc::mem_fun(this, &GameMenuView::OnChangeVolume));
		m_musicVolume->onChanged.connect(sigc::mem_fun(this, &GameMenuView::OnChangeVolume));
	}

	// Video mode selector
	{
		m_videoModes = Graphics::GetAvailableVideoModes();
		vbox->PackEnd((new Gui::Label(Lang::VIDEO_RESOLUTION))->Color(1.0f,1.0f,0.0f));

		m_screenModesGroup = new Gui::RadioGroup();

		// box to put the scroll portal and its scroll bar into
		Gui::HBox *scrollHBox = new Gui::HBox();
		vbox->PackEnd(scrollHBox);

		Gui::VScrollBar *scroll = new Gui::VScrollBar();
		Gui::VScrollPortal *portal = new Gui::VScrollPortal(280);
		scroll->SetAdjustment(&portal->vscrollAdjust);
		scrollHBox->PackEnd(portal);
		scrollHBox->PackEnd(scroll);

		Gui::VBox *vbox2 = new Gui::VBox();
		portal->Add(vbox2);

		for (std::vector<Graphics::VideoMode>::const_iterator it = m_videoModes.begin();
			it != m_videoModes.end(); ++it) {
			Gui::RadioButton *temp = new Gui::RadioButton(m_screenModesGroup);
			temp->onSelect.connect(sigc::bind(sigc::mem_fun(this,
				&GameMenuView::OnChangeVideoResolution), it - m_videoModes.begin()));
			Gui::HBox *hbox = new Gui::HBox();
			hbox->SetSpacing(5.0f);
			hbox->PackEnd(temp);
			hbox->PackEnd(new Gui::Label(stringf(Lang::X_BY_X, formatarg("x", int(it->width)), formatarg("y", int(it->height)))));
			vbox2->PackEnd(hbox);

			//mark the current video mode
			if ((Graphics::GetScreenWidth() == it->width) && (Graphics::GetScreenHeight() == it->height)) {
				temp->SetSelected(true);
			}
		}
	}

	//Graphical detail settings
	{
		Gui::HBox *detailBox = new Gui::HBox();
		detailBox->SetSpacing(20.0f);
		mainTab->Add(detailBox, 350, 60);

		vbox = new Gui::VBox();
		vbox->SetSpacing(5.0f);
		detailBox->PackEnd(vbox);

		vbox->PackEnd((new Gui::Label(Lang::CITY_DETAIL_LEVEL))->Color(1.0f,1.0f,0.0f));
		m_cityDetailGroup = new Gui::RadioGroup();

		for (int i=0; i<5; i++) {
			Gui::RadioButton *rb = new Gui::RadioButton(m_cityDetailGroup);
			rb->onSelect.connect(sigc::bind(sigc::mem_fun(this, &GameMenuView::OnChangeCityDetail), i));
			Gui::HBox *hbox = new Gui::HBox();
			hbox->SetSpacing(5.0f);
			hbox->PackEnd(rb);
			hbox->PackEnd(new Gui::Label(planet_detail_desc[i]));
			vbox->PackEnd(hbox);
		}

		vbox = new Gui::VBox();
		vbox->SetSpacing(5.0f);
		detailBox->PackEnd(vbox);

		vbox->PackEnd((new Gui::Label(Lang::PLANET_DETAIL_DISTANCE))->Color(1.0f,1.0f,0.0f));
		m_planetDetailGroup = new Gui::RadioGroup();

		for (int i=0; i<5; i++) {
			Gui::RadioButton *rb = new Gui::RadioButton(m_planetDetailGroup);
			rb->onSelect.connect(sigc::bind(sigc::mem_fun(this, &GameMenuView::OnChangePlanetDetail), i));
			Gui::HBox *hbox = new Gui::HBox();
			hbox->SetSpacing(5.0f);
			hbox->PackEnd(rb);
			hbox->PackEnd(new Gui::Label(planet_detail_desc[i]));
			vbox->PackEnd(hbox);
		}

		vbox = new Gui::VBox();
		vbox->SetSpacing(5.0f);
		detailBox->PackEnd(vbox);

		vbox->PackEnd((new Gui::Label(Lang::PLANET_TEXTURES))->Color(1.0f,1.0f,0.0f));
		m_planetTextureGroup = new Gui::RadioGroup();

		for (int i=0; i<2; i++) {
			Gui::RadioButton *rb = new Gui::RadioButton(m_planetTextureGroup);
			rb->onSelect.connect(sigc::bind(sigc::mem_fun(this, &GameMenuView::OnChangePlanetTextures), i));
			Gui::HBox *hbox = new Gui::HBox();
			hbox->SetSpacing(5.0f);
			hbox->PackEnd(rb);
			hbox->PackEnd(new Gui::Label(planet_textures_desc[i]));
			vbox->PackEnd(hbox);
		}

		vbox = new Gui::VBox();
		vbox->SetSpacing(5.0f);
		detailBox->PackEnd(vbox);

		vbox->PackEnd((new Gui::Label(Lang::FRACTAL_DETAIL))->Color(1.0f,1.0f,0.0f));
		m_planetFractalGroup = new Gui::RadioGroup();

		for (int i=0; i<5; i++) {
			Gui::RadioButton *rb = new Gui::RadioButton(m_planetFractalGroup);
			rb->onSelect.connect(sigc::bind(sigc::mem_fun(this, &GameMenuView::OnChangeFractalMultiple), i));
			Gui::HBox *hbox = new Gui::HBox();
			hbox->SetSpacing(5.0f);
			hbox->PackEnd(rb);
			hbox->PackEnd(new Gui::Label(planet_fractal_desc[i]));
			vbox->PackEnd(hbox);
		}
	}


	// language

	vbox = new Gui::VBox();
	vbox->SetSizeRequest(300, 200);
	mainTab->Add(vbox, 400, 250);

	vbox->PackEnd((new Gui::Label(Lang::LANGUAGE_SELECTION))->Color(1.0f,1.0f,0.0f));

	m_languageGroup = new Gui::RadioGroup();
	const std::vector<std::string> &availableLanguages = Lang::GetAvailableLanguages();

	{
		// box to put the scroll portal and its scroll bar into
		Gui::HBox *scrollHBox = new Gui::HBox();
		vbox->PackEnd(scrollHBox);

		Gui::VScrollBar *scroll = new Gui::VScrollBar();
		Gui::VScrollPortal *portal = new Gui::VScrollPortal(280);
		scroll->SetAdjustment(&portal->vscrollAdjust);
		scrollHBox->PackEnd(portal);
		scrollHBox->PackEnd(scroll);

		Gui::VBox *vbox2 = new Gui::VBox();
		portal->Add(vbox2);

		for (std::vector<std::string>::const_iterator i = availableLanguages.begin(); i != availableLanguages.end(); ++i) {
			Gui::RadioButton *temp = new Gui::RadioButton(m_languageGroup);
			temp->onSelect.connect(sigc::bind(sigc::mem_fun(this, &GameMenuView::OnChangeLanguage), *i));
			Gui::HBox *hbox = new Gui::HBox();
			hbox->SetSpacing(5.0f);
			hbox->PackEnd(temp);
			hbox->PackEnd(new Gui::Label(*i));
			vbox2->PackEnd(hbox);
			if ((*i) == Pi::config->String("Lang"))
				temp->SetSelected(true);
		}
	}

	// key binding tab 1
	{
		Gui::Fixed *keybindingTab = new Gui::Fixed(800, 600);
		tabs->AddPage(new Gui::Label(Lang::CONTROLS), keybindingTab);

		Gui::VBox *box1 = new Gui::VBox();
		box1->SetSpacing(5.0f);
		keybindingTab->Add(box1, 10, 10);

		Gui::VBox *box2 = new Gui::VBox();
		box2->SetSpacing(5.0f);
		keybindingTab->Add(box2, 400, 10);

		BuildControlBindingList(KeyBindings::BINDING_PROTOS_CONTROLS, box1, box2);

		Gui::VBox *box = box2;

		m_toggleJoystick = new Gui::ToggleButton();
		m_toggleJoystick->onChange.connect(sigc::mem_fun(this, &GameMenuView::OnToggleJoystick));
		Gui::HBox *hbox = new Gui::HBox();
		hbox->SetSpacing(5.0f);
		hbox->PackEnd(m_toggleJoystick);
		hbox->PackEnd(new Gui::Label(Lang::ENABLE_JOYSTICK));
		box->PackEnd(hbox);

		// Invert Mouse
		m_toggleMouseYInvert = new Gui::ToggleButton();
		m_toggleMouseYInvert->onChange.connect(sigc::mem_fun(this, &GameMenuView::OnToggleMouseYInvert));
		box->PackEnd((new Gui::Label(Lang::MOUSE_INPUT))->Color(1.0f,1.0f,0.0f));

		Gui::HBox *mybox = new Gui::HBox();
		mybox->SetSpacing(5.0f);
		mybox->PackEnd(m_toggleMouseYInvert);
		mybox->PackEnd(new Gui::Label(Lang::INVERT_MOUSE_Y));
		box->PackEnd(mybox);

		// Toggle nav tunnel
		m_toggleNavTunnel = new Gui::ToggleButton();
		m_toggleNavTunnel->onChange.connect(sigc::mem_fun(this, &GameMenuView::OnToggleNavTunnel));
		box->PackEnd((new Gui::Label(Lang::HUD))->Color(1.0f,1.0f,0.0f));

		Gui::HBox *guibox = new Gui::HBox();
		guibox->SetSpacing(5.0f);
		guibox->PackEnd(m_toggleNavTunnel);
		guibox->PackEnd(new Gui::Label(Lang::DISPLAY_NAV_TUNNEL));
		box->PackEnd(guibox);
	}

	// key binding tab 2
	{
		Gui::Fixed *keybindingTab = new Gui::Fixed(800, 600);
		tabs->AddPage(new Gui::Label(Lang::VIEW), keybindingTab);

		Gui::VBox *box1 = new Gui::VBox();
		box1->SetSpacing(5.0f);
		keybindingTab->Add(box1, 10, 10);

		BuildControlBindingList(KeyBindings::BINDING_PROTOS_VIEW, box1, 0);
	}
}

void GameMenuView::BuildControlBindingList(const KeyBindings::BindingPrototype *protos, Gui::VBox *box1, Gui::VBox *box2)
{
	assert(protos);
	Gui::VBox *box = box1;
	for (int i=0; protos[i].label; i++) {
		const KeyBindings::BindingPrototype* proto = &protos[i];

		if (!proto->function) {
			// section header
			box->PackEnd((new Gui::Label(proto->label))->Color(1.0f, 1.0f, 0.0f));
		} else {
			if (proto->kb) {
				KeyGetter *keyg;
				KeyBindings::KeyBinding kb = KeyBindings::KeyBindingFromString(Pi::config->String(proto->function));
				keyg = new KeyGetter(proto->label, kb);
				keyg->onChange.connect(sigc::bind(sigc::mem_fun(this, &GameMenuView::OnChangeKeyBinding), proto->function));
				box->PackEnd(keyg);
			} else if (proto->ab) {
				AxisGetter *axisg;
				KeyBindings::AxisBinding ab = KeyBindings::AxisBindingFromString(Pi::config->String(proto->function).c_str());
				axisg = new AxisGetter(proto->label, ab);
				axisg->onChange.connect(sigc::bind(sigc::mem_fun(this, &GameMenuView::OnChangeAxisBinding), proto->function));
				box->PackEnd(axisg);
			} else {
				assert(0);
			}
		}

		// 2nd column
		if ((i == 20) && box2) {
			box = box2;
		}
	}
}

void GameMenuView::OnChangeKeyBinding(const KeyBindings::KeyBinding &kb, const char *fnName)
{
	Pi::config->SetString(fnName, KeyBindings::KeyBindingToString(kb).c_str());
	Pi::config->Save();
	KeyBindings::UpdateBindings();
}

void GameMenuView::OnChangeAxisBinding(const KeyBindings::AxisBinding &ab, const char *function) {
	Pi::config->SetString(function, KeyBindings::AxisBindingToString(ab).c_str());
	Pi::config->Save();
	KeyBindings::UpdateBindings();
}

void GameMenuView::OnChangeVolume()
{
	const float masterVol = m_masterVolume->GetValue();
	const float sfxVol = m_sfxVolume->GetValue();
	const float musVol = m_musicVolume->GetValue();

	if (m_masterVolume->IsMuted())
		Sound::Pause(1);
	else
		Sound::Pause(0);

	Sound::SetMasterVolume(masterVol);

	if (m_sfxVolume->IsMuted())
		Sound::SetSfxVolume(0.f);
	else
		Sound::SetSfxVolume(sfxVol); //can't "pause" sfx separately

	if (m_musicVolume->IsMuted())
		Pi::GetMusicPlayer().SetEnabled(false);
	else
		Pi::GetMusicPlayer().SetEnabled(true);

	Pi::GetMusicPlayer().SetVolume(musVol);

	Pi::config->SetFloat("MasterVolume", masterVol);
	Pi::config->SetFloat("SfxVolume", sfxVol);
	Pi::config->SetFloat("MusicVolume", musVol);
	Pi::config->SetFloat("MasterMuted", m_masterVolume->IsMuted());
	Pi::config->SetFloat("SfxMuted", m_sfxVolume->IsMuted());
	Pi::config->SetFloat("MusicMuted", m_musicVolume->IsMuted());
	Pi::config->Save();
}

void GameMenuView::OnChangePlanetDetail(int level)
{
	if (level == Pi::detail.planets) return;
	m_changedDetailLevel = true;
	Pi::detail.planets = level;
	Pi::config->SetInt("DetailPlanets", level);
	Pi::config->Save();
}

void GameMenuView::OnChangePlanetTextures(int level)
{
	if (level == Pi::detail.textures) return;
	m_changedDetailLevel = true;
	Pi::detail.textures = level;
	Pi::config->SetInt("Textures", level);
	Pi::config->Save();
}
void GameMenuView::OnChangeFractalMultiple(int level)
{
	if (level == Pi::detail.fracmult) return;
	m_changedDetailLevel = true;
	Pi::detail.fracmult = level;
	Pi::config->SetInt("FractalMultiple", level);
	Pi::config->Save();
}

void GameMenuView::OnChangeCityDetail(int level)
{
	if (level == Pi::detail.cities) return;
	m_changedDetailLevel = true;
	Pi::detail.cities = level;
	Pi::config->SetInt("DetailCities", level);
	Pi::config->Save();
}

void GameMenuView::OnChangeLanguage(std::string &lang)
{
	Pi::config->SetString("Lang", lang.c_str());
	Pi::config->Save();
}

void GameMenuView::OnChangeVideoResolution(int modeIndex)
{
	const Graphics::VideoMode &mode = m_videoModes.at(modeIndex);
	Pi::config->SetInt("ScrWidth", mode.width);
	Pi::config->SetInt("ScrHeight", mode.height);
	Pi::config->Save();
}

void GameMenuView::OnToggleFullscreen(Gui::ToggleButton *b, bool state)
{
	Pi::config->SetInt("StartFullscreen", (state ? 1 : 0));
	Pi::config->Save();
//#ifndef _WIN32
	// XXX figure out how to do it in windows
//	SDL_WM_ToggleFullScreen(Pi::scrSurface);
//#endif
}

void GameMenuView::OnToggleCompressTextures(Gui::ToggleButton *b, bool state)
{
	Pi::config->SetInt("UseTextureCompression", (state ? 1 : 0));
	Pi::config->Save();
}

void GameMenuView::OnToggleShaders(Gui::ToggleButton *b, bool state)
{
	Pi::config->SetInt("DisableShaders", (state ? 0 : 1));
	Pi::config->Save();
}

void GameMenuView::OnToggleJoystick(Gui::ToggleButton *b, bool state)
{
	Pi::config->SetInt("EnableJoystick", (state ? 1 : 0));
	Pi::config->Save();
	Pi::SetJoystickEnabled(state);
}

void GameMenuView::OnToggleMouseYInvert(Gui::ToggleButton *b, bool state)
{
	Pi::config->SetInt("InvertMouseY", (state ? 1 : 0));
	Pi::config->Save();
	Pi::SetMouseYInvert(state);
}

void GameMenuView::OnToggleNavTunnel(Gui::ToggleButton *b, bool state) {
	Pi::config->SetInt("DisplayNavTunnel", (state ? 1 : 0));
	Pi::config->Save();
	Pi::SetNavTunnelDisplayed(state);
}

void GameMenuView::OpenSaveDialog()
{
	if (Pi::game->IsHyperspace()) {
		Pi::cpan->MsgLog()->Message("", Lang::CANT_SAVE_IN_HYPERSPACE);
		return;
	}

	std::string filename;
	bool ok = ShowFileSelectorDialog(FileSelectorWidget::SAVE, Lang::SELECT_FILENAME_TO_SAVE, filename);
	if (ok) {
		const std::string path = FileSystem::JoinPath(Pi::GetSaveDir(), filename);
		try {
			Game::SaveGame(filename, Pi::game);
			Pi::cpan->MsgLog()->Message("", Lang::GAME_SAVED_TO + path); // XXX stringf with an arg would be better
		}
		catch (CouldNotOpenFileException) {
			Gui::Screen::ShowBadError(stringf(Lang::COULD_NOT_OPEN_FILENAME, formatarg("path", path)).c_str());
		}
		catch (CouldNotWriteToFileException) {
			Gui::Screen::ShowBadError(Lang::GAME_SAVE_CANNOT_WRITE);
		}
	}
}

void GameMenuView::OpenLoadDialog()
{
	std::string filename;
	bool ok = ShowFileSelectorDialog(FileSelectorWidget::LOAD, Lang::SELECT_FILENAME_TO_LOAD, filename);
	if (ok) {
		// XXX one day we'll be able to load a new game without tearing down the old one
		Pi::EndGame();
		try {
			Game *newGame = Game::LoadGame(filename);
			Pi::game = newGame;
			Pi::InitGame();
			Pi::StartGame();
		}
		catch (SavedGameCorruptException) {
			Gui::Screen::ShowBadError(Lang::GAME_LOAD_CORRUPT);
		}
		catch (SavedGameWrongVersionException) {
			Gui::Screen::ShowBadError(Lang::GAME_LOAD_WRONG_VERSION);
		}
		catch (CouldNotOpenFileException) {
			Gui::Screen::ShowBadError(Lang::GAME_LOAD_CANNOT_OPEN);
		}
	}
}

void GameMenuView::ShowAll() {
	View::ShowAll();
	if (Pi::game) {
		m_saveButton->Show();
		m_loadButton->Show();
		m_exitButton->Show();
		m_menuButton->Hide();
	}
	else {
		m_saveButton->Hide();
		m_loadButton->Hide();
		m_exitButton->Hide();
		m_menuButton->Show();
	}
}

void GameMenuView::OnSwitchTo() {
	m_changedDetailLevel = false;
	m_planetDetailGroup->SetSelected(Pi::detail.planets);
	m_planetTextureGroup->SetSelected(Pi::detail.textures);
	m_planetFractalGroup->SetSelected(Pi::detail.fracmult);
	m_cityDetailGroup->SetSelected(Pi::detail.cities);
	m_toggleShaders->SetPressed(Pi::config->Int("DisableShaders") == 0);
	m_toggleFullscreen->SetPressed(Pi::config->Int("StartFullscreen") != 0);
	m_toggleCompressTextures->SetPressed(Pi::config->Int("UseTextureCompression") != 0);
	m_toggleJoystick->SetPressed(Pi::IsJoystickEnabled());
	m_toggleMouseYInvert->SetPressed(Pi::IsMouseYInvert());
	m_toggleNavTunnel->SetPressed(Pi::IsNavTunnelDisplayed());
}

void GameMenuView::OnSwitchFrom() {
	if (m_changedDetailLevel) {
		Pi::OnChangeDetailLevel();
	}
}
