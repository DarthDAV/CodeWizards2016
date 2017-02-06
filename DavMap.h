#pragma once

#ifndef _DAV_MAP_H_
#define _DAV_MAP_H_

#include <vector>
#include <list>
#include "DavGameEnvironment.h"
#include "DavGeometry.h"

namespace dav
{

	class Pathfinding;

	class LocalMap
	{
	public:		
		enum CellContent
		{
			ccUnknown = 0,
			ccSelf,
			ccEmpty,
			ccAlly,
			ccNeutral,
			ccEnemy,
			ccPartialEmpty
		};

		struct Cell
		{			
			const model::CircularUnit * unit;
			CellContent content;
		};

		enum Direction
		{
			drTop = 0,
			drDiagTR,
			drRight,
			drDiagBR,
			drBottom,
			drDiagBL,
			drLeft,
			drDiagTL,
			_Direction_Count
		};

		struct Coordinates
		{
			int row;
			int col;

			Coordinates() : row(0), col(0)
			{

			}
			
			Coordinates(int _row, int _col) : row(_row), col(_col)
			{

			}
		};

		const double CELL_SIZE = 70.0;
		const int ROWS_COUNT = 21;
		const int COLS_COUNT = 21;

		const int SELF_ROW = 10;
		const int SELF_COL = 10;

		const double NEAR_DISTANCE = 210.0;
		const double COLLISION_RISK = 7.5;
		const double BLOCKED_ALERT = 50.0;

	private:		
		
		Pathfinding  * pathfinding;
		
		Point2D selfPos;
		Point2D zeroPos;
		model::Faction selfFaction;

		Cell cells[21][21];

		int aroundCoord[_Direction_Count][2];
		int aroundShift[_Direction_Count][2];
		const Cell * aroundCells[_Direction_Count];
		
		double nearestEnemyDistance, nearestAllyDistance;
		const model::CircularUnit *nearestEnemy, *nearestAlly;
		std::vector<const model::CircularUnit *> enemies;
		std::vector<const model::CircularUnit *> allies;
		std::vector<const model::CircularUnit *> nearUnits;
		
		void clear();
		
		void setPartialAround(int row, int col);

		void updateCollisions();

		bool getNearestPlace(const Point2D & desiredEndPoint, Point2D & result) const;
		
		Direction calcDirection(const Point2D & beginPoint, const Point2D & endPoint) const;

		bool isCanMoveDirect(Direction direction) const;
				
		bool isPossibleCollision(const Point2D & point, double radius, std::vector<const model::CircularUnit *> & result) const;

		bool isPossibleCollision(const Point2D & point, std::vector<const model::CircularUnit *> & result) const
		{
			return isPossibleCollision(point, COLLISION_RISK + CELL_SIZE / 2.0, result);
		}
				
		void convertCollisions(const Point2D & position, const std::vector<const model::CircularUnit *> & positionCollisionUnits, bool * collisions) const;

#ifdef DEBUG_MAP
		char getText(CellContent content) const;
#endif

	public:

		static bool isDiag(LocalMap::Direction directin)
		{
			return 	directin == drDiagTR || directin == drDiagBR || directin == drDiagBL || directin == drDiagTL;
		}
		
		LocalMap();

		~LocalMap();

		int getWidth() const
		{
			return COLS_COUNT;
		}

		int getHeight() const
		{
			return ROWS_COUNT;
		}
		
		void init(const model::Wizard & self);
		
		void add(const model::CircularUnit & unit);

		void fixing();

		double getEnemyDistance() const
		{
			return nearestEnemyDistance;
		}

		double getAllyDistance() const
		{
			return nearestAllyDistance;
		}

		const Cell & cell(int row, int col) const
		{
			//TODOD
			return cells[row][col];
		}

		const Cell & cell(Direction direction) const
		{
			int row = aroundCoord[direction][0];
			int col = aroundCoord[direction][1];
			return cell(row, col);
		}

		bool getCellPos(const Point2D & pos, int & row, int & col) const;
		
		bool getCellPos(const Point2D & pos, Coordinates & result) const
		{
			return getCellPos(pos, result.row, result.col);
		}

		bool cellToPos(int row, int col, Point2D & result) const;

		bool cellToPos(Direction direction, Point2D & result) const
		{
			int row = aroundCoord[direction][0];
			int col = aroundCoord[direction][1];
			return cellToPos(row, col, result);
		}

		bool normalize(const Point2D & pos, Point2D & normPos) const;

		bool findFirstAround(CellContent needContent, int & row, int & col) const;

		bool calcWay(const Point2D & desiredEndPoint, std::vector<Point2D> & result) const;

		const std::vector<const model::CircularUnit *> & getEnemies() const
		{
			return enemies;
		}

		const std::vector<const model::CircularUnit *> & getAllies() const
		{
			return allies;
		}

		const model::CircularUnit * getNearestEnemy() const
		{
			return nearestEnemy;
		}

		const model::CircularUnit *getNearestAlly() const
		{
			return nearestAlly;
		}
		
		bool isValid(int row, int col) const
		{
			return row >= 0 && row < ROWS_COUNT && col >= 0 && col < COLS_COUNT;			
		}

#ifdef DEBUG_MAP
		bool saveToFile() const;
#endif

		bool isWayBlocked(const Point2D & targetPoint);

		bool getNearCell(int midRow, int midCol, Direction direction, int &resultRow, int &resultCol) const;

		static Direction prevDirecton(Direction direction);
		static Direction nextDirecton(Direction direction);

	};

	class Pathfinding
	{
	public:

		struct Vertex
		{
			int G;//—тоимость передвижени€ из стартовой вершины к данной вершине, следу€ найденному пути к этой вершине
			int H;//ѕримерна€ стоимость передвижени€ от данной вершины до целевой вершины
			int F;//ќценка стоимости пути (сумма G и H)
			bool isEmpty;//¬ершина проходима
			bool isClosed;//¬ершина находитс€ в закрытом списке
			const Vertex * target;//Ќа кого указывает
			int selfRow;
			int selfCol;

			Vertex()
			{
				clear();
			}

			void clear()
			{
				G = 0;
				H = 0;
				F = 0;
				isEmpty = false;
				isClosed = false;
				target = nullptr;
				selfRow = -1;
				selfCol = -1;
			}
		};

	private:	

		const LocalMap * map;
		Vertex ** vertices;
		int width;
		int height;

		LocalMap::Coordinates begin; 
		LocalMap::Coordinates end;

		const Vertex * beginVertex;
		const Vertex * endVertex;

		std::vector<LocalMap::Coordinates> path;
		std::list<const Vertex *> openList;

		const int MAX_F = 1000000;

		void prepare();

		bool startFind();

		bool find(int fromRow, int fromCol);
		
		int calcH(int row, int col) const;

		void addToOpenList(const Vertex & vertex);
		void removeFromOpenList(Vertex & vertex);
		void sortOpenList();

		static LocalMap::Direction calcDirection(int fromRow, int fromCol, int toRow, int toCol);
		
		bool isBarrierCorner(int fromRow, int fromCol, LocalMap::Direction direction) const;

		void makePath();

	public:
		
		Pathfinding(const LocalMap * _map);

		bool findPath(const LocalMap::Coordinates & _begin, const LocalMap::Coordinates & _end);

		void getPath(std::vector<LocalMap::Coordinates> & result) const;

		~Pathfinding();

	};

}

#endif