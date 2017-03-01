#pragma once

#ifndef _DAV_CARTOGRAPHER_H_
#define _DAV_CARTOGRAPHER_H_

#include <vector>
#include <list>
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

		Point2D markers[_PointMarker_Size];
		Point2D nearMarkers[_PointMarker_Size];

		bool respawnDetected;
		GameEnvironment * env;

		LocalMap locMap;
		Object2D self;
		
		const double NEAR_BASE = 1500.0;
		model::Faction enemiesFaction, alliesFaction;
		std::vector<Point2D> laneWaypoints[3];

		std::vector<const model::LivingUnit *> alliesNearBase;
		std::vector<const model::LivingUnit *> enemiesNearBase;
	
		void prepareMarkers();
		void prepareDefaultWays();

		model::LaneType getBestLane(const Point2D & beginPoint, const Point2D & endPoint) const;
		double calcOverheads(const Point2D & beginPoint, const Point2D & endPoint, model::LaneType lane) const;
		void calcLaneFragment(const Point2D & beginPoint, const Point2D & endPoint, model::LaneType lane, std::vector<Point2D> & result) const;

		bool checkNearBase(const model::LivingUnit & unit);
		double calcWayLen(const Point2D & beginPoint, const Point2D & endPoint, int laneBegin, int laneEnd, model::LaneType lane) const;
	
	public:
		
		Cartographer(GameEnvironment * env);

		const Point2D &  getPoint(PointMarker pointMarker)
		{
			return markers[pointMarker];
		}

		const Point2D &  getCollectionPoint(PointMarker pointMarker)
		{
			return nearMarkers[pointMarker];
		}

		const Point2D &  getNearestCollectionPoint(const Point2D & forPoint) const;

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
		bool calcWayForce(const Point2D & desiredEndPoint, std::vector<Point2D> & result) const;

		double getNearEnemyDistance()  const
		{
			return locMap.getEnemyDistance();
		}

		double getNearAllyDistance()  const
		{
			return locMap.getAllyDistance();
		}

		const std::vector<const model::LivingUnit *> & getNearEnemies( ) const
		{
			return locMap.getEnemies();
		}

		const std::vector<const model::LivingUnit *> &  getNearAllies() const
		{
			return locMap.getAllies();
		}
		
		const model::LivingUnit * getNearestEnemy() const
		{
			return locMap.getNearestEnemy();
		}

		const model::LivingUnit *getNearestAlly() const
		{
			return locMap.getNearestAlly();
		}

		const model::Building * getNearAlliedBuilding() const;

		const std::vector<const model::LivingUnit *> & getEnemiesNearBase() const
		{
			return enemiesNearBase;
		}

		const std::vector<const model::LivingUnit *> & getAlliesNearBase()  const
		{
			return alliesNearBase;
		}

		model::LaneType whatLane(const Point2D & point) const;

		void getWizards(model::Faction faction, std::list<const model::Wizard *> & result) const;

		
		model::Faction getEnemiesFaction() const
		{
			return enemiesFaction;
		}
		
		model::Faction getAlliesFaction() const
		{
			return alliesFaction;
		}

		double getOptimalMoveSpeed(const Point2D & targetPoint)
		{			
			/*
			//TODO Двигаться вместе с союзниками
			if (locMap.isWayBlocked(targetPoint))
			{
				return 2.0;
			}*/
						
			return env->getMaxForwardSpeed();
		}

		~Cartographer();
	};

}

#endif