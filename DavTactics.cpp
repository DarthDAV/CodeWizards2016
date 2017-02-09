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
		randomLocalWay();//TODO Разобраться
		return true;
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
	//curGlobalIndex = getNextGlobalWaypointIndex();
	if (curGlobalIndex < (globalWaypoints.size() - 1))
	{
		++curGlobalIndex;
	}	

	if (!cg->calcWay(globalWaypoints[curGlobalIndex], localWaypoints))
	{		
		randomLocalWay();
	}

	curLocalIndex = 1;
	resetLastMove();
	return true;
}

int MoveTactics::getNextGlobalWaypointIndex()
{
	const Point2D & self = cg->getSelf().getCenter();

	int endIndex = globalWaypoints.size() - 1;
	const Point2D & endPoint = globalWaypoints[endIndex];
	for (int index = 1; index < endIndex; ++index)
	{
		const Point2D &  point = globalWaypoints[index];

		if (point.getDistanceTo(self) <= WAYPOINT_RADIUS) {
			return index + 1;
		}

		if (endPoint.getDistanceTo(point) < endPoint.getDistanceTo(self)) {
			return index;
		}
	}

	return endIndex;
}

void MoveTactics::randomLocalWay()
{
	localWaypoints.clear();
	localWaypoints.push_back(cg->getSelf().getCenter());
	localWaypoints.push_back(cg->getSelf().getCenter().getShift(env->randomDouble(-35.0, 35.0), env->randomDouble(-35.0, 35.0)));
}

Tactics::TacticsStatus MoveTactics::work()
{
	move();
	return status;
}

void MoveTactics::move()
{
	env->move->setSpeed(0);
	env->move->setStrafeSpeed(0);
	
	if (!calcContinue())
	{
		return;
	}
	
	moveForward();
}

bool MoveTactics::calcContinue()
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
				return false;
			}
			else if (!targetNextGlobalWaypoint())
			{
				status = tsFailure;
				return false;
			}
		}
		else
		{
			++curLocalIndex;//TODO
			return true;
		}
	}

	return true;
}

void MoveTactics::moveForward()
{
	//Застряли
	if (isBlockade())
	{
		if (!cg->calcWay(globalWaypoints[curGlobalIndex], localWaypoints))
		{
			randomLocalWay();			
		}
		curLocalIndex = 1;
		resetLastMove();
		return;
	}

	const Point2D & targetPoint = localWaypoints[curLocalIndex];
	double angle = getFastAngle(env->self->getAngleTo(targetPoint.getX(), targetPoint.getY()));
	if (std::abs(angle) > ANGLE_ACCURACY) {
		env->move->setTurn(angle);
		resetLastMove();
		return;
	}

	double moveSpeed = getOptimalSpeed(targetPoint);
	env->move->setSpeed(moveSpeed);
	saveLastMove();
}

bool MoveTactics::isGlobalWaypointReached()
{
	const Point2D & curGlobalPoint = globalWaypoints[curGlobalIndex];
	return cg->getSelf().getCenter().getDistanceTo(curGlobalPoint) < WAYPOINT_RADIUS;
}

bool MoveTactics::isGlobalWayEnd()
{
	return (curGlobalIndex == globalWaypoints.size() - 1) && isGlobalWaypointReached();
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

Tactics::TacticsStatus RetreatTactics::work()
{
	move();
	return status;
}

void RetreatTactics::move()
{
	env->move->setSpeed(0);
	env->move->setStrafeSpeed(0);

	if (!calcContinue())
	{
		return;
	}

	double nearEnemyDistance = cg->getNearEnemyDistance();
	if (nearEnemyDistance < env->self->getCastRange())
	{
		//Если рядом враги

		if (isNearEnemyBackward())
		{
			moveBackward();
		}
		else
		{
			moveForward();
		}

		if (env->move->getSpeed() != 0)
		{
			attack();
		}
	}
	else
	{
		moveForward();
	}	
	
}

bool RetreatTactics::isNearEnemyBackward()
{
	//TODO Исправить
	const model::CircularUnit * enemy = cg->getNearestEnemy();
	if (enemy == nullptr)
	{
		return false;
	}

	Point2D enemyPoint(*enemy);
	const Point2D & targetPoint = globalWaypoints[globalWaypoints.size() - 1];
	const Point2D & selfPoint = cg->getSelf().getCenter();

	if (enemyPoint.getDistanceTo(targetPoint) > selfPoint.getDistanceTo(targetPoint))
	{
		return true;
	}

	return false;	
}

void RetreatTactics::attack()
{
	double nearEnemyDistance = cg->getNearEnemyDistance();
	model::ActionType action = model::_ACTION_UNKNOWN_;

	if (env->isSkillLeanded(model::SKILL_SHIELD) && env->getActionRechargeTime(model::ACTION_SHIELD) == 0 && env->self->getMana() > 60)
	{
		action = model::ACTION_SHIELD;
	}
	else if (nearEnemyDistance < 100 && env->getActionRechargeTime(model::ACTION_STAFF) == 0)
	{
		action = model::ACTION_STAFF;
	}
	else if (env->getActionRechargeTime(model::ACTION_MAGIC_MISSILE) == 0)
	{
		action = model::ACTION_MAGIC_MISSILE;
	}

	if (action != model::_ACTION_UNKNOWN_)
	{
		env->move->setCastAngle(0);
		env->move->setMinCastDistance(nearEnemyDistance);//TODO
		env->move->setAction(action);
	}
}

void RetreatTactics::moveBackward()
{
	//Застряли
	if (isBlockade())
	{
		if (!cg->calcWay(globalWaypoints[curGlobalIndex], localWaypoints))
		{
			randomLocalWay();			
		}
		curLocalIndex = 1;
		resetLastMove();
		return;
	}

	const Point2D & targetPoint = localWaypoints[curLocalIndex];
	double angle = getFastAngle(PI + env->self->getAngleTo(targetPoint.getX(), targetPoint.getY()));
	if (std::abs(angle) > ANGLE_ACCURACY) {
		env->move->setTurn(angle);
		resetLastMove();
		return;
	}

	env->move->setSpeed(-1.0 * env->getMaxBackwardSpeed());
	saveLastMove();
}

BattleTactics::BattleTactics() : Tactics()
{
	status = tsInProgress;
}

Tactics::TacticsStatus BattleTactics::work()
{	
	status = tsInProgress;
	
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

	double distance = cg->getSelf().getCenter().getDistanceTo(*target);
	if (distance > env->self->getCastRange())
	{
		status = tsCompleted;
		return status;
	}

	/*if ((cg->getNearEnemyDistance() < 150.0 && cg->getNearAllyDistance() > 150.0) || env->self->getLife() < 60.0)
	{
		//TODO Отступление
		env->move->setSpeed(-1.0*env->getMaxBackwardSpeed());
	}*/

	if (env->isSkillLeanded(model::SKILL_SHIELD) && env->self->getMana() > 60)
	{
		if (env->getActionRechargeTime(model::ACTION_SHIELD) == 0)
		{
			env->move->setAction(model::ACTION_SHIELD);
			return status;
		}
	}

	double angle = getFastAngle(env->self->getAngleTo(*target));
	if (std::abs(angle) > env->game->getStaffSector() / 2.0)
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

	if (env->move->getSpeed() == 0.0)
	{
		env->move->setTurn(angle);
	}

	return status;
}

BattleTactics::~BattleTactics()
{

}
