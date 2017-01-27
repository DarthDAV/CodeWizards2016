#include "DavMap.h"

#include <cstring>

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
	aroundShift[drTop][0] = -1;
	aroundShift[drTop][1] = 0;
	aroundCoord[drTop][0] = SELF_ROW - 1;
	aroundCoord[drTop][1] = SELF_COL;
	
	aroundShift[drDiagTR][0] = -1;
	aroundShift[drDiagTR][1] = 1;
	aroundCoord[drDiagTR][0] = SELF_ROW - 1;
	aroundCoord[drDiagTR][1] = SELF_COL + 1;

	aroundShift[drRight][0] = 0;
	aroundShift[drRight][1] = 1;
	aroundCoord[drRight][0] = SELF_ROW;
	aroundCoord[drRight][1] = SELF_COL + 1;

	aroundShift[drDiagBR][0] = 1;
	aroundShift[drDiagBR][1] = 1;
	aroundCoord[drDiagBR][0] = SELF_ROW + 1;
	aroundCoord[drDiagBR][1] = SELF_COL + 1;

	aroundShift[drBottom][0] = 1;
	aroundShift[drBottom][1] = 0;
	aroundCoord[drBottom][0] = SELF_ROW + 1;
	aroundCoord[drBottom][1] = SELF_COL;

	aroundShift[drDiagBL][0] = 1;
	aroundShift[drDiagBL][1] = -1;
	aroundCoord[drDiagBL][0] = SELF_ROW + 1;
	aroundCoord[drDiagBL][1] = SELF_COL - 1;

	aroundShift[drLeft][0] = 0;
	aroundShift[drLeft][1] = -1;
	aroundCoord[drLeft][0] = SELF_ROW;
	aroundCoord[drLeft][1] = SELF_COL - 1;

	aroundShift[drDiagTL][0] = -1;
	aroundShift[drDiagTL][1] = -1;
	aroundCoord[drDiagTL][0] = SELF_ROW - 1;
	aroundCoord[drDiagTL][1] = SELF_COL - 1;

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

	nearestEnemyDistance = 4000;
	nearestAllyDistance = 4000;

	nearestEnemy = nullptr;
	nearestAlly = nullptr;

	enemies.clear();
	allies.clear();
	nearUnits.clear();

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
		double distance = selfPos.getDistanceTo(unit);
		if (distance < nearestAllyDistance)
		{
			nearestAllyDistance = distance;
			nearestAlly = &unit;
		}
		allies.push_back(&unit);
	}
	else
	{
		content = ccEnemy;
		double distance = selfPos.getDistanceTo(unit);
		if (distance < nearestEnemyDistance)
		{
			nearestEnemyDistance = distance;
			nearestEnemy = &unit;
		}
		enemies.push_back(&unit);
	}

	cells[row][col].content = content;
	cells[row][col].unit = &unit;

	if (selfPos.getDistanceTo(unit) <= NEAR_DISTANCE)
	{
		nearUnits.push_back(&unit);
	}
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

}

bool LocalMap::findFirstAround(CellContent needContent, int & row, int & col) const
{

	for (int i = 0; i < _Direction_Count; ++i)
	{
		row = aroundCoord[i][0];
		col = aroundCoord[i][1];
		
		if (cell(row, col).content == needContent)
		{
			return true;
		}
	}

	return false;
}


