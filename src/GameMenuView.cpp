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

#if _GNU_SOURCE
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#elif _WIN32
#include "win32-dirent.h"
#endif

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
		Gui::ToolTip *t = new Gui::ToolTip(msg);
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
		m_binding.u.keyboard.mod = SDLMod(e->keysym.mod & (KMOD_CTRL | KMOD_ALT | KMOD_META | KMOD_SHIFT));
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
		Gui::ToolTip *t = new Gui::ToolTip(msg);
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

/*
 * Must create the folders if they do not exist already.
 */
std::string GetFullSavefileDirPath()
{
	return GetPiUserDir("savefiles");
}

/* Not dirs, not . or .. */
static void GetDirectoryContents(const char *name, std::list<std::string> &files)
{
	DIR *dir = opendir(name);
	if (!dir) {
		//if (-1 == mkdir(name, 0770)
		Gui::Screen::ShowBadError(stringf(Lang::COULD_NOT_OPEN_FILENAME, formatarg("path", name)).c_str());
		return;
	}
	struct dirent *entry;

	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".")==0) continue;
		if (strcmp(entry->d_name, "..")==0) continue;
		files.push_back(entry->d_name);
	}

	closedir(dir);

	files.sort();
}

class SimpleLabelButton: public Gui::LabelButton
{
public:
	SimpleLabelButton(Gui::Label *label): Gui::LabelButton(label) {
		SetPadding(0.0f);
	}
	virtual void Draw() {
		m_label->Draw();
	}
};

class FileDialog: public Gui::VBox {
public:
	enum TYPE { LOAD, SAVE };
	FileDialog(TYPE t, const char *title): Gui::VBox() {
		m_type = t;
		m_title = title;
		SetTransparency(false);
		SetSpacing(5.0f);
		SetSizeRequest(FLT_MAX, FLT_MAX);
	}

	void ShowAll() {
		DeleteAllChildren();
		PackEnd(new Gui::Label(m_title));
		m_tentry = new Gui::TextEntry();
		PackEnd(m_tentry);

		std::list<std::string> files;
		GetDirectoryContents(GetFullSavefileDirPath().c_str(), files);

		Gui::HBox *hbox = new Gui::HBox();
		PackEnd(hbox);

		Gui::HBox *buttonBox = new Gui::HBox();
		buttonBox->SetSpacing(5.0f);
		Gui::Button *b = new Gui::LabelButton(new Gui::Label(m_type == SAVE ? Lang::SAVE : Lang::LOAD));
		b->onClick.connect(sigc::mem_fun(this, &FileDialog::OnClickAction));
		buttonBox->PackEnd(b);
		b = new Gui::LabelButton(new Gui::Label(Lang::CANCEL));
		b->onClick.connect(sigc::mem_fun(this, &FileDialog::OnClickCancel));
		buttonBox->PackEnd(b);
		PackEnd(buttonBox);


		Gui::VScrollBar *scroll = new Gui::VScrollBar();
		Gui::VScrollPortal *portal = new Gui::VScrollPortal(390);
		portal->SetTransparency(false);
		scroll->SetAdjustment(&portal->vscrollAdjust);
		hbox->PackEnd(portal);
		hbox->PackEnd(scroll);

		Gui::Box *vbox = new Gui::VBox();
		for (std::list<std::string>::iterator i = files.begin(); i!=files.end(); ++i) {
			b = new SimpleLabelButton(new Gui::Label(*i));
			b->onClick.connect(sigc::bind(sigc::mem_fun(this, &FileDialog::OnClickFile), *i));
			vbox->PackEnd(b);
		}
		portal->Add(vbox);
		
		Gui::VBox::ShowAll();
	}
	sigc::signal<void,std::string> onClickAction;
	sigc::signal<void> onClickCancel;
private:
	void OnClickAction() {
		onClickAction.emit(m_tentry->GetText());
	}
	void OnClickCancel() {
		onClickCancel.emit();
	}
	void OnClickFile(std::string file) {
		m_tentry->SetText(file);
	}
	Gui::TextEntry *m_tentry;
	TYPE m_type;
	std::string m_title;
};

