#include "DavTactics.h"

using namespace dav;

GameEnvironment * Tactics::env = 0;
Cartographer * Tactics::cg = 0;

Tactics::Tactics() : status(tsNotConfigured)
{
}


Tactics::~Tactics()
{
}


MoveTactics::MoveTactics() : Tactics()
{
	curGlobalIndex = -1;
	curLocalIndex = -1;
	isMoving = false;
}

bool MoveTactics::calcWay(Point2D endPoint)
{
	//Строим путь от начальной точки (волщебника) до конечной точки
	if (!cg->calcWayByLane(endPoint, globalWaypoints))
	{
		status = tsFailure;
		return false;
	}

	curGlobalIndex = 0;
	if (!targetNextGlobalWaypoint())
	{
		status = tsFailure;
		return false;
	}	

	status = tsInProgress;
	return true;
}

bool MoveTactics::targetNextGlobalWaypoint()
{
	++curGlobalIndex;
	if (!cg->calcWay(globalWaypoints[curGlobalIndex], localWaypoints))
	{
		status = tsFailure;
		return false;
	}

	curLocalIndex = 1;
	isMoving = false;
	return true;
}


Tactics::TacticsStatus MoveTactics::work()
{
	move();
	return status;
}

bool MoveTactics::move()
{
	if (status != tsInProgress)
	{
		return false;
	}

	if (isLocalWaypointReached())
	{
		if (isLocalWayEnd())
		{
			if (isGlobalWayEnd())
			{
				status = tsCompleted;
				return true;
			}
			else if (!targetNextGlobalWaypoint())
			{
				status = tsFailure;
				return false;
			}			
		}
		else
		{
			++curLocalIndex;
			isMoving = false;
		}	
	}

	//Застряли
	if (isMoving && prevPos == cg->getSelf().getCenter())
	{
		if (!cg->calcWay(globalWaypoints[curGlobalIndex], localWaypoints))
		{
			status = tsFailure;
			return false;
		}
		
		isMoving = false;
		curLocalIndex = 1;
	}

	//Если не застряли, продолжаем движение
	env->move->setStrafeSpeed(0);

	const Point2D & targetPoint = localWaypoints[curLocalIndex];

	double angle = env->self->getAngleTo(targetPoint.getX(), targetPoint.getY());
	if (abs(angle) > 0) {
		env->move->setTurn(angle);
		isMoving = false;
		return true;
	}

	env->move->setStrafeSpeed(0);
	env->move->setSpeed(env->getMaxForwardSpeed());
	
	isMoving = true;
	prevPos = cg->getSelf().getCenter();

	return true;	
}

bool MoveTactics::isGlobalWaypointReached()
{
	const Point2D & curGlobalPoint = globalWaypoints[curGlobalIndex];
	return cg->getSelf().getCenter().getDistanceTo(curGlobalPoint) < WAYPOINT_RADIUS;//TODO
}

bool MoveTactics::isGlobalWayEnd()
{
	return curGlobalIndex == globalWaypoints.size() - 1;
}

bool MoveTactics::isLocalWaypointReached()
{
	const Point2D & curLocalPoint = localWaypoints[curLocalIndex];
	return cg->getSelf().getCenter().getDistanceTo(curLocalPoint) <= 10.0;
}

bool MoveTactics::isLocalWayEnd()
{
	return curLocalIndex == localWaypoints.size() - 1;
}

MoveTactics::~MoveTactics()
{
}

BattleTactics::BattleTactics() : Tactics()
{
	status = tsInProgress;
}

Tactics::TacticsStatus BattleTactics::work()
{	
	env->move->setSpeed(0);
	env->move->setStrafeSpeed(0);

	updateTargets();
	
	if (!targets.size())
	{
		status = tsCompleted;
		return status;
	}

	const Object2D * target = getNearestTarget();
	if (target == nullptr)
	{
		status = tsCompleted;
		return status;
	}

	double distance = cg->getSelf().getCenter().getDistanceTo(target->getCenter());
	if (distance > env->self->getCastRange()) 
	{
		status = tsCompleted;
		return status;
	}

	if (distance < 140)
	{
		env->move->setSpeed(-1.0*env->getMaxBackwardSpeed());
		//env->move->setStrafeSpeed(env->getMaxStrafeSpeed()*(env->randomBool() ? - 1.0 : 1.0));
	}
			
	double angle = env->self->getAngleTo(target->getCenter().getX(), target->getCenter().getY());
	if (abs(angle) > env->game->getStaffSector() / 2.0) 
	{
		env->move->setTurn(angle);
		return status;
	}

	if (env->isSkillLeanded(model::SKILL_SHIELD) && env->self->getMana() > 60)
	{
		if (env->getActionRechargeTime(model::ACTION_SHIELD)== 0)
		{ 
			env->move->setAction(model::ACTION_SHIELD);
			return status;
		}
	} 
	else if (env->getActionRechargeTime(model::ACTION_MAGIC_MISSILE) == 0)
	{
		env->move->setAction(model::ACTION_MAGIC_MISSILE);
		env->move->setCastAngle(angle);
		env->move->setMinCastDistance(distance - target->getRadius() + env->game->getMagicMissileRadius());
		return status;
	}
	
	return status;
}

void BattleTactics::updateTargets()
{
	cg->getEnemies(cg->getSelf().getCenter(), 500.0, targets);
}

Object2D * BattleTactics::getNearestTarget()
{
	double nearestTargetDistance = 4000;
	Object2D *nearesTarget = nullptr;
	
	if (!targets.size())
	{
		return nullptr;
	}

	for (int i = 0; i < targets.size(); ++i) 
	{
		Object2D & target = targets[i];
		double distance = cg->getSelf().getCenter().getDistanceTo(target.getCenter());
		if (distance < nearestTargetDistance)
		{
			nearesTarget = &target;
		}
	}

	return nearesTarget;
}

BattleTactics::~BattleTactics()
{

}
