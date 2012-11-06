// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_CELLSPEC_H
#define UI_CELLSPEC_H

#include <cassert>

struct lua_State;

namespace UI {

class CellSpec {
public:
	static const int MAX_CELLS = 8;

	CellSpec(int n) : numCells(n) {
		assert(n >= 1 && n < MAX_CELLS);
		for (int i = 0; i < MAX_CELLS; i++) cellPercent[i] = 1.0f/n;
	}

	inline CellSpec(float cp0, float cp1) : numCells(2) {
		cellPercent[0] = cp0; cellPercent[1] = cp1;
		NormaliseCells();
	}
	inline CellSpec(float cp0, float cp1, float cp2) : numCells(3) {
		cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2;
		NormaliseCells();
	}
	inline CellSpec(float cp0, float cp1, float cp2, float cp3) : numCells(4) {
		cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2; cellPercent[3] = cp3;
		NormaliseCells();
	}
	inline CellSpec(float cp0, float cp1, float cp2, float cp3, float cp4) : numCells(5) {
		cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2; cellPercent[3] = cp3; cellPercent[4] = cp4;
		NormaliseCells();
	}
	inline CellSpec(float cp0, float cp1, float cp2, float cp3, float cp4, float cp5) : numCells(6) {
		cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2; cellPercent[3] = cp3; cellPercent[4] = cp4; cellPercent[5] = cp5;
		NormaliseCells();
	}
	inline CellSpec(float cp0, float cp1, float cp2, float cp3, float cp4, float cp5, float cp6) : numCells(7) {
		cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2; cellPercent[3] = cp3; cellPercent[4] = cp4; cellPercent[5] = cp5; cellPercent[6] = cp6;
		NormaliseCells();
	}
	inline CellSpec(float cp0, float cp1, float cp2, float cp3, float cp4, float cp5, float cp6, float cp7) : numCells(8) {
		cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2; cellPercent[3] = cp3; cellPercent[4] = cp4; cellPercent[5] = cp5; cellPercent[6] = cp6; cellPercent[7] = cp7;
		NormaliseCells();
	}

	inline CellSpec(const float cp[], int n) : numCells(n) {
		assert(n >= 0 && n <= MAX_CELLS);
		for (int i = 0; i < n; i++) cellPercent[i] = cp[i];
		NormaliseCells();
	}

	static CellSpec FromLuaTable(lua_State *l, int idx);

	int numCells;
	float cellPercent[MAX_CELLS];

private:
	void NormaliseCells() {
		float total = 0.0f;
		for (int i = 0; i < numCells; ++i) { total += cellPercent[i]; }
		for (int i = 0; i < numCells; ++i) { cellPercent[i] /= total; }
	}
};

}

#endif