class SaveDialogView: public View {
public:
	SaveDialogView() {
		SetTransparency(false);
		SetBgColor(0,0,0,1.0);

		Gui::Fixed *f2 = new Gui::Fixed(410, 410);
		f2->SetTransparency(false);
		Add(f2, 195, 45);
		Gui::Fixed *f = new Gui::Fixed(400, 400);
		f2->Add(f, 5, 5);
		m_fileDialog = new FileDialog(FileDialog::SAVE, Lang::SELECT_FILENAME_TO_SAVE);
		f->Add(m_fileDialog, 0, 0);

		m_fileDialog->onClickCancel.connect(sigc::mem_fun(this, &SaveDialogView::OnClickBack));
		m_fileDialog->onClickAction.connect(sigc::mem_fun(this, &SaveDialogView::OnClickSave));
	}
	virtual void Update() {}
	virtual void Draw3D() {}
	virtual void OnSwitchTo() {}
private:
	void OnClickSave(std::string filename) {
		if (filename.empty()) return;
		std::string fullname = join_path(GetFullSavefileDirPath().c_str(), filename.c_str(), 0);
		Serializer::SaveGame(fullname.c_str());
		Pi::cpan->MsgLog()->Message("", Lang::GAME_SAVED_TO+fullname);
		m_fileDialog->ShowAll();
	}
	void OnClickBack() { Pi::SetView(Pi::gameMenuView); }
	FileDialog *m_fileDialog;
};

class LoadDialogView: public View {
public:
	LoadDialogView() {
		SetTransparency(false);
		SetBgColor(0,0,0,1.0);

		Gui::Fixed *f2 = new Gui::Fixed(410, 410);
		f2->SetTransparency(false);
		Add(f2, 195, 45);
		Gui::Fixed *f = new Gui::Fixed(400, 400);
		f2->Add(f, 5, 5);
		m_fileDialog = new FileDialog(FileDialog::LOAD, Lang::SELECT_FILENAME_TO_LOAD);
		f->Add(m_fileDialog, 0, 0);

		m_fileDialog->onClickCancel.connect(sigc::mem_fun(this, &LoadDialogView::OnClickBack));
		m_fileDialog->onClickAction.connect(sigc::mem_fun(this, &LoadDialogView::OnClickLoad));
	}
	virtual void Update() {}
	virtual void Draw3D() {}
	virtual void OnSwitchTo() {}
private:
    
    // XXX this is an insane mess. what we want to do is load the game up into
    // a brand new Space object, and once we're sure the load is completed
    // successfully, throw away the old Space object and swap in the new one.
    // unfortunately we don't have a Space object right now, and its going to
    // take a lot of work elsewhere to get us one
    //
    // until then, we really can't guarantee that the game is in a consistent
    // state after a load fails, so we just throw them back to the menu
    
	void OnClickLoad(std::string filename) {
		if (filename.empty()) return;
		std::string fullname = join_path(GetFullSavefileDirPath().c_str(), filename.c_str(), 0);

        if (Pi::IsGameStarted())
			Pi::EndGame();

		Pi::UninitGame();
		Pi::InitGame();

		try {
			Serializer::LoadGame(fullname.c_str());
		} catch (SavedGameCorruptException) {
			Gui::Screen::ShowBadError(Lang::GAME_LOAD_CORRUPT);
			Pi::UninitGame();
			Pi::InitGame();
			Pi::SetView(Pi::gameMenuView); // Pi::currentView is unset, set it back to the gameMenuView
			return;
		} catch (CouldNotOpenFileException) {
			Gui::Screen::ShowBadError(Lang::GAME_LOAD_CANNOT_OPEN);
			Pi::UninitGame();
			Pi::InitGame();
			Pi::SetView(Pi::gameMenuView); // Pi::currentView is unset, set it back to the gameMenuView
			return;
		}

		Pi::StartGame();

		// Pi::currentView is unset, but this view is still shown, so
		// must un-show it
		Pi::SetView(Pi::gameMenuView);
		Pi::SetView(Pi::worldView);
	}
	void OnClickBack() { Pi::SetView(Pi::gameMenuView); }
	FileDialog *m_fileDialog;
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
	m_subview = 0;

	Gui::Tabbed *tabs = new Gui::Tabbed();
	Add(tabs, 0, 0);

	Gui::Fixed *mainTab = new Gui::Fixed(800, 600);
	tabs->AddPage(new Gui::Label(Lang::SIGHTS_SOUNDS_SAVES), mainTab);

