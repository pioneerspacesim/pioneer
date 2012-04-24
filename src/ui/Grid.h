#ifndef _UI_GRID_H
#define _UI_GRID_H

#include "Container.h"

namespace UI {

class Grid : public Container {
private:
	class CellSet {
	public:
		inline CellSet(float cp0) : numCells(1) {
			cellPercent[0] = cp0;
		}
		inline CellSet(float cp0, float cp1) : numCells(2) {
			cellPercent[0] = cp0; cellPercent[1] = cp1;
		}
		inline CellSet(float cp0, float cp1, float cp2) : numCells(3) {
			cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2;
		}
		inline CellSet(float cp0, float cp1, float cp2, float cp3) : numCells(4) {
			cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2; cellPercent[3] = cp3;
		}
		inline CellSet(float cp0, float cp1, float cp2, float cp3, float cp4) : numCells(5) {
			cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2; cellPercent[3] = cp3; cellPercent[4] = cp4;
		}
		inline CellSet(float cp0, float cp1, float cp2, float cp3, float cp4, float cp5) : numCells(6) {
			cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2; cellPercent[3] = cp3; cellPercent[4] = cp4; cellPercent[5] = cp5;
		}
		inline CellSet(float cp0, float cp1, float cp2, float cp3, float cp4, float cp5, float cp6) : numCells(7) {
			cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2; cellPercent[3] = cp3; cellPercent[4] = cp4; cellPercent[5] = cp5; cellPercent[6] = cp6;
		}
		inline CellSet(float cp0, float cp1, float cp2, float cp3, float cp4, float cp5, float cp6, float cp7) : numCells(8) {
			cellPercent[0] = cp0; cellPercent[1] = cp1; cellPercent[2] = cp2; cellPercent[3] = cp3; cellPercent[4] = cp4; cellPercent[5] = cp5; cellPercent[6] = cp6; cellPercent[7] = cp7;
		}

	private:
		int numCells;
		float cellPercent[8];
	};

public:
	class HCellSet : CellSet {};
	class VCellSet : CellSet {};

protected:
	friend class Context;
	Grid(Context *context, const HCellSet &horizCellSet);
	Grid(Context *context, const VCellSet &vertCellSet);
	Grid(Context *context, const HCellSet &horizCellSet, const VCellSet &vertCellSet);

public:
	virtual vector2f PreferredSize();
	virtual void Layout();

	void AddRow(const WidgetSet &set);
	void AddColumn(const WidgetSet &set);
};

}

#endif
