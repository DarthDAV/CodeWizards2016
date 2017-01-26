#include "DavMap.h"

using namespace dav;

Map::Map(int _worldWidth, int _worldHeight) : worldWidth(_worldWidth), worldHeight(_worldHeight)
{
	rowsCount = worldHeight / CELL_SIZE;
	colsCount = worldWidth / CELL_SIZE;

	cells = new Cell *[rowsCount];
	for (size_t i = 0; i < rowsCount; i++)
	{
		cells[i] = new Cell[colsCount];
	}
}

Map::Cell * Map::cell(int row, int col)
{
	return &cells[row][col];
}

Map::Cell * Map::findCell(int worldX, int worldY)
{
	int col = worldX / CELL_SIZE;
	int row = worldY / CELL_SIZE;
	return cell(row, col);
}

void Map::clear()
{
	for (size_t i = 0; i < rowsCount; i++)
	{
		for (size_t j = 0; j < colsCount; j++)
		{
			cells[i][j].clear();
		}
	}

}

void Map::add(const model::Building & building)
{
	Cell * cell = findCell(building.getX(), building.getY());
	cell->isUnexplored = false;
	cell->buildings.push_back(&building);	
}

void Map::add(const model::Wizard & wizard)
{
	Cell * cell = findCell(wizard.getX(), wizard.getY());
	cell->isUnexplored = false;
	cell->wizards.push_back(&wizard);
}

void Map::add(const model::Minion & minion)
{
	Cell * cell = findCell(minion.getX(), minion.getY());
	cell->isUnexplored = false;
	cell->minions.push_back(&minion);
}

void Map::add(const model::Tree & tree)
{
	Cell * cell = findCell(tree.getX(), tree.getY());
	cell->isUnexplored = false;
	cell->trees.push_back(&tree);
}

Map::~Map()
{
	for (size_t i = 0; i < rowsCount; i++)
	{
		delete[] cells[i];
	}
	delete[] cells;
}

LocalMap::LocalMap()
{
	AroundCoord[drTop][0] = SELF_ROW - 1;
	AroundCoord[drTop][1] = SELF_COL;
	
	AroundCoord[drDiagTR][0] = SELF_ROW - 1;
	AroundCoord[drDiagTR][1] = SELF_COL + 1;

	AroundCoord[drRight][0] = SELF_ROW;
	AroundCoord[drRight][1] = SELF_COL + 1;

	AroundCoord[drDiagBR][0] = SELF_ROW + 1;
	AroundCoord[drDiagBR][1] = SELF_COL + 1;

	AroundCoord[drBottom][0] = SELF_ROW + 1;
	AroundCoord[drBottom][1] = SELF_COL;

	AroundCoord[drDiagBL][0] = SELF_ROW + 1;
	AroundCoord[drDiagBL][1] = SELF_COL - 1;

	AroundCoord[drLeft][0] = SELF_ROW;
	AroundCoord[drLeft][1] = SELF_COL - 1;

	AroundCoord[drDiagTL][0] = SELF_ROW - 1;
	AroundCoord[drDiagTL][1] = SELF_COL;

	for (int i = 0; i < _Direction_Count; ++i)
	{
		aroundCells[i] = &(cell((Direction)i));
	}

	clear();
}

void LocalMap::clear()
{
	for (size_t i = 0; i < ROWS_COUNT; i++)
	{
		for (size_t j = 0; j < COLS_COUNT; j++)
		{
			cells[i][j].content = ccUnknown;
			cells[i][j].unit = nullptr;
		}
	}

}

void LocalMap::init(const model::Wizard & self)
{
	clear();
	
	cells[SELF_ROW][SELF_COL].content = ccSelf;
	cells[SELF_ROW][SELF_COL].unit = &self;

	selfPos = Point2D(self);
	selfFaction = self.getFaction();
	zeroPos = selfPos.getShift(-1.0 * (CELL_SIZE*SELF_COL + CELL_SIZE / 2), -1.0 * (CELL_SIZE*SELF_ROW+ CELL_SIZE / 2));
}

