// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPCPANELMULTIFUNCDISPLAYS_H
#define _SHIPCPANELMULTIFUNCDISPLAYS_H

#include "gui/Gui.h"
#include "Serializer.h"
#include "Object.h"
#include "json/json.h"

class Body;
namespace Graphics { class Renderer; }

enum multifuncfunc_t {
	MFUNC_RADAR,
	MFUNC_EQUIPMENT,
	MFUNC_MAX
};

class IMultiFunc {
public:
	sigc::signal<void> onGrabFocus;
	sigc::signal<void> onUngrabFocus;
	virtual void Update() = 0;
};

class RadarWidget: public IMultiFunc, public Gui::Widget {
public:
	RadarWidget(Graphics::Renderer *r);
	RadarWidget(Graphics::Renderer *r, const Json::Value &jsonObj);
	virtual ~RadarWidget();
	void GetSizeRequested(float size[2]);
	void ToggleMode();
	void InitScaling(void);
	void Draw();
	virtual void Update();

	void TimeStepUpdate(float step);

	void SaveToJson(Json::Value &jsonObj);

private:
	void InitObject();

	void DrawBlobs(bool below);
	void GenerateBaseGeometry();
	void GenerateRingsAndSpokes();
	void DrawRingsAndSpokes(bool blend);

	sigc::connection m_toggleScanModeConnection;

	struct Contact {
		Object::Type type;
		vector3d pos;
		bool isSpecial;
	};
	std::list<Contact> m_contacts;
	Graphics::Drawables::Lines m_contactLines;
	Graphics::Drawables::Points m_contactBlobs;

	enum RadarMode { RADAR_MODE_AUTO, RADAR_MODE_MANUAL };
	RadarMode m_mode;

	float m_currentRange, m_manualRange, m_targetRange;
	float m_scale;

	float m_x;
	float m_y;

	float m_lastRange;
	float RADAR_XSHRINK;
	float RADAR_YSHRINK;

	std::vector<vector3f> m_circle;
	std::vector<vector3f> m_spokes;
	std::vector<vector3f> m_vts;
	std::vector<vector3f> m_edgeVts;
	std::vector<Color> m_edgeCols;

	Graphics::Renderer *m_renderer;
	Graphics::RenderState *m_renderState;

	Graphics::Drawables::Lines m_scanLines;
	Graphics::Drawables::Lines m_edgeLines;
};

#endif /* _SHIPCPANELMULTIFUNCDISPLAYS_H */
