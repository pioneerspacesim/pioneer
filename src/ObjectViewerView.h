// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OBJECTVIEWERVIEW_H
#define _OBJECTVIEWERVIEW_H

#include "Camera.h"
#include "View.h"

class Body;
class SystemBody;

class ObjectViewerView : public View {
public:
	ObjectViewerView();
	void Update() override;
	void Draw3D() override;

protected:
	void OnSwitchTo() override;

	void DrawPiGui() override;

private:
	void ReloadState();
	void OnChangeTerrain();

	void DrawInfoWindow();
	void DrawControlsWindow();

	Body *m_targetBody;
	const SystemBody *m_systemBody;
	bool m_isTerrainBody;

	struct ControlState {
		uint32_t seed;
		double mass;
		double radius;
		double life;
		double volatileGas;
		double volatileIces;
		double volatileLiquid;
		double metallicity;
		double volcanicity;
	} m_state;

	float viewingDist;
	matrix4x4d m_camRot;

	RefCountedPtr<CameraContext> m_cameraContext;
	std::unique_ptr<Camera> m_camera;
};

#endif /* _OBJECTVIEWERVIEW_H */
