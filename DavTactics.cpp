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
	return true;
}


Tactics::TacticsStatus MoveTactics::work()
{
	move();
	return status;
}

bool MoveTactics::move()
{
	env->move->setSpeed(0);
	env->move->setStrafeSpeed(0);
	
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
		}	
	}
	
	//Можем врезаться
	if (!cg->isDirectMovePossible(localWaypoints[curLocalIndex]))
	{
		if (!cg->calcWay(globalWaypoints[curGlobalIndex], localWaypoints))
		{
			status = tsFailure;
			return false;
		}
		
		curLocalIndex = 1;
	}

	const Point2D & targetPoint = localWaypoints[curLocalIndex];

	double angle = env->self->getAngleTo(targetPoint.getX(), targetPoint.getY());
	if (abs(angle) > 0.1) {
		env->move->setTurn(angle);
		return true;
	}

	env->move->setStrafeSpeed(0);
	env->move->setSpeed(env->getMaxForwardSpeed());	
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

	if (cg->getNearEnemyDistance() > env->self->getCastRange())
	{
		status = tsCompleted;
		return status;
	}

	const model::CircularUnit * target = cg->getNearestEnemy();
	if (target == nullptr)
	{
		status = tsCompleted;
		return status;
	}

	double distance = cg->getSelf().getCenter().getDistanceTo(*target);//TODO
	if (distance > env->self->getCastRange())//TODO 
	{
		status = tsCompleted;
		return status;
	}

	double angle = env->self->getAngleTo(*target);
	if (abs(angle) > env->game->getStaffSector() / 2.0) 
	{
		env->move->setTurn(angle);
		return status;
	}

	if (distance < 100.0 || cg->getNearAllyDistance() > 200.0)
	{
		//Отступление
		env->move->setSpeed(-1.0*env->getMaxBackwardSpeed());
		env->move->setStrafeSpeed(env->getMaxStrafeSpeed()*(env->randomBool() ? - 1.0 : 1.0));
	}

	if (distance < (target->getRadius() + 35.0 + 70.0)  && env->getActionRechargeTime(model::ACTION_STAFF) == 0)
	{
		env->move->setAction(model::ACTION_STAFF);
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

	if(env->isSkillLeanded(model::SKILL_FROST_BOLT) && env->self->getMana() > 48)
	{
		if (env->getActionRechargeTime(model::ACTION_FROST_BOLT) == 0)
		{
			env->move->setAction(model::ACTION_FROST_BOLT);
			env->move->setCastAngle(angle);
			env->move->setMinCastDistance(distance - target->getRadius() + env->game->getMagicMissileRadius());
			return status;
		}
	}
	
	if (env->getActionRechargeTime(model::ACTION_MAGIC_MISSILE) == 0)
	{
		env->move->setAction(model::ACTION_MAGIC_MISSILE);
		env->move->setCastAngle(angle);
		env->move->setMinCastDistance(distance - target->getRadius() + env->game->getMagicMissileRadius());
		return status;
	}
	
	return status;
}

BattleTactics::~BattleTactics()
{

}
