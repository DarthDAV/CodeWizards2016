#pragma once

#ifndef _DAV_MAP_H_
#define _DAV_MAP_H_

#include <vector>
#include "DavGameEnvironment.h"
#include "DavGeometry.h"

namespace dav
{
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


		const double CELL_SIZE = 70.0;
		const int ROWS_COUNT = 21;
		const int COLS_COUNT = 21;

		const int SELF_ROW = 10;
		const int SELF_COL = 10;

		const double NEAR_DISTANCE = 210.0;
		const double COLLISION_RISK = 7.5;
		const double BLOCKED_ALERT = 50.0;

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
		
		void setPartialAround(int row, int col);

		void updateCollisions();

		bool getNearestPlace(const Point2D & desiredEndPoint, Point2D & result) const;
		
		Direction calcDirection(const Point2D & beginPoint, const Point2D & endPoint) const;
		Direction prevDirecton(Direction direction) const;
		Direction nextDirecton(Direction direction) const;

		bool isCanMoveDirect(Direction direction) const;

		bool getNearCell(int midRow, int midCol, Direction direction, int &resultRow, int &resultCol) const;

		bool isPossibleCollision(const Point2D & point, double radius, std::vector<const model::CircularUnit *> & result) const;

		bool isPossibleCollision(const Point2D & point, std::vector<const model::CircularUnit *> & result) const
		{
			return isPossibleCollision(point, COLLISION_RISK + CELL_SIZE / 2.0, result);
		}
				
		void convertCollisions(const Point2D & position, const std::vector<const model::CircularUnit *> & positionCollisionUnits, bool * collisions) const;

		char getText(CellContent content) const;

		bool isDiag(LocalMap::Direction directin) const
		{
			return 	directin == drDiagTR || directin == drDiagBR || directin == drDiagBL || directin == drDiagTL;
		}

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
			return row >= 0 && row < ROWS_COUNT && col >= 0 && col < COLS_COUNT;			
		}

		bool saveToFile();

		bool isWayBlocked(const Point2D & targetPoint);

	};

}

#endif