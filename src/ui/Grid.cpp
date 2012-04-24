#include "Grid.h"

namespace UI {

Grid::Grid(Context *context, const HCellSet &horizCellSet) : Container(context)
{
}

Grid::Grid(Context *context, const VCellSet &vertCellSet) : Container(context)
{
}

Grid::Grid(Context *context, const HCellSet &horizCellSet, const VCellSet &vertCellSet) : Container(context)
{
}

vector2f Grid::PreferredSize()
{
	return 0;
}

void Grid::Layout()
{
}

void Grid::AddRow(const WidgetSet &set)
{
}

void Grid::AddColumn(const WidgetSet &set)
{
}

}