	mainTab->Add((new Gui::Label(Lang::PIONEER))->Shadow(true), 350, 10);
	SetTransparency(false);
	Gui::Label *l = new Gui::Label(Lang::PIONEER);
	l->Color(1,.7,0);
	m_rightRegion2->Add(l, 10, 0);
	
	{
		Gui::LabelButton *b;
		Gui::Box *hbox = new Gui::HBox();
		hbox->SetSpacing(5.0f);
		mainTab->Add(hbox, 20, 30);
		b = new Gui::LabelButton(new Gui::Label(Lang::SAVE_THE_GAME));
		b->SetShortcut(SDLK_s, KMOD_NONE);
		b->onClick.connect(sigc::mem_fun(this, &GameMenuView::OpenSaveDialog));
		hbox->PackEnd(b);
		b = new Gui::LabelButton(new Gui::Label(Lang::LOAD_A_GAME));
		b->onClick.connect(sigc::mem_fun(this, &GameMenuView::OpenLoadDialog));
		b->SetShortcut(SDLK_l, KMOD_NONE);
		hbox->PackEnd(b);
		b = new Gui::LabelButton(new Gui::Label(Lang::EXIT_THIS_GAME));
		b->onClick.connect(sigc::mem_fun(this, &GameMenuView::HideAll));
		b->onClick.connect(sigc::ptr_fun(&Pi::EndGame));
		hbox->PackEnd(b);
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
		vbox->PackEnd(hbox);
		
		m_toggleHDR = new Gui::ToggleButton();
		m_toggleHDR->onChange.connect(sigc::mem_fun(this, &GameMenuView::OnToggleHDR));
		hbox = new Gui::HBox();
		hbox->SetSpacing(5.0f);
		hbox->PackEnd(m_toggleHDR);
		hbox->PackEnd(new Gui::Label(Lang::USE_HDR));
		vbox->PackEnd(hbox);
		if (!Render::IsHDRAvailable()) m_toggleHDR->SetEnabled(false);
		
		vbox->PackEnd((new Gui::Label(Lang::SOUND_SETTINGS))->Color(1.0f,1.0f,0.0f));
		m_masterVolume = new VolumeControl(Lang::VOL_MASTER, Pi::config.Float("MasterVolume"), Pi::config.Int("MasterMuted"));
		vbox->PackEnd(m_masterVolume);
		m_sfxVolume = new VolumeControl(Lang::VOL_EFFECTS, Pi::config.Float("SfxVolume"), Pi::config.Int("SfxMuted"));
		vbox->PackEnd(m_sfxVolume);
		m_musicVolume = new VolumeControl(Lang::VOL_MUSIC, Pi::config.Float("MusicVolume"), Pi::config.Int("MusicMuted"));
		vbox->PackEnd(m_musicVolume);

		m_masterVolume->onChanged.connect(sigc::mem_fun(this, &GameMenuView::OnChangeVolume));
		m_sfxVolume->onChanged.connect(sigc::mem_fun(this, &GameMenuView::OnChangeVolume));
		m_musicVolume->onChanged.connect(sigc::mem_fun(this, &GameMenuView::OnChangeVolume));
	}

	vbox->PackEnd((new Gui::Label(Lang::VIDEO_RESOLUTION))->Color(1.0f,1.0f,0.0f));

	m_screenModesGroup = new Gui::RadioGroup();
	SDL_Rect **modes;
	modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
	if ((modes!=0) && (modes != reinterpret_cast<SDL_Rect**>(-1))) {
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
		
		for (int i=0; modes[i]; ++i) {
			Gui::RadioButton *temp = new Gui::RadioButton(m_screenModesGroup);
			temp->onSelect.connect(sigc::bind(sigc::mem_fun(this,
					&GameMenuView::OnChangeVideoResolution), i));
			Gui::HBox *hbox = new Gui::HBox();
			hbox->SetSpacing(5.0f);
			hbox->PackEnd(temp);
			hbox->PackEnd(new Gui::Label(stringf(Lang::X_BY_X, formatarg("x", int(modes[i]->w)), formatarg("y", int(modes[i]->h)))));
			vbox2->PackEnd(hbox);
			if ((Pi::GetScrWidth() == modes[i]->w) && (Pi::GetScrHeight() == modes[i]->h)) {
				temp->SetSelected(true);
			}
		}
	}


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
	

	// language
	
	vbox = new Gui::VBox();
	vbox->SetSizeRequest(300, 200);
	mainTab->Add(vbox, 400, 250);

