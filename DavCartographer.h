#pragma once

#ifndef _DAV_CARTOGRAPHER_H_
#define _DAV_CARTOGRAPHER_H_

#include <vector>
#include "DavGeometry.h"
#include "DavGameEnvironment.h"
#include "DavMap.h"

namespace dav
{

	class Cartographer
	{
	public:
		enum PointMarker
		{
			MET1 = 0, 
			MET2, 
			TET1, 
			TET2, 
			BET1, 
			BET2, 
			EB,
			MAT1, 
			MAT2, 
			TAT1, 
			TAT2, 
			BAT1, 
			BAT2, 
			AB,
			_PointMarker_Size
		};

	protected:

		Point2D Markers[_PointMarker_Size];

		bool respawnDetected;
		GameEnvironment * env;
		Map * map;

		LocalMap locMap;

		Object2D self;
		
		model::Faction enemiesFaction;

		std::vector<Point2D> laneWaypoints[3];
	
		void prepareMarkers();
		void prepareDefaultWays();

		int getBestLane(const Point2D & beginPoint, const Point2D & endPoint) const;
		double calcOverheads(const Point2D & beginPoint, const Point2D & endPoint, int lane) const;

		bool getNearestPlace(const Point2D & desiredEndPoint, Point2D & result) const;
		bool getPlaceAround(LocalMap::CellContent content, Point2D & result) const;
	
	public:
		
		Cartographer(GameEnvironment * env);

		const Point2D &  getPoint(PointMarker pointMarker)
		{
			return Markers[pointMarker];
		}

		void update();

		const Object2D & getSelf() const
		{
			return self;
		}

		bool isRespawn() const
		{
			return respawnDetected;
		}

		bool calcWayByLane(const Point2D & endPoint, std::vector<Point2D> & result) const;
		bool calcWay(const Point2D & desiredEndPoint, std::vector<Point2D> & result) const;

		double getNearEnemyDistance()  const;
		void getEnemies(const Point2D & position, double scanRadius, std::vector<Object2D> & result) const;
		
		~Cartographer();
	};

}

#endif