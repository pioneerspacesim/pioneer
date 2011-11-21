#ifndef _LOADSAVEDIALOG_H
#define _LOADSAVEDIALOG_H

#include "libs.h"

class LoadDialog {
public:
	LoadDialog();
	void MainLoop();

private:
	void OnClickLoad(std::string filename);
	void OnClickBack();

	bool m_done;
};

#endif