	vbox->PackEnd((new Gui::Label(Lang::LANGUAGE_SELECTION))->Color(1.0f,1.0f,0.0f));

	m_languageGroup = new Gui::RadioGroup();
	const std::list<std::string> availableLanguages = Lang::GetAvailableLanguages();

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
		
		for (std::list<std::string>::const_iterator i = availableLanguages.begin(); i != availableLanguages.end(); i++) {
			Gui::RadioButton *temp = new Gui::RadioButton(m_languageGroup);
			temp->onSelect.connect(sigc::bind(sigc::mem_fun(this, &GameMenuView::OnChangeLanguage), *i));
			Gui::HBox *hbox = new Gui::HBox();
			hbox->SetSpacing(5.0f);
			hbox->PackEnd(temp);
			hbox->PackEnd(new Gui::Label(*i));
			vbox2->PackEnd(hbox);
			if ((*i) == Pi::config.String("Lang"))
				temp->SetSelected(true);
		}
	}


	// key binding tab
	{
		Gui::Fixed *keybindingTab = new Gui::Fixed(800, 600);
		tabs->AddPage(new Gui::Label(Lang::CONTROLS), keybindingTab);

		Gui::VBox *box1 = new Gui::VBox();
		box1->SetSpacing(5.0f);
		keybindingTab->Add(box1, 10, 10);

		Gui::VBox *box2 = new Gui::VBox();
		box2->SetSpacing(5.0f);
		keybindingTab->Add(box2, 400, 10);

		Gui::VBox *box = box1;
		KeyGetter *keyg;

		for (int i=0; KeyBindings::bindingProtos[i].label; i++) {
			const char *label = KeyBindings::bindingProtos[i].label;
			const char *function = KeyBindings::bindingProtos[i].function;

			if (function) {
				KeyBindings::KeyBinding kb = KeyBindings::KeyBindingFromString(Pi::config.String(function));
				keyg = new KeyGetter(label, kb);
				keyg->onChange.connect(sigc::bind(sigc::mem_fun(this, &GameMenuView::OnChangeKeyBinding), function));
				box->PackEnd(keyg);
			} else {
				// section
				box->PackEnd((new Gui::Label(label))->Color(1.0f, 1.0f, 0.0f));
			}

			/* 2nd column */
			if (i == 20) {
				box = box2;
			}
		}

		for (int i=0; KeyBindings::axisBindingProtos[i].label; i++) {
			AxisGetter *axisg;
			const char *label = KeyBindings::axisBindingProtos[i].label;
			const char *function = KeyBindings::axisBindingProtos[i].function;

			if (function) {
				KeyBindings::AxisBinding ab = KeyBindings::AxisBindingFromString(Pi::config.String(function).c_str());
				axisg = new AxisGetter(label, ab);
				axisg->onChange.connect(sigc::bind(sigc::mem_fun(this, &GameMenuView::OnChangeAxisBinding), function));
				box->PackEnd(axisg);
			} else {
				// section
				box->PackEnd((new Gui::Label(label))->Color(1.0f, 1.0f, 0.0f));
			}

			/* 2nd column */
			if (i == 20) {
				box = box2;
			}
		}

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
	}
}

void GameMenuView::OnChangeKeyBinding(const KeyBindings::KeyBinding &kb, const char *fnName)
{
	Pi::config.SetString(fnName, KeyBindings::KeyBindingToString(kb).c_str());
	Pi::config.Save();
	KeyBindings::OnKeyBindingsChanged();
}

void GameMenuView::OnChangeAxisBinding(const KeyBindings::AxisBinding &ab, const char *function) {
	Pi::config.SetString(function, KeyBindings::AxisBindingToString(ab).c_str());
	Pi::config.Save();
	KeyBindings::OnKeyBindingsChanged();
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

	Pi::config.SetFloat("MasterVolume", masterVol);
	Pi::config.SetFloat("SfxVolume", sfxVol);
	Pi::config.SetFloat("MusicVolume", musVol);
	Pi::config.SetFloat("MasterMuted", m_masterVolume->IsMuted());
	Pi::config.SetFloat("SfxMuted", m_sfxVolume->IsMuted());
	Pi::config.SetFloat("MusicMuted", m_musicVolume->IsMuted());
	Pi::config.Save();
}
	