bool LocalMap::calcWay(const Point2D & desiredEndPoint, std::vector<Point2D> & result) const
{
	result.clear();

	Point2D endPoint;//Окончание пути с учётом свободного места
	if (!getNearestPlace(desiredEndPoint, endPoint))
	{
		return false;
	}

	Direction desiredDirection = calcDirection(selfPos, endPoint);
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

	bool succes = false;
	for (int i = 0; i < _Direction_Count; ++i)
	{
		if (isCanMoveDirect(possibleDirections[i]))
		{
			direction = possibleDirections[i];
			succes = true;
			break;
		}
	}

	if (!succes)
	{
		direction = Direction(std::rand() % _Direction_Count);//TODO
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

bool LocalMap::getNearestPlace(const Point2D & desiredEndPoint, Point2D & result) const
{
	int cRow, cCol;
	if (!getCellPos(desiredEndPoint, cRow, cCol))
	{
		return false;
	}

	LocalMap::CellContent content = cell(cRow, cCol).content;
	if (content == LocalMap::ccEmpty)
	{
		result = desiredEndPoint;
		return true;
	}

	int nRow, nCol;
	for (int i = 0; i < _Direction_Count; ++i)
	{
		if (!getNearCell(cRow, cCol, Direction(i), nRow, nCol))
		{
			continue;
		}

		if (cell(nRow, nCol).content != ccEmpty)
		{
			continue;
		}

		return cellToPos(nRow, nCol, result);

	}

	return false;
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

const bool LocalMap::isDirectMovePossible(const Point2D & targetPoint) const
{
	if (!nearUnits.size())
	{
		return true;
	}

	Direction direction = calcDirection(selfPos, targetPoint);

	std::vector<const model::CircularUnit *> collisionUnits;
	if (isPossibleCollision(selfPos, collisionUnits))
	{
		bool collisions[_Direction_Count];
		convertCollisions(selfPos, collisionUnits, collisions);

		Direction prev = prevDirecton(direction);
		Direction next = nextDirecton(direction);

		if (collisions[direction] || collisions[prev] || collisions[next] || collisions[prevDirecton(prev)] || collisions[nextDirecton(next)])
		{
			return false;
		}
	}

	if (isPossibleCollision(targetPoint, collisionUnits))
	{
		return false;
	}
	
	double x = (selfPos.getX() + targetPoint.getX()) / 2.0;
	double y = (selfPos.getY() + targetPoint.getY()) / 2.0;
	Point2D midPoint(x, y);
	if (isPossibleCollision(midPoint, collisionUnits))
	{
		return false;
	}

	return true;

}

bool LocalMap::isCanMoveDirect(LocalMap::Direction direction) const
{
	if (aroundCells[direction]->content != ccEmpty)
	{
		return false;
	}

	if (!nearUnits.size())
	{
		return true;
	}

	std::vector<const model::CircularUnit *> collisionUnits;
	if (isPossibleCollision(selfPos, collisionUnits))
	{
		bool collisions[_Direction_Count];
		convertCollisions(selfPos, collisionUnits, collisions);

		Direction prev = prevDirecton(direction);
		Direction next = nextDirecton(direction);

		if (collisions[direction] || collisions[prev] || collisions[next] || collisions[prevDirecton(prev)] || collisions[nextDirecton(next)])
		{
			return false;
		}
	}

	Point2D endPoint;
	cellToPos(direction, endPoint);
	if (isPossibleCollision(endPoint, collisionUnits))
	{
		return false;
	}

	Point2D midPoint = selfPos.getShift(aroundShift[direction][1] * CELL_SIZE  / 2.0, aroundShift[direction][0] * CELL_SIZE / 2.0);
	if (isPossibleCollision(midPoint, collisionUnits))
	{
		return false;
	}
	
	return true;	
}

bool LocalMap::isPossibleCollision(const Point2D & point, std::vector<const model::CircularUnit *> & result) const
{
	result.clear();

	Object2D object(point, COLLISION_RISK + CELL_SIZE / 2.0);

	for (int i = 0; i < nearUnits.size(); ++i)
	{
		const model::CircularUnit * unit = nearUnits[i];
		
		if(object.isCollision(*unit))
		{ 
			result.push_back(unit);
		}
	}

	return result.size();
}

void LocalMap::convertCollisions(const Point2D & position, const std::vector<const model::CircularUnit *> & positionCollisionUnits, bool * collisions) const
{
	std::memset(collisions, 0, _Direction_Count*sizeof(bool));
	for (int i = 0; i < positionCollisionUnits.size(); ++i)
	{
		const model::CircularUnit * unit = positionCollisionUnits[i];
		Direction direction = calcDirection(position, Point2D(*unit));//TODO
		collisions[direction] = true;
	}

}

LocalMap::Direction LocalMap::calcDirection(const Point2D & beginPoint, const Point2D & endPoint) const
{
	double begX = beginPoint.getX();
	double begY = beginPoint.getY();
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

bool LocalMap::getNearCell(int midRow, int midCol, LocalMap::Direction direction, int &resultRow, int &resultCol) const
{
	if (!isValid(midRow, midCol))
	{
		return false;
	}
	
	resultRow = midRow + aroundShift[direction][0];
	resultCol = midCol + aroundShift[direction][1];

	if (!isValid(resultRow, resultCol))
	{
		return false;
	}
			
	return true;
}

