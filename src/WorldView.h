// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _WORLDVIEW_H
#define _WORLDVIEW_H

#include "gui/GuiWidget.h"
#include "pigui/PiGuiView.h"
#include "ship/ShipViewController.h"

class Body;
class Camera;
class SpeedLines;
class NavTunnelWidget;
class Game;

enum VelIconType {
	V_PROGRADE,
	V_RETROGRADE,
	V_BURN
};

enum PlaneType {
	NONE,
	ROTATIONAL,
	PARENT
};

namespace Gui {
	class TexturedQuad;
}

class WorldView : public PiGuiView {
public:
	static void RegisterInputBindings();
	friend class NavTunnelWidget;
	WorldView(Game *game);
	WorldView(const Json &jsonObj, Game *game);
	virtual ~WorldView();

	void Update() override;
	void Draw3D() override;
	void Draw() override;
	void SaveToJson(Json &jsonObj) override;

	RefCountedPtr<CameraContext> GetCameraContext() const { return m_cameraContext; }

	ViewController *GetViewController() const { return m_viewController; }
	void SetViewController(ViewController *newView);

	std::unique_ptr<ShipViewController> shipView;

	int GetActiveWeapon() const;

	std::tuple<double, double, double> CalculateHeadingPitchRoll(enum PlaneType);

	vector3d WorldSpaceToScreenSpace(const Body *body) const;
	vector3d WorldSpaceToScreenSpace(const vector3d &position) const;
	vector3d RelSpaceToScreenSpace(const vector3d &position) const;
	vector3d ShipSpaceToScreenSpace(const vector3d &position) const;
	vector3d GetTargetIndicatorScreenPosition(const Body *body) const;
	vector3d CameraSpaceToScreenSpace(const vector3d &pos) const;

	void BeginCameraFrame() { m_cameraContext->BeginFrame(); };
	void EndCameraFrame() { m_cameraContext->EndFrame(); };

	bool ShouldShowLabels() { return m_labelsOn; }

protected:
	void OnSwitchTo() override;
	void OnSwitchFrom() override;

private:
	void InitObject();

	enum IndicatorSide {
		INDICATOR_HIDDEN,
		INDICATOR_ONSCREEN,
		INDICATOR_LEFT,
		INDICATOR_RIGHT,
		INDICATOR_TOP,
		INDICATOR_BOTTOM
	};

	struct Indicator {
		vector2f pos;
		vector2f realpos;
		IndicatorSide side;
		Indicator() :
			pos(0.0f, 0.0f),
			realpos(0.0f, 0.0f),
			side(INDICATOR_HIDDEN)
		{}
	};

	void UpdateProjectedObjects();
	void UpdateIndicator(Indicator &indicator, const vector3d &direction);
	void HideIndicator(Indicator &indicator);

	void OnToggleLabels();

	void DrawCombatTargetIndicator(const Indicator &target, const Indicator &lead, const Color &c);
	void DrawEdgeMarker(const Indicator &marker, const Color &c);

	/// Handler for "requestTimeAccelerationInc" event
	void OnRequestTimeAccelInc();
	/// Handler for "requestTimeAccelerationDec" event
	void OnRequestTimeAccelDec();
	void SelectBody(Body *, bool reselectIsDeselect);

	Game *m_game;
	ViewController *m_viewController;

	NavTunnelWidget *m_navTunnel;
	std::unique_ptr<SpeedLines> m_speedLines;

	bool m_labelsOn;

	sigc::connection m_onToggleHudModeCon;
	sigc::connection m_onIncTimeAccelCon;
	sigc::connection m_onDecTimeAccelCon;

	RefCountedPtr<CameraContext> m_cameraContext;
	std::unique_ptr<Camera> m_camera;

	Indicator m_combatTargetIndicator;
	Indicator m_targetLeadIndicator;

	Graphics::RenderState *m_blendState;

	Graphics::Drawables::Line3D m_edgeMarker;
	Graphics::Drawables::Lines m_indicator;

	struct InputBinding : public Input::InputFrame {
		using InputFrame::InputFrame;

		Action *toggleHudMode;
		Action *increaseTimeAcceleration;
		Action *decreaseTimeAcceleration;

		void RegisterBindings() override;
	} InputBindings;
};

class NavTunnelWidget : public Gui::Widget {
public:
	NavTunnelWidget(WorldView *worldView, Graphics::RenderState *);
	virtual void Draw();
	virtual void GetSizeRequested(float size[2]);
	void DrawTargetGuideSquare(const vector2f &pos, const float size, const Color &c);

private:
	void CreateVertexBuffer(const Uint32 size);

	WorldView *m_worldView;
	Graphics::RenderState *m_renderState;
	RefCountedPtr<Graphics::Material> m_material;
	std::unique_ptr<Graphics::VertexBuffer> m_vbuffer;
};

#endif /* _WORLDVIEW_H */
