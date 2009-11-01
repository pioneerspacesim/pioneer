#include "GameMenuView.h"
#include "Pi.h"
#include "Serializer.h"
#include "WorldView.h"
#include "ShipCpanel.h"

#if _GNU_SOURCE
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#elif _WIN32
#include <shlobj.h>
#include <shlwapi.h>
#include <atlbase.h>
#endif

/*
 * Must create the folders if they do not exist already.
 */
std::string GetFullSavefileDirPath()
{
	// i think this test only works with glibc...
#if _GNU_SOURCE
	const char *homedir = getenv("HOME");
	std::string path = join_path(homedir, ".pioneer", 0);
	DIR *dir = opendir(path.c_str());
	if (!dir) {
		if (mkdir(path.c_str(), 0770) == -1) {
			Gui::Screen::ShowBadError(stringf(128, "Error: Could not create or open '%s'.", path.c_str()).c_str());
		}
	}
	closedir(dir);
	path = join_path(homedir, ".pioneer", "savefiles", 0);
	dir = opendir(path.c_str());
	if (!dir) {
		if (mkdir(path.c_str(), 0770) == -1) {
			Gui::Screen::ShowBadError(stringf(128, "Error: Could not create or open '%s'.", path.c_str()).c_str());
		}
	}
	closedir(dir);
	return path;
#elif _WIN32
	try {
		TCHAR path[MAX_PATH];
		if(S_OK != SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path))
			throw std::runtime_error("SHGetFolderPath");

		if(!PathAppend(path, CA2T("Pioneer")))
			throw std::runtime_error("PathAppend");

		if(!PathFileExists(path) && ERROR_SUCCESS != SHCreateDirectoryEx(0, path, 0))
			throw std::runtime_error("SHCreateDirectoryEx");

		return std::string(CT2A(path));
	}
	catch(const std::exception&) {
		Gui::Screen::ShowBadError("Can't get path to save directory");
		return "";
	}
#else
# error Unsupported system
#endif
}

/* Not dirs, not . or .. */
static void GetDirectoryContents(const char *name, std::list<std::string> &files)
{
#if _GNU_SOURCE
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
#elif _WIN32
	try {
		WIN32_FIND_DATA fd;
		const HANDLE hFind = FindFirstFile(CA2T((std::string(name) + "/*").c_str()), &fd);
		if(INVALID_HANDLE_VALUE == hFind)
			throw std::runtime_error("FindFirstFile");
		do {
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;
			files.push_back(std::string(CT2A(fd.cFileName)));
		}
		while(FindNextFile(hFind, &fd) != 0);
	}
	catch(const std::exception&) {
		Gui::Screen::ShowBadError("Error getting directory contents");
	}
#else
# error Unsupported turd
#endif
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
		Serializer::Write::Game(fullname.c_str());
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
		Serializer::Read::Game(fullname.c_str());
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
	
	Gui::LabelButton *b;

	Gui::Box *vbox = new Gui::VBox();
	vbox->SetSpacing(5.0);
	Add(vbox, 5, 100);

	b = new Gui::LabelButton(new Gui::Label("[S] Save the game"));
	b->SetShortcut(SDLK_s, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &GameMenuView::OpenSaveDialog));
	vbox->PackEnd(b, false);
	b = new Gui::LabelButton(new Gui::Label("[L] Load a game"));
	b->onClick.connect(sigc::mem_fun(this, &GameMenuView::OpenLoadDialog));
	b->SetShortcut(SDLK_l, KMOD_NONE);
	vbox->PackEnd(b, false);
	b = new Gui::LabelButton(new Gui::Label("Exit this game"));
	b->onClick.connect(sigc::mem_fun(this, &GameMenuView::HideAll));
	b->onClick.connect(sigc::ptr_fun(&Pi::EndGame));
	vbox->PackEnd(b, false);

	vbox = new Gui::VBox();
	vbox->SetSpacing(5.0f);
	Add(vbox, 400, 100);

	vbox->PackEnd(new Gui::Label("Planet detail level:"));
	Gui::RadioGroup *g = new Gui::RadioGroup();

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
}
	
void GameMenuView::OnChangePlanetDetail(int level)
{
	m_changedDetailLevel = true;
	Pi::detail.planets = level;
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
	}
}

