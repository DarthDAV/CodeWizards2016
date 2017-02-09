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

		double getFastAngle(double angle)
		{
			double absAngle = std::abs(angle);
			if (std::abs(angle) > PI)
			{
				return (2.0* PI - absAngle) * -1.0 * (angle / absAngle);//TODO :'(
			}

			return angle;
		}
	
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
				
		const double WAYPOINT_RADIUS = 45.0;
		const double LOCAL_WAYPOINT_RADIUS = 5.0;
		const double ANGLE_ACCURACY = 0.02;

		std::vector<Point2D> globalWaypoints;
		std::vector<Point2D> localWaypoints;
				
		int curGlobalIndex;
		int curLocalIndex;		

		virtual void move();

		bool calcContinue();
		void moveForward();
				
		bool isGlobalWaypointReached();
		bool isGlobalWayEnd();
		bool isLocalWaypointReached();
		bool isLocalWayEnd();

		bool targetNextGlobalWaypoint();
		int getNextGlobalWaypointIndex();
		void randomLocalWay();
				
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
				
		virtual bool calcWay(Point2D endPoint);

		virtual TacticsStatus work();

		virtual ~MoveTactics();
	};

	class RetreatTactics : public MoveTactics
	{
	protected:

		virtual void move();

		bool isNearEnemyBackward();

		void attack();

		void moveBackward();
	
	public:
		RetreatTactics() : MoveTactics()
		{

		}

		TacticsStatus work();

		virtual ~RetreatTactics()
		{

		}

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