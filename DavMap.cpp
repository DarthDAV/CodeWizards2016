#include "DavMap.h"

#include <cstring>
#include <cstring>
#include <algorithm>
#include <limits>

#ifdef DEBUG_MAP
#include <iostream>
#include <fstream>
#endif

using namespace dav;

DirectionHelper::DirectionHelper()
{
	aroundShift[drTop][0] = -1;
	aroundShift[drTop][1] = 0;

	aroundShift[drDiagTR][0] = -1;
	aroundShift[drDiagTR][1] = 1;

	aroundShift[drRight][0] = 0;
	aroundShift[drRight][1] = 1;

	aroundShift[drDiagBR][0] = 1;
	aroundShift[drDiagBR][1] = 1;

	aroundShift[drBottom][0] = 1;
	aroundShift[drBottom][1] = 0;

	aroundShift[drDiagBL][0] = 1;
	aroundShift[drDiagBL][1] = -1;

	aroundShift[drLeft][0] = 0;
	aroundShift[drLeft][1] = -1;

	aroundShift[drDiagTL][0] = -1;
	aroundShift[drDiagTL][1] = -1;
}

LocalMap::LocalMap() 
{
	pathfinding = new Pathfinding(this);

	aroundCoord[drTop][0] = SELF_ROW - 1;
	aroundCoord[drTop][1] = SELF_COL;
	aroundCoord[drDiagTR][0] = SELF_ROW - 1;
	aroundCoord[drDiagTR][1] = SELF_COL + 1;
	aroundCoord[drRight][0] = SELF_ROW;
	aroundCoord[drRight][1] = SELF_COL + 1;
	aroundCoord[drDiagBR][0] = SELF_ROW + 1;
	aroundCoord[drDiagBR][1] = SELF_COL + 1;
	aroundCoord[drBottom][0] = SELF_ROW + 1;
	aroundCoord[drBottom][1] = SELF_COL;
	aroundCoord[drDiagBL][0] = SELF_ROW + 1;
	aroundCoord[drDiagBL][1] = SELF_COL - 1;
	aroundCoord[drLeft][0] = SELF_ROW;
	aroundCoord[drLeft][1] = SELF_COL - 1;
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

	nearestEnemyDistance = std::numeric_limits<double>::max();
	nearestAllyDistance = std::numeric_limits<double>::max();
	nearestEnemy = nullptr;
	nearestAlly = nullptr;

	enemies.clear();
	allies.clear();
	nearUnits.clear();
}

LocalMap::~LocalMap()
{
	delete pathfinding;
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

void LocalMap::add(const model::LivingUnit & unit)
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

	Cell & cell = cells[row][col];

	cell.content = content;
	cell.unit = &unit;

	if (selfPos.getDistanceTo(unit) <= NEAR_DISTANCE)
	{
		nearUnits.push_back(&unit);
	}

	setPartialAround(row, col);
}

