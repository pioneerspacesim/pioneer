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
	vector3d GetExternalViewTranslation();
	void ApplyExternalViewRotation(matrix4x4d &m);
	virtual void Save();
	virtual void Load();
	enum CamType { CAM_FRONT, CAM_REAR, CAM_EXTERNAL };
	void SetCamType(enum CamType);
	enum CamType GetCamType() { return m_camType; }
	
	float m_externalViewRotX, m_externalViewRotY;
	float m_externalViewDist;
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
	enum CamType m_camType;
};

#endif /* _WORLDVIEW_H */
