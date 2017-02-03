#pragma once

#ifndef _DAV_TACTICS_H_
#define _DAV_TACTICS_H_

#include <vector>
#include "DavGeometry.h"
#include "DavGameEnvironment.h"
#include "DavCartographer.h"

namespace dav
{

	class Tactics
	{
	public:

		enum TacticsStatus
		{
			tsCompleted = 0,
			tsInProgress,
			tsNotConfigured,
			tsFailure
		};

	protected:

		TacticsStatus status;
	
	public:
		
		static GameEnvironment * env;
		static Cartographer * cg;
		
		Tactics();
		
		virtual TacticsStatus work() = 0;

		TacticsStatus getStatus()
		{
			return status;
		}

		virtual ~Tactics();
	};

	class MoveTactics : public Tactics
	{
	protected:
				
		const double WAYPOINT_RADIUS = 60.0;
		const double LOCAL_WAYPOINT_RADIUS = 12.5;

		std::vector<Point2D> globalWaypoints;
		std::vector<Point2D> localWaypoints;
				
		int curGlobalIndex;
		int curLocalIndex;		

		bool move();
				
		//Point2D getNextWaypoint();
		//Point2D getPreviousWaypoint();

		bool isGlobalWaypointReached();
		bool isGlobalWayEnd();
		bool isLocalWaypointReached();
		bool isLocalWayEnd();

		bool targetNextGlobalWaypoint();

		bool isLastMove;
		Point2D lastMovePos;

		void resetLastMove()
		{
			isLastMove = false;
			lastMovePos.setPosition(-1, -1);
		}

		void saveLastMove()
		{
			isLastMove = true;
			lastMovePos = cg->getSelf().getCenter();
		}

		bool isBlockade()
		{
			if (!isLastMove)
			{
				return false;
			}

			return lastMovePos == cg->getSelf().getCenter();
		}

		double getOptimalSpeed(const Point2D & targetPoint);

	public:
		MoveTactics();
				
		bool calcWay(Point2D endPoint);

		TacticsStatus work();

		virtual ~MoveTactics();
	};

	class BattleTactics : public Tactics
	{
	protected:


	public:

		BattleTactics();

		TacticsStatus work();

		virtual ~BattleTactics();
	};

}

#endif