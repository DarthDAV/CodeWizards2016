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

	private:		
		
		Point2D selfPos;
		Point2D zeroPos;
		model::Faction selfFaction;

		Cell cells[21][21];

		int AroundCoord[_Direction_Count][2];
		bool collisions[_Direction_Count];
		const Cell * aroundCells[_Direction_Count];
		
		LocalMap::Direction calcDirection(const Point2D & endPoint) const;

		void clear();

		void updateCollisions();

		Direction prevDirecton(Direction direction) const;
		Direction nextDirecton(Direction direction) const;

		bool isCanMove(Direction direction) const;

	public:
		
		LocalMap();
		
		void init(const model::Wizard & self);
		
		void add(const model::CircularUnit & unit);

		void fixing();

		const Cell & cell(int row, int col) const
		{
			/*if (row < 0 && row < ROWS_COUNT)
			{
				return ccUnknown;
			}
			else if (col < 0 && col > COLS_COUNT)
			{
				return ccUnknown;
			}*/

			return cells[row][col];
		}

		const Cell & cell(Direction direction) const
		{
			int row = AroundCoord[direction][0];
			int col = AroundCoord[direction][1];
			return cell(row, col);
		}

		bool getCellPos(const Point2D & pos, int & row, int & col) const;

		bool cellToPos(int row, int col, Point2D & result) const;

		bool cellToPos(Direction direction, Point2D & result) const
		{
			int row = AroundCoord[direction][0];
			int col = AroundCoord[direction][1];
			return cellToPos(row, col, result);
		}

		bool normalize(const Point2D & pos, Point2D & normPos) const;

		bool findFirstAround(CellContent needContent, int & row, int & col) const;

		bool calcWay(Point2D & desiredEndPoint, std::vector<Point2D> & result) const;
	};

}

#endif