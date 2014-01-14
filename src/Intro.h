// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _INTRO_H
#define _INTRO_H

#include "Cutscene.h"
#include "Background.h"
#include "ShipType.h"
#include "scenegraph/ModelSkin.h"

class Intro : public Cutscene {
public:
	Intro(Graphics::Renderer *r, int width, int height);
	~Intro();
	virtual void Draw(float time);

private:
	void Reset(float time);
	bool m_needReset;

	std::vector<SceneGraph::Model*> m_models;
	SceneGraph::ModelSkin m_skin;

	float m_startTime;

	unsigned int m_modelIndex;
	float m_zoomBegin, m_zoomEnd;
	float m_dist;

	std::unique_ptr<Background::Container> m_background;

	int m_spinnerLeft;
	int m_spinnerWidth;
	float m_spinnerRatio;
};

#endif
