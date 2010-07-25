#include "GameMenuView.h"
#include "Pi.h"
#include "Serializer.h"
#include "WorldView.h"
#include "ShipCpanel.h"
#include "Sound.h"

#if _GNU_SOURCE
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#elif _WIN32
#include <../msvc/win32-dirent.h>
#endif

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
		Gui::Screen::ShowBadError(stringf(128, "Could not open %s\n", name).c_str());
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
		PackEnd(new Gui::Label(m_title), false);
		m_tentry = new Gui::TextEntry();
		PackEnd(m_tentry, false);

		std::list<std::string> files;
		GetDirectoryContents(GetFullSavefileDirPath().c_str(), files);

		Gui::HBox *hbox = new Gui::HBox();
		hbox->SetSizeRequest(1,1);
		PackEnd(hbox, true);

		Gui::HBox *buttonBox = new Gui::HBox();
		buttonBox->SetSpacing(5.0f);
		Gui::Button *b = new Gui::LabelButton(new Gui::Label(m_type == SAVE ? "Save" : "Load"));
		b->onClick.connect(sigc::mem_fun(this, &FileDialog::OnClickAction));
		buttonBox->PackEnd(b, false);
		b = new Gui::LabelButton(new Gui::Label("Cancel"));
		b->onClick.connect(sigc::mem_fun(this, &FileDialog::OnClickCancel));
		buttonBox->PackEnd(b, false);
		PackEnd(buttonBox, false);


		Gui::VScrollBar *scroll = new Gui::VScrollBar();
		Gui::VScrollPortal *portal = new Gui::VScrollPortal(350,100);
		portal->SetTransparency(false);
		scroll->SetAdjustment(&portal->vscrollAdjust);
		hbox->PackEnd(portal, true);
		hbox->PackEnd(scroll, false);

		Gui::Box *vbox = new Gui::VBox();
		for (std::list<std::string>::iterator i = files.begin(); i!=files.end(); ++i) {
			Gui::Button *b = new SimpleLabelButton(new Gui::Label(*i));
			b->onClick.connect(sigc::bind(sigc::mem_fun(this, &FileDialog::OnClickFile), *i));
			vbox->PackEnd(b, false);
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
		m_fileDialog = new FileDialog(FileDialog::SAVE, "Select a file to save to or enter a new filename");
		f->Add(m_fileDialog, 0, 0);

		m_fileDialog->onClickCancel.connect(sigc::mem_fun(this, &SaveDialogView::OnClickBack));
		m_fileDialog->onClickAction.connect(sigc::mem_fun(this, &SaveDialogView::OnClickSave));
	}
	virtual void Update() {}
	virtual void Draw3D() {}
	virtual void OnSwitchTo() {}
private:
	void OnClickSave(std::string filename) {
		std::string fullname = join_path(GetFullSavefileDirPath().c_str(), filename.c_str(), 0);
		Serializer::SaveGame(fullname.c_str());
		Pi::cpan->MsgLog()->Message("", "Game saved to "+fullname);
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
		m_fileDialog = new FileDialog(FileDialog::LOAD, "Select a file to load");
		f->Add(m_fileDialog, 0, 0);

		m_fileDialog->onClickCancel.connect(sigc::mem_fun(this, &LoadDialogView::OnClickBack));
		m_fileDialog->onClickAction.connect(sigc::mem_fun(this, &LoadDialogView::OnClickLoad));
	}
	virtual void Update() {}
	virtual void Draw3D() {}
	virtual void OnSwitchTo() {}
private:
	void OnClickLoad(std::string filename) {
		std::string fullname = join_path(GetFullSavefileDirPath().c_str(), filename.c_str(), 0);
		Pi::UninitGame();
		Pi::InitGame();
		if (!Serializer::LoadGame(fullname.c_str())) abort();
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
	"Low", "Medium", "High", "Very high", "Very very high"
};