void LocalMap::setPartialAround(int row, int col)
{
	//TODO �����
	const model::CircularUnit * unit = cells[row][col].unit;

	if (unit->getRadius() >= CELL_SIZE)
	{
		int aRow, aCol;
		for (int rsh = -2; rsh <= 2; ++rsh)
		{
			aRow = row + rsh;
			for (int csh = -2; csh <= 2; ++csh)
			{
				aCol = col + csh;
				if (!isValid(aRow, aCol))
				{
					continue;
				}

				Cell & cell = cells[aRow][aCol];
				if (cell.content != ccUnknown)
				{
					continue;
				}

				Point2D point;
				if (!cellToPos(aRow, aCol, point))
				{
					continue;
				}

				double distance = point.getDistanceTo(*unit);
				if (distance < (unit->getRadius() + CELL_SIZE / 2.0 + COLLISION_RISK))
				{
					cell.content = ccPartialEmpty;
				}

			}
		}

		return;
	}
	
	for (int i = 0; i < _Direction_Count; ++i)
	{
		int aRow, aCol;
		dh.getShift(row, col, Direction(i), aRow, aCol);
		if (!isValid(aRow, aCol))
		{
			continue;
		}

		Cell & cell = cells[aRow][aCol];
		if (cell.content != ccUnknown)
		{
			continue;
		}

		Point2D point;
		if (!cellToPos(aRow, aCol, point))
		{
			continue;
		}
		
		double distance = point.getDistanceTo(*unit);
		if (distance < (unit->getRadius() + CELL_SIZE / 2.0 + COLLISION_RISK))
		{
			cell.content = ccPartialEmpty;
		}

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
	return isValid(row, col);
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
	double zeroX = zeroPos.getX();
	double zeroY = zeroPos.getY();

	for (size_t row = 0; row < ROWS_COUNT; row++)
	{
		double y = zeroY + row*CELL_SIZE;
		if (y < 0 || y >= 3930)
		{
			continue;
		}
		
		for (size_t col = 0; col < COLS_COUNT; col++)
		{						
			double x = zeroX + col*CELL_SIZE;
			if (x < 0 || x >= 3930)
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

#ifdef DEBUG_MAP
bool LocalMap::saveToFile() const
{
	char textMap[21][22];//TODO
	for (size_t row = 0; row < ROWS_COUNT; row++)
	{
		for (size_t col = 0; col < COLS_COUNT; col++)
		{
			char text;

			text = getText(cells[row][col].content);			
			textMap[row][col] = text;
		}

		textMap[row][21] = 0;
	}

	std::vector<LocalMap::Coordinates> path;
	pathfinding->getPath(path);

	for (int i = 0; i < path.size(); ++i)
	{
		const Coordinates & coord = path[i];
		if (SELF_ROW == coord.row && SELF_COL == coord.col)
		{
			continue;
		}

		if (i == path.size() - 1)
		{
			textMap[coord.row][coord.col] = 'T';
		}
		else
		{
			textMap[coord.row][coord.col] = 'X';
		}
	}
	
	std::string fileName = "map.txt";
	std::ofstream file(fileName, std::ios_base::out | std::ios_base::app);
	if (!file.is_open())
	{
		return false;
	}
	
	file << "                     \n";
	file << "---------------------\n";
	file << "                     \n";

	for (size_t row = 0; row < ROWS_COUNT; row++)
	{
		file << textMap[row] << "\n";
	}
	
	file.close();
	return true;
}

char LocalMap::getText(CellContent content) const
{
	switch (content)
	{
		case ccUnknown:
			return 'U';
		case ccSelf:
			return 'M';
		case ccEmpty:
			return ' ';
		case ccAlly:
			return 'A';
		case ccNeutral:
			return 'N';
		case ccEnemy:
			return 'E';
		case ccPartialEmpty:
			return 'P';
	}

	return '#';
}
#endif

bool LocalMap::calcWay(const Point2D & desiredEndPoint, std::vector<Point2D> & result, bool force) const
{
	result.clear();

	const Point2D & beginPoint = selfPos;

	Point2D endPoint;//��������� ���� � ������ ���������� �����
	if (!getNearestPlace(desiredEndPoint, endPoint))
	{		
		return false;
	}

	Coordinates beginCoord;
	Coordinates endCoord;
	getCellPos(beginPoint, beginCoord);
	getCellPos(endPoint, endCoord);

	pathfinding->setPartialIsEmpty(force);
	if (!pathfinding->findPath(beginCoord, endCoord))
	{
		return false;
	}

	std::vector<LocalMap::Coordinates> path;
	pathfinding->getPath(path);

	Point2D point;
	for (int i = 0; i < path.size(); ++i)
	{
		const Coordinates & coord = path[i];
		cellToPos(coord.row, coord.col, point);
		result.push_back(point);
	}

	return true;	
}

bool LocalMap::getNearestPlace(const Point2D & desiredEndPoint, Point2D & result) const
{
	int row, col;
	if (!getCellPos(desiredEndPoint, row, col))
	{
		//����� �� ������� ��������� ����� ��������� ����� ���� �� ���� �����
		//��������� �� ��������� ���� �����
		getNearestBorder(desiredEndPoint, row, col);		
	}

	if (cell(row, col).content == LocalMap::ccEmpty)
	{
		return cellToPos(row, col, result);;
	}

	int altRow, altCol;
	if (findFirstAround(row, col, 1, LocalMap::ccEmpty, altRow, altCol))
	{
		return cellToPos(altRow, altCol, result);
	}

	//��� ����� ���������� � �������� �������
	//���� � ������ �������
	if (!findFirstAround(row, col, 5, LocalMap::ccEmpty, altRow, altCol))
	{
		return cellToPos(altRow, altCol, result);
	}

	return false;
}

void LocalMap::getNearestBorder(const Point2D &  desiredEndPoint, int &resultRow, int &resultCol) const
{
	double xShift = desiredEndPoint.getX() - zeroPos.getX();
	double yShift = desiredEndPoint.getY() - zeroPos.getY();

	resultRow = yShift / CELL_SIZE;
	resultCol = xShift / CELL_SIZE;

	if (resultRow < 0)
	{
		resultRow = 0;
	}
	else if (resultRow >= ROWS_COUNT)
	{
		resultRow = ROWS_COUNT - 1;
	}

	if (resultCol < 0)
	{
		resultCol = 0;
	}
	else if (resultCol >= COLS_COUNT)
	{
		resultCol = COLS_COUNT - 1;
	}
}

bool LocalMap::findFirstAround(const int midRow, const int midCol, const int shift, const CellContent needContent, int & resultRow, int & resultCol) const
{
	int row, col;

	if (shift == 1)
	{
		for (int i = 0; i < _Direction_Count; ++i)
		{
			row = aroundCoord[i][0];
			col = aroundCoord[i][1];

			if (cell(row, col).content == needContent)
			{
				resultRow = row;
				resultCol = col;
				return true;
			}
		}
		
		return false;
	}

	for (int rsh = -1 * shift; rsh <= shift; ++rsh)
	{
		row = midRow + rsh;
		for (int csh = -1 * shift; csh <= shift; ++csh)
		{
			col = midCol + csh;
			if (!isValid(row, col))
			{
				continue;
			}

			if (cell(row, col).content == needContent)
			{
				resultRow = row;
				resultCol = col;
				return true;
			}
		}
	}

	return false;
}

bool LocalMap::isPossibleCollision(const Point2D & point, double radius, std::vector<const model::CircularUnit *> & result) const
{
	result.clear();

	Object2D object(point, radius);

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
		Direction direction = calcDirection(position, Point2D(*unit));
		collisions[direction] = true;
	}

}

Direction LocalMap::calcDirection(const Point2D & beginPoint, const Point2D & endPoint) const
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

bool LocalMap::isWayBlocked(const Point2D & targetPoint)
{
	if (!nearUnits.size())
	{
		return false;
	}	
	
	Direction direction = calcDirection(selfPos, targetPoint);
	Direction prev = dh.prevDirecton(direction);
	Direction next = dh.nextDirecton(direction);

	if (aroundCells[direction]->content != ccEmpty)
	{
		return true;
	}
	else if (dh.isDiag(direction))
	{
		if (aroundCells[prev]->content != ccEmpty || aroundCells[next]->content != ccEmpty)
		{
			return true;
		}
	}
	
	std::vector<const model::CircularUnit *> collisionUnits;
	if (!isPossibleCollision(selfPos, BLOCKED_ALERT, collisionUnits))
	{
		return false;
	}
	
	bool collisions[_Direction_Count];
	convertCollisions(selfPos, collisionUnits, collisions);

	if (collisions[direction])
	{
		return true;
	}	
	else if (dh.isDiag(direction))
	{
		if (collisions[prev] || collisions[next])
		{
			return true;
		}
	}

	return false;
}

Pathfinding::Pathfinding(const LocalMap * _map) : map(_map), partialIsEmpty(false)
{
	
	width = map->getWidth();
	height = map->getHeight();

	vertices = new Vertex*[height];
	for (int i = 0; i < height; ++i)
	{
		vertices[i] = new Vertex[width];
	}
}

Pathfinding::~Pathfinding()
{
	for (int i = 0; i < height; ++i)
	{
		delete[] vertices[i];
	}

	delete[] vertices;
}

bool Pathfinding::findPath(const LocalMap::Coordinates & _begin, const LocalMap::Coordinates & _end)
{
	path.clear();
	openList.clear();
	
	begin = _begin;
	end = _end;

	beginVertex = &(vertices[begin.row][begin.col]);
	endVertex = &(vertices[end.row][end.col]);

	prepare();

	if (!startFind())
	{
		return false;
	}

	 makePath();
	 return true;
}

void Pathfinding::prepare()
{
	for (int row = 0; row < height; ++row)
	{
		for (int col = 0; col < height; ++col)
		{
			Vertex & vertex = vertices[row][col];
			vertex.clear();
			
			LocalMap::CellContent content = map->cell(row, col).content;
			if (partialIsEmpty && content == LocalMap::ccPartialEmpty)
			{
				vertex.isEmpty = true;
			}
			else
			{
				vertex.isEmpty = ( content == LocalMap::ccEmpty );
			}

			vertex.selfRow = row;
			vertex.selfCol = col;
		}
	}
}

bool Pathfinding::startFind()
{
	int fromRow = begin.row;
	int fromCol = begin.col;

	vertices[fromRow][fromCol].isClosed = true;

	int row;
	int col;

	for (int i = 0; i < _Direction_Count; ++i)
	{
		Direction direction = Direction(i);
		bool isDiag = dh.isDiag(direction);

		dh.getShift(fromRow, fromCol, direction, row, col);
		if (!isValid(row, col))
		{
			continue;
		}

		Vertex & vertex = vertices[row][col];
		if ((!vertex.isEmpty) || vertex.isClosed)
		{
			continue;
		}

		if (isDiag)
		{
			//���� ���������� ������ ������ �� ���������
			if (isBarrierCorner(fromRow, fromCol, direction))
			{
				continue;
			}
		}

		vertex.G = isDiag ? 14 : 10;
		vertex.H = calcH(row, col);
		vertex.F = vertex.G + vertex.H;
		vertex.target = beginVertex;

		addToOpens(vertex);
		if ((&vertex) == endVertex)
		{
			return true;//����� �� ����� ����
		}
	}

	if (!openList.size())
	{
		return false;//��� ����
	}

	const Vertex * bestVertex = openList.front();
	return find(bestVertex->selfRow, bestVertex->selfCol);
}

void Pathfinding::addToOpens(const Vertex & vertex)
{
	//TODO ��������������� ������
	openList.push_back(&vertex);
	sortOpens();
}

void Pathfinding::removeFromOpens(Vertex & vertex)
{
	vertex.isClosed = true;
	openList.remove(&vertex);
}

bool compareVertex(const Pathfinding::Vertex *p1, const Pathfinding::Vertex *p2)
{
	return p1->F < p2->F;
}

void Pathfinding::sortOpens()
{
	openList.sort(compareVertex);
}

bool Pathfinding::find(int fromRow, int fromCol)
{
	Vertex & fromVertex = vertices[fromRow][fromCol];
	removeFromOpens(fromVertex);

	int row;
	int col;

	for (int i = 0; i < _Direction_Count; ++i)
	{		
		Direction direction = Direction(i);
		bool isDiag = dh.isDiag(direction);

		dh.getShift(fromRow, fromCol, direction, row, col);
		if (!isValid(row, col))
		{
			continue;
		}

		Vertex & vertex = vertices[row][col];
		if ( (!vertex.isEmpty) || vertex.isClosed )
		{
			continue;
		}

		if (isDiag)
		{
			//���� ���������� ������ ������ �� ���������
			if (isBarrierCorner(fromRow, fromCol, direction))
			{
				continue;
			}
		}
		

		//���� 
		if (vertex.target != nullptr)
		{
			Direction nearDireciton = calcDirection(fromRow, fromCol, row, col);
			int pathG = fromVertex.G + (isDiag ? 14 : 10);
			
			if (pathG < vertex.G)
			{
				vertex.target = &fromVertex;
				vertex.G = pathG;
				vertex.F = vertex.G + vertex.H;
				sortOpens();
			}
		}
		else
		{
			vertex.target = &fromVertex;
			vertex.G = fromVertex.G + (isDiag ? 14 : 10);
			vertex.H = calcH(row, col);
			vertex.F = vertex.G + vertex.H;

			addToOpens(vertex);
			if ((&vertex) == endVertex)
			{
				return true;//����� �� ����� ����
			}
		}

	}

	if (!openList.size())
	{
		return false;//��� ����
	}

	const Vertex * bestVertex = openList.front();
	return find(bestVertex->selfRow, bestVertex->selfCol);
}

int Pathfinding::calcH(int row, int col) const
{
	int rCount = std::abs(end.row - row);
	int cCount = std::abs(end.col - col);
	return (rCount + cCount)*10;
}

Direction Pathfinding::calcDirection(int fromRow, int fromCol, int toRow, int toCol)
{
	if (fromCol == toCol)
	{
		if (toRow < fromRow)
		{
			return drTop;
		}
		else
		{
			return drBottom;
		}
	}
	else if (fromRow == toRow)
	{
		if (toCol > fromRow)
		{
			return drRight;
		}
		else
		{
			return drLeft;
		}
	}
	else if (toCol > fromCol && toRow < fromRow)
	{
		return drDiagTR;
	}
	else if (toCol > fromCol && toRow > fromRow)
	{
		return drDiagBR;
	}
	else if (toCol < fromCol && toRow < fromRow)
	{
		return drDiagTL;
	}
	else
	{
		return drDiagBL;
	}

}

bool Pathfinding::isBarrierCorner(int fromRow, int fromCol, Direction direction) const
{
	Direction prev = dh.prevDirecton(direction);
	Direction next = dh.nextDirecton(direction);
	
	int row, col;
	
	dh.getShift(fromRow, fromCol, prev, row, col);
	if (isValid(row, col))
	{
		if (!vertices[row][col].isEmpty)
		{
			return true;
		}
	}

	dh.getShift(fromRow, fromCol, next, row, col);
	if (isValid(row, col))
	{
		if (!vertices[row][col].isEmpty)
		{
			return true;
		}
	}
	
	return false;
}

void Pathfinding::makePath()
{
	const Vertex * vertex = endVertex->target;

	path.push_back(end);
	while (vertex != beginVertex)
	{
		path.push_back(LocalMap::Coordinates(vertex->selfRow, vertex->selfCol));
		vertex = vertex->target;
	}
	path.push_back(begin);

	std::reverse(path.begin(), path.end());//TODO
}

void Pathfinding::getPath(std::vector<LocalMap::Coordinates> & result) const
{	
	result = path;	
}

