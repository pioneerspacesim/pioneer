#ifndef _WORLDVIEW_H
#define _WORLDVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"

class Body;

class WorldView: public View {
public:
	WorldView();
	virtual void Update();
	virtual void Draw3D();
	static const float PICK_OBJECT_RECT_SIZE;
	void UpdateCommsOptions();
	bool GetShowLabels() { return labelsOn; }
	void DrawBgStars();
private:
	Gui::Button *AddCommsOption(const std::string msg, int ypos);
	void OnClickHyperspace();
	void OnClickBlastoff();
	void OnChangeWheelsState(Gui::MultiStateImageButton *b);
	void OnChangeLabelsState(Gui::MultiStateImageButton *b);
	virtual bool OnMouseDown(Gui::MouseButtonEvent *e);
	Body* PickBody(const float screenX, const float screenY) const;
	Gui::ImageButton *m_hyperspaceButton;
	GLuint m_bgstarsDlist;
	Gui::Fixed *commsOptions;
	Gui::Label *flightStatus;
	Gui::ImageButton *launchButton;
	bool labelsOn;
};

#endif /* _WORLDVIEW_H */