GameMenuView::GameMenuView(): View()
{
	m_subview = 0;

	Add((new Gui::Label("PIONEER"))->Shadow(true), 350, 10);
	SetTransparency(false);
	Gui::Label *l = new Gui::Label("PIONEER");
	l->Color(1,.7,0);
	m_rightRegion2->Add(l, 10, 0);
	
	{
		Gui::LabelButton *b;
		Gui::Box *hbox = new Gui::HBox();
		hbox->SetSpacing(5.0f);
		Add(hbox, 20, 50);
		b = new Gui::LabelButton(new Gui::Label("[S] Save the game"));
		b->SetShortcut(SDLK_s, KMOD_NONE);
		b->onClick.connect(sigc::mem_fun(this, &GameMenuView::OpenSaveDialog));
		hbox->PackEnd(b, false);
		b = new Gui::LabelButton(new Gui::Label("[L] Load a game"));
		b->onClick.connect(sigc::mem_fun(this, &GameMenuView::OpenLoadDialog));
		b->SetShortcut(SDLK_l, KMOD_NONE);
		hbox->PackEnd(b, false);
		b = new Gui::LabelButton(new Gui::Label("Exit this game"));
		b->onClick.connect(sigc::mem_fun(this, &GameMenuView::HideAll));
		b->onClick.connect(sigc::ptr_fun(&Pi::EndGame));
		hbox->PackEnd(b, false);
	}

	Gui::Box *vbox = new Gui::VBox();
	vbox->SetSizeRequest(300, 1000);
	vbox->SetSpacing(5.0);
	Add(vbox, 20, 100);

	vbox->PackEnd(new Gui::Label("Video resolution (restart game to apply)"), false);

	Gui::RadioGroup *g = new Gui::RadioGroup();
	SDL_Rect **modes;
	modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
	if ((modes!=0) && (modes != (SDL_Rect**)-1)) {
		for (int i=0; modes[i]; ++i) {
			Gui::RadioButton *temp = new Gui::RadioButton(g);
			temp->onSelect.connect(sigc::bind(sigc::mem_fun(this,
					&GameMenuView::OnChangeVideoResolution), i));
			Gui::HBox *hbox = new Gui::HBox();
			hbox->SetSpacing(5.0f);
			hbox->PackEnd(temp, false);
			hbox->PackEnd(new Gui::Label(stringf(256, "%dx%d", modes[i]->w, modes[i]->h)), false);
			vbox->PackEnd(hbox, false);
			if ((Pi::GetScrWidth() == modes[i]->w) && (Pi::GetScrHeight() == modes[i]->h)) {
				temp->SetSelected(true);
			}
		}
	}

	{
		vbox->PackEnd(new Gui::Label("Windowed or fullscreen (restart to apply)"), false);
		m_toggleFullscreen = new Gui::ToggleButton();
		m_toggleFullscreen->onChange.connect(sigc::mem_fun(this, &GameMenuView::OnToggleFullscreen));
		Gui::HBox *hbox = new Gui::HBox();
		hbox->SetSpacing(5.0f);
		hbox->PackEnd(m_toggleFullscreen, false);
		hbox->PackEnd(new Gui::Label("Full screen"), false);
		vbox->PackEnd(hbox, false);
		
		vbox->PackEnd(new Gui::Label("Other graphics settings"), false);
		m_toggleShaders = new Gui::ToggleButton();
		m_toggleShaders->onChange.connect(sigc::mem_fun(this, &GameMenuView::OnToggleShaders));
		hbox = new Gui::HBox();
		hbox->SetSpacing(5.0f);
		hbox->PackEnd(m_toggleShaders, false);
		hbox->PackEnd(new Gui::Label("Use shaders"), false);
		vbox->PackEnd(hbox, false);
		
		vbox->PackEnd(new Gui::Label("Sound settings"), false);
		m_sfxVolume = new Gui::Adjustment();
		m_sfxVolume->SetValue(Sound::GetGlobalVolume());
		m_sfxVolume->onValueChanged.connect(sigc::mem_fun(this, &GameMenuView::OnChangeVolume));
		Gui::HScale *sfxVol = new Gui::HScale();
		sfxVol->SetAdjustment(m_sfxVolume);
		hbox = new Gui::HBox();
		hbox->PackEnd(new Gui::Label("Sound effects volume: (min)"));
		hbox->PackEnd(sfxVol, false);
		hbox->PackEnd(new Gui::Label("(max)"));
		vbox->PackEnd(hbox, false);
	}


	vbox = new Gui::VBox();
	vbox->SetSpacing(5.0f);
	Add(vbox, 600, 100);

	vbox->PackEnd(new Gui::Label("Planet detail level:"));
	g = new Gui::RadioGroup();

	for (int i=0; i<5; i++) {
		m_planetDetail[i] = new Gui::RadioButton(g);
		m_planetDetail[i]->onSelect.connect(sigc::bind(sigc::mem_fun(this,
					&GameMenuView::OnChangePlanetDetail), i));
		Gui::HBox *hbox = new Gui::HBox();
		hbox->SetSpacing(5.0f);
		hbox->PackEnd(m_planetDetail[i], false);
		hbox->PackEnd(new Gui::Label(planet_detail_desc[i]), false);
		vbox->PackEnd(hbox, false);
	}
	// just a spacer
	vbox->PackEnd(new Gui::Fixed(10,20));
	
	vbox->PackEnd(new Gui::Label("City detail level:"));
	g = new Gui::RadioGroup();

	for (int i=0; i<5; i++) {
		m_cityDetail[i] = new Gui::RadioButton(g);
		m_cityDetail[i]->onSelect.connect(sigc::bind(sigc::mem_fun(this,
					&GameMenuView::OnChangeCityDetail), i));
		Gui::HBox *hbox = new Gui::HBox();
		hbox->SetSpacing(5.0f);
		hbox->PackEnd(m_cityDetail[i], false);
		hbox->PackEnd(new Gui::Label(planet_detail_desc[i]), false);
		vbox->PackEnd(hbox, false);
	}
}

void GameMenuView::OnChangeVolume()
{
	float sfxVol = m_sfxVolume->GetValue();
	Sound::SetGlobalVolume(sfxVol);
	Pi::config.SetFloat("SfxVolume", sfxVol);
	Pi::config.Save();
}
	
void GameMenuView::OnChangePlanetDetail(int level)
{
	m_changedDetailLevel = true;
	Pi::detail.planets = level;
	Pi::config.SetInt("DetailPlanets", level);
	Pi::config.Save();
}

void GameMenuView::OnChangeCityDetail(int level)
{
	m_changedDetailLevel = true;
	Pi::detail.cities = level;
	Pi::config.SetInt("DetailCities", level);
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
		m_planetDetail[Pi::detail.planets]->OnActivate();
		m_cityDetail[Pi::detail.cities]->OnActivate();
		m_toggleShaders->SetPressed(Render::AreShadersEnabled());
		m_toggleFullscreen->SetPressed(Pi::config.Int("StartFullscreen"));
	}
}