void LocalMap::add(const model::CircularUnit & unit)
{
	Point2D pos(unit);	
	int col, row;
	
	if (!getCellPos(pos, row, col))
	{
		return;
	}

	CellContent content = ccUnknown;
	model::Faction faction = unit.getFaction();	
	if (faction == model::FACTION_NEUTRAL || faction == model::FACTION_OTHER)
	{
		content = ccNeutral;
	}
	else if (faction == selfFaction)
	{	
		content = ccAlly;
	}
	else
	{
		content = ccEnemy;
	}

	cells[row][col].content = content;
	cells[row][col].unit = &unit;
}

bool LocalMap::getCellPos(const Point2D & pos, int & row, int & col) const
{	
	double xShift = pos.getX() - zeroPos.getX();
	double yShift = pos.getY() - zeroPos.getY();

	if (xShift < 0 || xShift >= COLS_COUNT*CELL_SIZE)
	{
		return false;
	}
	else if (yShift < 0 || yShift >= ROWS_COUNT*CELL_SIZE)
	{
		return false;
	}

	row = yShift / CELL_SIZE;
	col = xShift / CELL_SIZE;
	return true;
}

bool LocalMap::cellToPos(int row, int col, Point2D & result) const
{
	double y = zeroPos.getY() + row * CELL_SIZE + CELL_SIZE / 2;
	double x = zeroPos.getX() + col * CELL_SIZE + CELL_SIZE / 2;

	result.setPosition(x, y);
	return true;//TODO position validation
}

bool LocalMap::normalize(const Point2D & pos, Point2D & normPos) const
{
	int col, row;
	if (!getCellPos(pos, row, col))
	{
		return false;
	}
	
	return cellToPos(row, col, normPos);
}

void LocalMap::fixing()
{
	//TODO Лишнее захватывает Все отрицательные координаты остаются ccUnknown
	//Остальное незанятое помечается как ccEmpty
	//TODO учесть радиус обзора

	double zeroX = zeroPos.getX();
	double zeroY = zeroPos.getY();

	for (size_t row = 0; row < ROWS_COUNT; row++)
	{
		double y = zeroY + row*CELL_SIZE;
		if (y < 0 || y > 4000.0)
		{
			continue;
		}
		
		for (size_t col = 0; col < COLS_COUNT; col++)
		{						
			double x = zeroX + col*CELL_SIZE;
			if (x < 0 || x > 4000)
			{
				continue;
			}		

			if (cells[row][col].content == ccUnknown)
			{
				cells[row][col].content = ccEmpty;
			}
		}
	}

	updateCollisions();
}

void LocalMap::updateCollisions()
{
	const model::CircularUnit & self = *(cells[SELF_ROW][SELF_COL].unit);
	for (int i = 0; i < _Direction_Count; ++i)
	{
		const model::CircularUnit * unit = cell((Direction)i).unit;
		if (unit == nullptr)
		{
			collisions[i] = false;
			continue;
		}

		collisions[i] = Object2D::isCollision(self, *unit);
	}
}

bool LocalMap::findFirstAround(CellContent needContent, int & row, int & col) const
{

	for (int i = 0; i < _Direction_Count; ++i)
	{
		row = AroundCoord[i][0];
		col = AroundCoord[i][1];
		
		if (cell(row, col).content == needContent)
		{
			return true;
		}
	}

	return false;
}