void GameMenuView::OnChangePlanetDetail(int level)
{
	if (level == Pi::detail.planets) return;
	m_changedDetailLevel = true;
	Pi::detail.planets = level;
	Pi::config.SetInt("DetailPlanets", level);
	Pi::config.Save();
}

void GameMenuView::OnChangePlanetTextures(int level)
{
	if (level == Pi::detail.textures) return;
	m_changedDetailLevel = true;
	Pi::detail.textures = level;
	Pi::config.SetInt("Textures", level);
	Pi::config.Save();
}
void GameMenuView::OnChangeFractalMultiple(int level)
{
	if (level == Pi::detail.fracmult) return;
	m_changedDetailLevel = true;
	Pi::detail.fracmult = level;
	Pi::config.SetInt("FractalMultiple", level);
	Pi::config.Save();
}

void GameMenuView::OnChangeCityDetail(int level)
{
	if (level == Pi::detail.cities) return;
	m_changedDetailLevel = true;
	Pi::detail.cities = level;
	Pi::config.SetInt("DetailCities", level);
	Pi::config.Save();
}

void GameMenuView::OnChangeLanguage(std::string &lang)
{
	Pi::config.SetString("Lang", lang.c_str());
	Pi::config.Save();
}

void GameMenuView::OnChangeVideoResolution(int res)
{
	SDL_Rect **modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
	Pi::config.SetInt("ScrWidth", modes[res]->w);
	Pi::config.SetInt("ScrHeight", modes[res]->h);
	Pi::config.Save();
}

void GameMenuView::OnToggleFullscreen(Gui::ToggleButton *b, bool state)
{
	Pi::config.SetInt("StartFullscreen", (state ? 1 : 0));
	Pi::config.Save();
//#ifndef _WIN32
	// XXX figure out how to do it in windows
//	SDL_WM_ToggleFullScreen(Pi::scrSurface);
//#endif
}

void GameMenuView::OnToggleShaders(Gui::ToggleButton *b, bool state)
{
	Pi::config.SetInt("DisableShaders", (state ? 0 : 1));
	Pi::config.Save();
	Render::ToggleShaders();
}

void GameMenuView::OnToggleHDR(Gui::ToggleButton *b, bool state)
{
	Pi::config.SetInt("EnableHDR", (state ? 1 : 0));
	Pi::config.Save();
	Render::ToggleHDR();
}

void GameMenuView::OnToggleJoystick(Gui::ToggleButton *b, bool state)
{
	Pi::config.SetInt("EnableJoystick", (state ? 1 : 0));
	Pi::config.Save();
	Pi::SetJoystickEnabled(state);
}

void GameMenuView::OnToggleMouseYInvert(Gui::ToggleButton *b, bool state)
{
	Pi::config.SetInt("InvertMouseY", (state ? 1 : 0));
	Pi::config.Save();
	Pi::SetMouseYInvert(state);
}

void GameMenuView::HideAll()
{
	if (m_changedDetailLevel) {
		Pi::OnChangeDetailLevel();
	}
	View::HideAll();
}

void GameMenuView::OpenSaveDialog()
{
	if (m_subview) delete m_subview;
	m_subview = new SaveDialogView;
	Pi::SetView(m_subview);
}

void GameMenuView::OpenLoadDialog()
{
	if (m_subview) delete m_subview;
	m_subview = new LoadDialogView;
	Pi::SetView(m_subview);
}

void GameMenuView::OnSwitchTo() {
	m_changedDetailLevel = false;
	if (m_subview) {
		delete m_subview;
		m_subview = 0;
	}
	// don't want to switch to this view if game not running
	if (!Pi::IsGameStarted()) {
		Pi::SetView(Pi::worldView);
	} else {
		m_planetDetailGroup->SetSelected(Pi::detail.planets);
		m_planetTextureGroup->SetSelected(Pi::detail.textures);
		m_planetFractalGroup->SetSelected(Pi::detail.fracmult);
		m_cityDetailGroup->SetSelected(Pi::detail.cities);
		m_toggleShaders->SetPressed(Render::AreShadersEnabled());
		m_toggleHDR->SetPressed(Render::IsHDREnabled());
		m_toggleFullscreen->SetPressed(Pi::config.Int("StartFullscreen") != 0);
		m_toggleJoystick->SetPressed(Pi::IsJoystickEnabled());
		m_toggleMouseYInvert->SetPressed(Pi::IsMouseYInvert());
	}
}
