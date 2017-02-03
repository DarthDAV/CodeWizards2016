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
	resetLastMove();
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
	
	//Застряли
	if (isBlockade())
	{
		if (!cg->calcWay(globalWaypoints[curGlobalIndex], localWaypoints))
		{
			status = tsFailure;
			return false;
		}
		curLocalIndex = 1;
		resetLastMove();
		return true;	
	}

	const Point2D & targetPoint = localWaypoints[curLocalIndex];
	double angle = env->self->getAngleTo(targetPoint.getX(), targetPoint.getY());
	if (abs(angle) > 0.1) {
		env->move->setTurn(angle);
		resetLastMove();
		return true;
	}
	
	double moveSpeed = getOptimalSpeed(targetPoint);	
	env->move->setSpeed(moveSpeed);
	saveLastMove();

	return true;	
}

bool MoveTactics::isGlobalWaypointReached()
{
	const Point2D & curGlobalPoint = globalWaypoints[curGlobalIndex];
	return cg->getSelf().getCenter().getDistanceTo(curGlobalPoint) < WAYPOINT_RADIUS;
}

bool MoveTactics::isGlobalWayEnd()
{
	return curGlobalIndex == globalWaypoints.size() - 1;
}

bool MoveTactics::isLocalWaypointReached()
{
	const Point2D & curLocalPoint = localWaypoints[curLocalIndex];
	return cg->getSelf().getCenter().getDistanceTo(curLocalPoint) <= LOCAL_WAYPOINT_RADIUS;
}

bool MoveTactics::isLocalWayEnd()
{
	return curLocalIndex == localWaypoints.size() - 1;
}

double MoveTactics::getOptimalSpeed(const Point2D & targetPoint)
{	
	return cg->getOptimalMoveSpeed(targetPoint);
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
	status = tsInProgress;
	
	env->move->setSpeed(0);

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

	if ((distance < 150.0 && cg->getNearAllyDistance() > 200.0) || env->self->getLife() < 60.0)
	{
		//TODO Отступление
		env->move->setSpeed(-1.0*env->getMaxBackwardSpeed());
	}

	if (env->isSkillLeanded(model::SKILL_SHIELD) && env->self->getMana() > 60)
	{
		if (env->getActionRechargeTime(model::ACTION_SHIELD) == 0)
		{
			env->move->setAction(model::ACTION_SHIELD);
			return status;
		}
	}

	double angle = env->self->getAngleTo(*target);
	if (abs(angle) > env->game->getStaffSector() / 1.6)
	{
		env->move->setTurn(angle);
		return status;
	}

	if (distance < (target->getRadius() + 35.0 + 70.0)  && env->getActionRechargeTime(model::ACTION_STAFF) == 0)
	{
		env->move->setAction(model::ACTION_STAFF);
		return status;
	}

	model::ActionType action = model::_ACTION_UNKNOWN_;
	if(env->isSkillLeanded(model::SKILL_FROST_BOLT) && env->self->getMana() > 48 && env->getActionRechargeTime(model::ACTION_FROST_BOLT) == 0)
	{
		action = model::ACTION_FROST_BOLT;
	}
	else if (env->getActionRechargeTime(model::ACTION_MAGIC_MISSILE) == 0)
	{
		action = model::ACTION_MAGIC_MISSILE;
	}

	if (action == model::_ACTION_UNKNOWN_)
	{
		return status;
	}
		
	env->move->setCastAngle(angle);
	env->move->setMinCastDistance(distance - target->getRadius() + env->game->getMagicMissileRadius());
	env->move->setAction(action);

	env->move->setTurn(angle);

	return status;
}

BattleTactics::~BattleTactics()
{

}
