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
	if (curGlobalIndex == (globalWaypoints.size() - 1))
	{
		curGlobalIndex = getNextGlobalWaypointIndex();//TODO Разобраться. Локальный путь закончился раньше глобального. Сбились с пути
	}
	else if (isGlobalWaypointReached())
	{
		++curGlobalIndex;//Следующая точка доступна только полсе достижения текущей
	}
	
	calcLocalWay();
	resetLastMove();
	return true;
}

bool MoveTactics::calcLocalWay()
{
	Point2D & targetRef = globalWaypoints[curGlobalIndex];
	if (!cg->calcWay(targetRef, localWaypoints))
	{
		if (env->randomBool())
		{
			if (!cg->calcWayForce(targetRef, localWaypoints))
			{
				randomLocalWay(targetRef);
			}
		}
		else
		{
			randomLocalWay(targetRef);
		}
	}

	if (targetRef != localWaypoints.back())
	{
		targetRef = localWaypoints.back();
	}
	curLocalIndex = 1;

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

void MoveTactics::randomLocalWay(const Point2D & endPoint)
{
	localWaypoints.clear();
	localWaypoints.push_back(cg->getSelf().getCenter());
	localWaypoints.push_back(endPoint.getShift(env->randomDouble(-10.0, 10.0), env->randomDouble(-10.0, 10.0)));
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
		targetNextGlobalWaypoint();
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

	if (isBestMoveBackward())
	{
		moveBackward();
	}
	else
	{
		moveForward();
	}

	//TODO
	if (env->move->getSpeed() != 0)
	{
		attack();
	}
}

bool RetreatTactics::isBestMoveBackward()
{
	const model::CircularUnit * enemy = cg->getNearestEnemy();
	if (enemy == nullptr)
	{
		//Врагов нет, быстрее будет идти передом
		return false;
	}

	//Враг в зоне досигаемости выстрела

	if (cg->getNearEnemyDistance() > 300.0)
	{
		//Отошли достаточно далеко от врага, проще отступать передом 
		return false;
	}


	Point2D targetPoint(localWaypoints[curLocalIndex]);
	double targetAngle = getFastAngle(env->self->getAngleTo(targetPoint.getX(), targetPoint.getY()));

	//Враг достаточно близко

	//Если разворачиваться к точке отсупления передом слишком долго, то отступаем задом
	if (std::abs(targetAngle) > PI / 2.0)
	{
		return true;
	}
	
	return false;	
}

void RetreatTactics::attack()
{
	const model::CircularUnit * enemy = getTargetByMinAngle();
	if (enemy == nullptr)
	{
		return;
	}
		
	double enemyAngle = getFastAngle(env->self->getAngleTo(*enemy));	
	if (std::abs(enemyAngle) > env->game->getStaffSector() / 2.0)
	{
		return;
	}	
	
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

	if (action == model::_ACTION_UNKNOWN_)
	{
		return;
	}

	double enemyDistance = cg->getSelf().getCenter().getDistanceTo(*enemy);

	env->move->setCastAngle(enemyAngle);
	env->move->setMinCastDistance(enemyDistance - enemy->getRadius() + env->game->getMagicMissileRadius());
	env->move->setAction(action);
}

const model::CircularUnit * RetreatTactics::getTargetByMinAngle()
{
	double bestAngleAbs = 2.0*PI;
	const model::CircularUnit * bestTarget = nullptr;
	
	auto enemies = cg->getNearEnemies();
	for (auto it = enemies.begin(); it != enemies.end(); ++it)
	{
		const model::CircularUnit * target = *it;
		double targetDistance = cg->getSelf().getCenter().getDistanceTo(*target);
		if (targetDistance > env->self->getCastRange())
		{
			continue;
		}

		double targetAngleAbs = std::abs(getFastAngle(env->self->getAngleTo(*target)));
		if (targetAngleAbs < bestAngleAbs)
		{
			bestAngleAbs = targetAngleAbs;
			bestTarget = target;
		}
	}

	return bestTarget;
}

void RetreatTactics::moveBackward()
{
	//Застряли
	if (isBlockade())
	{
		targetNextGlobalWaypoint();
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

	const model::LivingUnit * target = getTarget();
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


	double angle = getFastAngle(env->self->getAngleTo(*target));
	if (std::abs(angle) > env->game->getStaffSector() / 2.0)
	{
		env->move->setTurn(angle);
		return status;
	}

	if (env->isSkillLeanded(model::SKILL_SHIELD) && env->getActionRechargeTime(model::ACTION_SHIELD) == 0 && env->self->getMana() > 60)
	{
		env->move->setAction(model::ACTION_SHIELD);
		return status;
	}

	model::ActionType action = model::_ACTION_UNKNOWN_;
	if(env->isSkillLeanded(model::SKILL_FROST_BOLT) && env->getActionRechargeTime(model::ACTION_FROST_BOLT) == 0 && env->self->getMana() > 48)
	{
		action = model::ACTION_FROST_BOLT;
	}
	else if (env->getActionRechargeTime(model::ACTION_MAGIC_MISSILE) == 0)
	{
		action = model::ACTION_MAGIC_MISSILE;
	}

	if (action == model::_ACTION_UNKNOWN_)
	{
		if (std::abs(angle) > ANGLE_ACCURACY)
		{
			env->move->setTurn(angle);
		}

		if (target->getLife() < 40)
		{
			env->move->setSpeed(env->getMaxForwardSpeed());
		}
		
		auto nearestEnemy = cg->getNearestEnemy();
		if (cg->getNearEnemyDistance() < (nearestEnemy->getRadius() + 35.0 + 70.0) && env->getActionRechargeTime(model::ACTION_STAFF) == 0)
		{
			env->move->setAction(model::ACTION_STAFF);
		}
		return status;
	}
		
	env->move->setCastAngle(angle);
	env->move->setMinCastDistance(distance - target->getRadius() + env->game->getMagicMissileRadius());
	env->move->setAction(action);

	return status;
}

const model::LivingUnit * BattleTactics::getTarget()
{
	const model::LivingUnit * bestByMinDistance = cg->getNearestEnemy();
	if (cg->getNearEnemyDistance() < 150.0)
	{
		return bestByMinDistance;
	}	
	
	double minLife = 9001.0;
	int maxLevel = -1;
	double minAngle = 2.0 * PI;
	
	const model::LivingUnit * bestTarget = nullptr;
	const model::LivingUnit * bestByMinAngle = nullptr;
	const model::LivingUnit * bestByMinLife = nullptr;
	const model::LivingUnit * bestByMaxLevel = nullptr;
	const model::LivingUnit * bestByBuilding = nullptr;
	
	auto enemies = cg->getNearEnemies();
	for (auto&  enemy : enemies)
	{
		double targetDistance = cg->getSelf().getCenter().getDistanceTo(*enemy);
		if (targetDistance > env->self->getCastRange())
		{
			continue;
		}

		double life = enemy->getLife();
		if ((life == minLife) && (bestByMinLife != nullptr) && (enemy->getId() < bestByMinLife->getId()))
		{
			bestByMinLife = enemy;
		}
		else if (life < minLife)
		{
			minLife = life;
			bestByMinLife = enemy;
		}

		double angleAbs = std::abs(getFastAngle(env->self->getAngleTo(*enemy)));
		if (angleAbs < minAngle)
		{
			minAngle = angleAbs;
			bestByMinAngle = enemy;
		}

		const model::Building * building = dynamic_cast<const model::Building *>(enemy);
		if (building != nullptr)
		{
			bestByBuilding = enemy;
		}

		const model::Wizard * enemyWizard = dynamic_cast<const model::Wizard *>(enemy);
		if (enemyWizard == nullptr)
		{
			continue;
		}
		
		int level = enemyWizard->getLevel();
		if ( (level == maxLevel) && (bestByMaxLevel != nullptr) && (enemyWizard->getLife() < bestByMaxLevel->getLife()) )
		{
			bestByMaxLevel = enemy;
		}		
		else if (level > maxLevel)
		{
			maxLevel = level;
			bestByMaxLevel = enemy;
		}
	}

	if (env->self->getLife() < 50.0 && bestByMinAngle != nullptr)
	{
		return bestByMinAngle;
	}
	else if (bestByMinLife != nullptr && bestByMinLife->getLife() < 25.0)
	{
		return bestByMinLife;
	}
	else if (bestByMaxLevel != nullptr)
	{
		return bestByMaxLevel;
	}
	else if (bestByBuilding != nullptr)
	{
		return bestByBuilding;
	}
	else if (bestByMinLife != nullptr)
	{
		return bestByMinLife;
	}
	else if (bestByMinAngle != nullptr)
	{
		return bestByMinAngle;
	}

	return bestByMinDistance;
}

BattleTactics::~BattleTactics()
{

}
