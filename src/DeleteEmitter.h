#ifndef _DELETEEMITTER_H
#define _DELETEEMITTER_H

#include "libs.h"

class DeleteEmitter {
public:
	DeleteEmitter() {}
	virtual ~DeleteEmitter() {
		onDelete.emit();
	}
	sigc::signal<void> onDelete;

private:
	DeleteEmitter(const DeleteEmitter &) {}
};

#endif
