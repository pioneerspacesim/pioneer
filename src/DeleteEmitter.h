#ifndef _DELETEEMITTER_H
#define _DELETEEMITTER_H

#include "libs.h"

class DeleteEmitter {
public:
	virtual ~DeleteEmitter() {
		onDelete.emit();
	}
	sigc::signal<void> onDelete;
};

#endif