bool LocalMap::calcWay(Point2D & desiredEndPoint, std::vector<Point2D> & result) const
{
	result.clear();

	Point2D endPoint;
	if (!normalize(desiredEndPoint, endPoint))
	{
		return false;
	}

	Direction desiredDirection = calcDirection(endPoint);
	Direction direction = desiredDirection;

	Direction possibleDirections[_Direction_Count];
	possibleDirections[0] = desiredDirection;
	possibleDirections[1] = prevDirecton(possibleDirections[0]);
	possibleDirections[2] = nextDirecton(possibleDirections[0]);
	possibleDirections[3] = prevDirecton(possibleDirections[1]);
	possibleDirections[4] = nextDirecton(possibleDirections[2]);
	possibleDirections[5] = prevDirecton(possibleDirections[3]);
	possibleDirections[6] = nextDirecton(possibleDirections[4]);
	possibleDirections[7] = nextDirecton(possibleDirections[6]);

	for (int i = 0; i < _Direction_Count; ++i)
	{
		if (isCanMove(possibleDirections[i]))
		{
			direction = possibleDirections[i];
			break;
		}
	}
	
	Point2D point;
	if (!cellToPos(direction, point))
	{
		return false;
	}

	result.push_back(selfPos);
	result.push_back(point);
	result.push_back(endPoint);

	return true;
}

LocalMap::Direction LocalMap::prevDirecton(LocalMap::Direction direction) const
{
	if (direction == drTop)
	{
		return drDiagTL;
	}

	return LocalMap::Direction(direction - 1);
}

LocalMap::Direction LocalMap::nextDirecton(LocalMap::Direction direction) const
{
	if (direction == drDiagTL)
	{
		return drTop;
	}

	return LocalMap::Direction(direction + 1);
}

bool LocalMap::isCanMove(LocalMap::Direction direction) const
{
	Direction prev = prevDirecton(direction);
	Direction next = nextDirecton(direction);
	
	if (aroundCells[direction]->content != ccEmpty)
	{
		return false;
	}
	else if (collisions[direction] || collisions[prev] || collisions[next] || collisions[prevDirecton(prev)] || collisions[nextDirecton(next)])
	{
		return false;
	}
		
	return true;
	
	
	/*if (direction == drTop && around[direction]->content == ccEmpty)
	{
		if (around[drTop]->content == ccEmpty)
		{
			if (around[drRight] == ccEmpty && around[drDiagTR] == ccEmpty)
			{
				direction = drDiagTR;
			}
			else if (around[drLeft] == ccEmpty && around[drDiagTL] == ccEmpty)
			{
				direction = drDiagTL;
			}

			direction = Direction(rand() % _Direction_Count);
		}
	}


	if (around[drTop] == ccEmpty)
	{
		if (around[drRight] == ccEmpty && around[drDiagTR] == ccEmpty)
		{
			direction = drDiagTR;
		}
		else if (around[drLeft] == ccEmpty && around[drDiagTL] == ccEmpty)
		{
			direction = drDiagTL;
		}

		direction = Direction(rand() % _Direction_Count);
	}
	else if (around[drBottom] == ccEmpty)
	{
		if (around[drRight] == ccEmpty && around[drDiagBR] == ccEmpty)
		{
			direction = drDiagBR;
		}
		else if (around[drLeft] == ccEmpty && around[drDiagBL] == ccEmpty)
		{
			direction = drDiagBL;
		}

		direction = Direction(rand() % _Direction_Count);
	}
	else if (around[drRight] == ccEmpty)
	{
		direction = drRight;
	}
	else if (around[drLeft] == ccEmpty)
	{
		direction = drLeft;
	}
	else
	{
		direction = Direction(rand() % _Direction_Count);
	}*/


}


LocalMap::Direction LocalMap::calcDirection(const Point2D & endPoint) const
{
	double begX = selfPos.getX();
	double begY = selfPos.getY();
	double endX = endPoint.getX();
	double endY = endPoint.getY();
	
	if (begX == endX)
	{
		if (endY < begY)
		{
			return drTop;
		}
		else
		{
			return drBottom;
		}
	}
	else if(begY == endY)
	{
		if (endX > begX)
		{
			return drRight;
		}
		else
		{
			return drLeft;
		}
	}
	else if (endX > begX && endY < begY)
	{
		return drDiagTR;
	}
	else if (endX > begX && endY > begY)
	{
		return drDiagBR;
	}
	else if (endX < begX && endY < begY)
	{
		return drDiagTL;
	}
	else
	{
		return drDiagBL;
	}

}