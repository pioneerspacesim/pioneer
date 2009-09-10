#ifndef _WORLDVIEW_H
#define _WORLDVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"

class Body;
class Frame;
struct SBodyPath;

class WorldView: public View {
public:
	WorldView();
	virtual void Update();
	virtual void Draw3D();
	virtual void OnSwitchTo() {}
	static const float PICK_OBJECT_RECT_SIZE;
	void UpdateCommsOptions();
	bool GetShowLabels() { return m_labelsOn; }
	void DrawBgStars();
	vector3d GetExternalViewTranslation();
	void ApplyExternalViewRotation(matrix4x4d &m);
	virtual void Save();
	virtual void Load();
	enum CamType { CAM_FRONT, CAM_REAR, CAM_EXTERNAL };
	void SetCamType(enum CamType);
	enum CamType GetCamType() { return m_camType; }
	int GetNumLights() const { return m_numLights; }
	
	float m_externalViewRotX, m_externalViewRotY;
	float m_externalViewDist;
private:
	void DrawHUD(const Frame *cam_frame);
	void DrawTargetSquares();
	void DrawTargetSquare(const Body* const target);
	Gui::Button *AddCommsOption(const std::string msg, int ypos);
	void OnClickHyperspace();
	void OnClickBlastoff();
	void OnChangeWheelsState(Gui::MultiStateImageButton *b);
	void OnChangeLabelsState(Gui::MultiStateImageButton *b);
	void OnChangeFlightState(Gui::MultiStateImageButton *b);
	void OnChangeHyperspaceTarget();
	void OnPlayerDockOrUndock();
	virtual bool OnMouseDown(Gui::MouseButtonEvent *e);
	void SelectBody(Body *, bool reselectIsDeselect);
	Body* PickBody(const float screenX, const float screenY) const;
	Gui::ImageButton *m_hyperspaceButton;
	GLuint m_bgstarsVbo;
	Gui::Fixed *m_commsOptions;
	Gui::Label *m_flightStatus, *m_hyperTargetLabel;
	Gui::ImageButton *m_launchButton;
	Gui::MultiStateImageButton *m_flightControlButton;
	bool m_labelsOn;
	enum CamType m_camType;
	int m_numLights;
};

#endif /* _WORLDVIEW_H */
