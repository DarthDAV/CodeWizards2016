#pragma once

#ifndef _DAV_MAP_H_
#define _DAV_MAP_H_

#include <vector>
#include "DavGameEnvironment.h"
#include "DavGeometry.h"

namespace dav
{

	class Map
	{
	public:
		struct Cell
		{
		public:
			
			bool isUnexplored;
			
			std::vector<const model::Building *> buildings;
			std::vector<const model::Wizard *> wizards;
			std::vector<const model::Minion *> minions;
			std::vector<const model::Tree *> trees;
			
			Cell()
			{				
				isUnexplored = true;
			}

			void clear()
			{				
				buildings.clear();
				wizards.clear();
				minions.clear();
				trees.clear();
			}

		};

	private:
		const double CELL_SIZE = 400.0;
		int rowsCount;
		int colsCount;
		int worldWidth;
		int worldHeight;

		Cell ** cells;
	public:
		Map(int _worldWidth, int _worldHeight);

		Cell * cell(int row, int col);
		
		Cell * findCell(int worldX, int worldY);
		Cell * findCell(const Point2D & point)
		{
			return findCell(point.getX(), point.getY());
		}
		Cell * findCell(const Object2D & object)
		{
			return findCell(object.getCenter().getX(), object.getCenter().getY());
		}

		void clear();

		void add(const model::Building & building);
		void add(const model::Wizard & wizard);
		void add(const model::Minion & minion);
		void add(const model::Tree & tree);

		bool isValidPosition(const Point2D & point)
		{
			double x = point.getX();
			double y = point.getY();

			return !(x < 0 || x > worldWidth || y < 0 || y > worldHeight);
		}
		
		~Map();
	};

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
			ccEnemy
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


		const double CELL_SIZE = 70.0;
		const int ROWS_COUNT = 21;
		const int COLS_COUNT = 21;

		const int SELF_ROW = 10;
		const int SELF_COL = 10;

		const double NEAR_DISTANCE = 200.0;
		const double COLLISION_RISK = 5.0;

	private:		
		
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

		void updateCollisions();

		bool getNearestPlace(const Point2D & desiredEndPoint, Point2D & result) const;
		
		Direction calcDirection(const Point2D & beginPoint, const Point2D & endPoint) const;
		Direction prevDirecton(Direction direction) const;
		Direction nextDirecton(Direction direction) const;

		bool isCanMoveDirect(Direction direction) const;

		bool getNearCell(int midRow, int midCol, Direction direction, int &resultRow, int &resultCol) const;

		bool isPossibleCollision(const Point2D & point, std::vector<const model::CircularUnit *> & result) const;
		
		void convertCollisions(const Point2D & position, const std::vector<const model::CircularUnit *> & positionCollisionUnits, bool * collisions) const;

	public:
		
		LocalMap();
		
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
			if (row < 0 && row > ROWS_COUNT)
			{
				return false;
			}
			else if (col < 0 && col > COLS_COUNT)
			{
				return false;
			}

			return true;
		}

		const bool isDirectMovePossible(const Point2D & targetPoint) const;

	};

}

#endif