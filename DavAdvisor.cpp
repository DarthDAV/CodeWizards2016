#include "DavAdvisor.h"

#include <cstring>
#include <limits>
#include "DavGeometry.h"

using namespace dav;

Advisor::Advisor(GameEnvironment * env) : curTactics(nullptr), curLane(model::_LANE_UNKNOWN_), orderLane(model::_LANE_UNKNOWN_), orderSkill(model::_SKILL_UNKNOWN_), moveTactics(nullptr), battleTactics(nullptr), lastLevelUp(0), orderCountdown(ORDER_COUNTDOWN)
{
	this->env = env;
	cg = new Cartographer(env);

	dav::Tactics::env = env;
	dav::Tactics::cg = cg;

	baseRetreatPoint = cg->getCollectionPoint(Cartographer::AB);

	stageIt = plan.end();
	prepare();	
}

void Advisor::prepare()
{	
	prepareTactics();
	preparePlan();
	prepareSkills();
}

void Advisor::preparePlan()
{
	model::LaneType lane = model::_LANE_UNKNOWN_;
	if (!env->self->isMaster())
	{
		readMessages();
		if (isValidLane(orderLane))
		{
			lane = orderLane;
		}
	}
		
	if (lane == model::_LANE_UNKNOWN_)
	{
		lane = getDefaultLane();
	}

	useLane(lane);
}

model::LaneType Advisor::getDefaultLane() const
{
	model::LaneType lane = model::_LANE_UNKNOWN_;
	int wizardId = (int) env->self->getId();

	switch (wizardId) {
	case 1:
	case 2:
	case 6:
	case 7:
		lane = model::LANE_TOP;
		break;
	case 3:
	case 8:
		lane = model::LANE_MIDDLE;
		break;
	case 4:
	case 5:
	case 9:
	case 10:
		lane = model::LANE_BOTTOM;
		break;
	default:
		lane = model::LANE_MIDDLE;
		break;
	}

	return lane;
}

void Advisor::useLane(model::LaneType lane)
{
	curTactics = nullptr;
	plan.clear();
	stageIt = plan.end();

	switch (lane)
	{
	case model::LANE_TOP:
	{
		prepareTopLanePlan();
		break;
	}

	case model::LANE_MIDDLE:
	{
		prepareMiddleLanePlan();
		break;
	}

	case model::LANE_BOTTOM:
	{
		prepareBottomLanePlan();
		break;
	}

	}

}

void Advisor::prepareTopLanePlan()
{
	curLane = model::LANE_TOP;
	
	plan.clear();

	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::TAT1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::TAT2), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::TET1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::TET2), Stage::sgReachPoint));
	
	Point2D nearEB(cg->getPoint(Cartographer::EB).getShift(-200.0, -250.0));
	plan.push_back(Stage(nearEB, Stage::sgReachPoint));

	stageIt = plan.begin();
}
	
void Advisor::prepareMiddleLanePlan()
{
	curLane = model::LANE_MIDDLE;
	
	plan.clear();
	
	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::MAT1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::MAT2), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::MET1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::MET2), Stage::sgReachPoint));
	
	Point2D nearEB(cg->getPoint(Cartographer::EB).getShift(-200.0, -200.0));
	plan.push_back(Stage(nearEB, Stage::sgReachPoint));
	
	stageIt = plan.begin();
}

void Advisor::prepareBottomLanePlan()
{
	curLane = model::LANE_BOTTOM;
	
	plan.clear();

	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::BAT1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::BAT2), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::BET1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getCollectionPoint(Cartographer::BET2), Stage::sgReachPoint));

	Point2D nearEB(cg->getPoint(Cartographer::EB).getShift(+300.0, +200.0));
	plan.push_back(Stage(nearEB, Stage::sgReachPoint));

	stageIt = plan.begin();
}

void Advisor::prepareTactics()
{
	moveTactics = new MoveTactics();
	battleTactics = new BattleTactics();
	retreatTactics = new RetreatTactics();
}

void Advisor::prepareSkills()
{
	skillByLevel[0] = model::_SKILL_UNKNOWN_;
	skillByLevel[1] = model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1;
	skillByLevel[2] = model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1;
	skillByLevel[3] = model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2;
	skillByLevel[4] = model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2;
	skillByLevel[5] = model::SKILL_SHIELD;

	skillByLevel[6] = model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1;
	skillByLevel[7] = model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1;
	skillByLevel[8] = model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2;
	skillByLevel[9] = model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2;
	skillByLevel[10] = model::SKILL_FROST_BOLT;

	skillByLevel[11] = model::SKILL_RANGE_BONUS_PASSIVE_1;
	skillByLevel[12] = model::SKILL_RANGE_BONUS_AURA_1;
	skillByLevel[13] = model::SKILL_RANGE_BONUS_PASSIVE_2;
	skillByLevel[14] = model::SKILL_RANGE_BONUS_AURA_2;
	skillByLevel[15] = model::SKILL_ADVANCED_MAGIC_MISSILE;

	skillByLevel[16] = model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1;
	skillByLevel[17] = model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1;
	skillByLevel[18] = model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2;
	skillByLevel[19] = model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2;
	skillByLevel[20] = model::SKILL_HASTE;

	skillByLevel[21] = model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1;
	skillByLevel[22] = model::SKILL_STAFF_DAMAGE_BONUS_AURA_1;
	skillByLevel[23] = model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2;
	skillByLevel[24] = model::SKILL_STAFF_DAMAGE_BONUS_AURA_2;
	skillByLevel[25] = model::SKILL_FIREBALL;
}

Advisor::~Advisor()
{
	delete moveTactics;
	delete battleTactics;
	delete retreatTactics;
}

void Advisor::work()
{
	cg->update();
	
	if (env->self->isMaster())
	{
		giveOrders();
	}
	else
	{
		executeOrders();
	}
	
	skillLearn();
	
	if (cg->isRespawn())
	{
		onRespawn();
	}

	if (curTactics == nullptr)
	{
		useStageTactics();
	}

	if (isBaseInDanger())
	{
		if (!isRetreatToBase() && !isNearBase())
		{
			retreatToBase();
		}
		else if (isRetreatToBase() && isOnBase())
		{
			protectBase();
		}
		else if (isEnemyArea() && !isFight())
		{
			joinBattle();
		}
	}
	else if (isLowHealth() && !isRetreat() )
	{		
		retreatToNearAlliedBuilding();
	}
	else if (isFight() && ((cg->getNearEnemyDistance() < 200.0) || (cg->getNearAllyDistance() > 400.0)) )
	{
		retreatToNearAlliedBuilding();
	}
	else if (isEnemyArea() && !isFight() && !isRetreat())
	{
		joinBattle();
	}
	else if (isNormHealth() && isRetreatNotToBase())
	{		
		skipUrgentStageIfNeed();
	}
		
	curTactics->work();

	if (curTactics->getStatus() == Tactics::tsCompleted)
	{
		nextStage();
		useStageTactics();
	}
	else if (curTactics->getStatus() != Tactics::tsInProgress)
	{
		retreatToNearAlliedBuilding();//Если сбой тактики
	}
}

bool Advisor::isFight()
{
	return stageIt->getGoal() == Stage::sgDestroyEnemy;
}

void Advisor::giveOrders()
{
	--orderCountdown;
	if (orderCountdown > 0)
	{
		return;
	}

	orderCountdown = ORDER_COUNTDOWN;

	LaneAdvisor laneAdvisor(env, cg);
	laneAdvisor.analyzeSituation();
	laneAdvisor.balance();
	
	std::vector<model::Message> messages;
	laneAdvisor.getRecommendedOrders(messages);
	env->move->setMessages(messages);	

	//Приказ для себя
	orderLane = laneAdvisor.getSelfLane();
	changeLaneIfNeed(orderLane);
}

void Advisor::changeLaneIfNeed(model::LaneType toLane)
{
	//TODO Смена дорожки c переходами
	if (curLane == toLane || !isValidLane(toLane))
	{
		return;
	}
	
	useLane(toLane);
	stageIt = plan.insert(plan.begin(), Stage(baseRetreatPoint, Stage::sgReachPoint));
}

void Advisor::executeOrders()
{
	--orderCountdown;
	if (orderCountdown > 0)
	{
		return;
	}

	orderCountdown = ORDER_COUNTDOWN;
	
	model::LaneType prevOrderLane = orderLane;
	model::SkillType prevOrderSkill = orderSkill;//TODO Изучение умений
	readMessages();
	
	changeLaneIfNeed(orderLane);
}

void Advisor::readMessages()
{
	const std::vector<model::Message> & messages = env->self->getMessages();
	for (int i = 0; i < messages.size(); ++i)
	{
		const model::Message & message = messages[i];

		orderLane = message.getLane();
		orderSkill = message.getSkillToLearn();
	}
}

void Advisor::skillLearn()
{
	if (!env->game->isSkillsEnabled())
	{
		return;
	}

	int curLevel = env->self->getLevel();
	if (!curLevel)
	{
		return;
	}

	for (int i = 1 + lastLevelUp; i <= curLevel; ++i)
	{
		model::SkillType skill = getSkill(i);
		if (!env->isSkillLeanded(skill))
		{
			env->move->setSkillToLearn(skill);			
			return;
		}	

		lastLevelUp = i;//Изучено для уровня
	}
}

model::SkillType Advisor::getSkill(int level)
{
	return skillByLevel[level];
}

void Advisor::onRespawn()
{
	LaneAdvisor laneAdvisor(env, cg);
	laneAdvisor.analyzeSituation();

	model::LaneType lane = laneAdvisor.getWorstLane();
	if (!isValidLane(lane))
	{
		lane = getDefaultLane();
	}
	useLane(lane);

	if (env->self->isMaster())
	{
		orderCountdown = 1;
		giveOrders();
	}
}

bool Advisor::isBaseInDanger()
{		
	auto  enemiesNearBase = cg->getEnemiesNearBase();
	
	if (!enemiesNearBase.size())
	{
		return false;
	}

	if (enemiesNearBase.size() > 2)
	{
		return true;
	}

	return false;
}

bool Advisor::isRetreat()
{
	return stageIt->getGoal() == Stage::sgRetreatToPoint;
}

bool Advisor::isRetreatToBase()
{
	return stageIt->getGoal() == Stage::sgRetreatToPoint && stageIt->getPoint() == baseRetreatPoint;
}

bool Advisor::isRetreatNotToBase()
{
	return stageIt->getGoal() == Stage::sgRetreatToPoint && stageIt->getPoint() != baseRetreatPoint;
}

bool Advisor::isNearBase()
{
	return cg->getSelf().getCenter().getDistanceTo(baseRetreatPoint) < NEAR_BASE;
}

bool Advisor::isOnBase()
{
	return cg->getSelf().getCenter().getDistanceTo(baseRetreatPoint) < GROUP_RADIUS;
}

void Advisor::protectBase()
{
	dropUrgentStage();

	auto enemy = cg->getNearestEnemy();
	if (enemy != nullptr)
	{
		model::LaneType lane = cg->whatLane(Point2D(*enemy));
		if (isValidLane(lane))
		{
			curLane = lane;
		}
	}

	useLane(curLane);
	useStageTactics();	
}

void Advisor::retreatToBase()
{
	Stage stage(baseRetreatPoint, Stage::sgRetreatToPoint);
	setUrgentStage(stage);
	useStageTactics();
}

void Advisor::setUrgentStage(Stage & stage)
{
	dropUrgentStage();
	stageIt = plan.insert(stageIt, stage);
}

void Advisor::dropUrgentStage()
{
	if (stageIt->getGoal() == Stage::sgDestroyEnemy || stageIt->getGoal() == Stage::sgRetreatToPoint)
	{
		stageIt = plan.erase(stageIt);
	}
}

void Advisor::skipUrgentStageIfNeed()
{
	if (stageIt->getGoal() == Stage::sgDestroyEnemy || stageIt->getGoal() == Stage::sgRetreatToPoint)
	{
		stageIt = plan.erase(stageIt);
		useStageTactics();
	}
}

bool Advisor::isLowHealth()
{
	return double(env->self->getLife()) / double(env->self->getMaxLife()) < LOW_HEALTH;
}

bool Advisor::isNormHealth()
{
	return double(env->self->getLife()) / double(env->self->getMaxLife()) > NORM_HEALTH;
}

void Advisor::retreatToNearAlliedBuilding()
{
	const model::Building * building = cg->getNearAlliedBuilding();
	if (building == nullptr)
	{
		retreatToBase();
		return;
	}

	const Point2D & colPoint = cg->getNearestCollectionPoint(Point2D(*building));
	Stage stage(colPoint, Stage::sgRetreatToPoint);
	setUrgentStage(stage);
	useStageTactics();	
}

void Advisor::retreatToNearAlly()
{
	auto ally = cg->getNearestAlly();
	if (ally == nullptr)
	{
		retreatToNearAlliedBuilding();
		return;
	}

	Point2D point(*ally);
	Stage stage(point, Stage::sgRetreatToPoint);
	setUrgentStage(stage);
	useStageTactics();
}

bool Advisor::isEnemyArea()
{
	double distance = cg->getNearEnemyDistance();
	return distance <= env->game->getWizardCastRange() - 25.0;
}

void Advisor::joinBattle()
{
	Stage stage(cg->getSelf().getCenter(), Stage::sgDestroyEnemy);
	setUrgentStage(stage);
	useStageTactics();
}

void Advisor::nextStage()
{
	if (stageIt->getGoal() == Stage::sgDestroyEnemy || stageIt->getGoal() == Stage::sgRetreatToPoint)
	{
		stageIt = plan.erase(stageIt);
	}
	else
	{
		++stageIt;
	}

	if (stageIt == plan.end())
	{
		stageIt = plan.begin();//TODO План закончился
	}
}

void Advisor::useStageTactics()
{
	curTactics = nullptr;
	const Stage & curStag = *stageIt;
	
	if (curStag.getGoal() == Stage::sgReachPoint)
	{
		moveTactics->calcWay(curStag.getPoint());
		curTactics = moveTactics;
	}
	else if (curStag.getGoal() == Stage::sgDestroyEnemy)
	{
		curTactics = battleTactics;
	}
	if (curStag.getGoal() == Stage::sgRetreatToPoint)
	{
		retreatTactics->calcWay(curStag.getPoint());
		curTactics = retreatTactics;
	}

}

LaneAdvisor::LaneAdvisor(GameEnvironment * _env, Cartographer * _cg) : env(_env), cg(_cg)
{
	clear();
}

void LaneAdvisor::clear()
{
	beginWizardLane.clear();
	moveWizardLane.clear();
	allies.clear();
	enemies.clear();

	for (int i = 0; i < model::_LANE_COUNT_; ++i)
	{
		wizardsByLane[i][0].clear();
		wizardsByLane[i][1].clear();
	}

	std::memset(beginBalance, 0, sizeof(int)*model::_LANE_COUNT_);
	std::memset(beginReserve, 0, sizeof(int)*model::_LANE_COUNT_);
	std::memset(resultBalance, 0, sizeof(int)*model::_LANE_COUNT_);

	wizardsReserve.clear();
}

void LaneAdvisor::analyzeSituation()
{		
	cg->getWizards(cg->getAlliesFaction(), allies);
	cg->getWizards(cg->getEnemiesFaction(), enemies);
	
	calcWizardsByLane();

	for (int i = 0; i < model::_LANE_COUNT_; ++i)
	{
		beginBalance[i] = wizardsByLane[i][0].size() - wizardsByLane[i][1].size();

		//TODO Учесть башни и миньонов

		if (beginBalance[i] >= 1)
		{
			beginReserve[i] = beginBalance[i];
		}
	}
	
}

void LaneAdvisor::calcWizardsByLane()
{
	std::list<const model::Wizard *>::iterator it;
	for (it = enemies.begin(); it != enemies.end(); ++it)
	{
		const model::Wizard * wizard = *it;
		model::LaneType lane = cg->whatLane(Point2D(*wizard));
		wizardsByLane[lane][1].push_back(wizard);
	}

	for (it = allies.begin(); it != allies.end(); ++it)
	{
		const model::Wizard * wizard = *it;
		model::LaneType lane = cg->whatLane(Point2D(*wizard));
		wizardsByLane[lane][0].push_back(wizard);
		beginWizardLane[wizard] = lane;
	}

	model::LaneType selfLane = cg->whatLane(Point2D(*(env->self)));
	wizardsByLane[selfLane][0].push_back(env->self);
	beginWizardLane[env->self] = selfLane;
}

void LaneAdvisor::balance()
{
	std::memcpy(resultBalance, beginBalance, sizeof(int) * model::_LANE_COUNT_);
	
	calcWizardsReserve();	
	if (!wizardsReserve.size())
	{
		return;
	}	

	while (wizardsReserve.size() > 0)
	{		
		model::LaneType worstLane = getWorstLane();
		if (worstLane == model::_LANE_UNKNOWN_)
		{
			break;
		}

		int worstBalance = resultBalance[worstLane];
				
		if (worstBalance >= 0)
		{
			break;//TODO Равномерно распределять?
		}
		
		int needCount = worstBalance == 0 ? 1 : std::abs(worstBalance);
		int reinforcementCount = needCount > wizardsReserve.size() ? wizardsReserve.size() : needCount;

		while(reinforcementCount > 0)
		{
			const model::Wizard * wizard = wizardsReserve.front();	
			wizardsReserve.pop_front();
			--reinforcementCount;

			model::LaneType reserveLane = cg->whatLane(Point2D(*wizard));
			--(resultBalance[reserveLane]);//TODO Проверка

			moveWizardLane[wizard] = worstLane;
			++(resultBalance[worstLane]);

		}
	
	}
	
}

void LaneAdvisor::calcWizardsReserve()
{
	wizardsReserve.clear();

	for (int lane = 0; lane < model::_LANE_COUNT_; ++lane)
	{
		int reserveByLane = beginReserve[lane];
		for (int j = 0; j < reserveByLane; ++j)
		{
			wizardsReserve.push_back(wizardsByLane[lane][0][j]);
		}
	}

}

model::LaneType LaneAdvisor::getWorstLane()
{
	int worstBalance = std::numeric_limits<int>::max();
	model::LaneType worstLane = model::_LANE_UNKNOWN_;

	for (int i = 0; i < model::_LANE_COUNT_; ++i)
	{
		if (resultBalance[i] < worstBalance)
		{
			worstBalance = resultBalance[i];
			worstLane = (model::LaneType) i;
		}
	}

	return worstLane;
}

void LaneAdvisor::getRecommendedOrders(std::vector<model::Message> & result)
{
	std::vector<signed char> text;
		
	std::list<const model::Wizard *>::iterator it;
	for (it = allies.begin(); it != allies.end(); ++it)
	{
		model::LaneType lane = model::_LANE_UNKNOWN_;
		model::SkillType skill = model::_SKILL_UNKNOWN_;
		
		lane = getLaneToMove(*it);
		
		model::Message message(lane, skill, text);//TODO
		result.push_back(message);
	}
}

model::LaneType LaneAdvisor::getLaneToMove(const model::Wizard * wizard)
{
	std::map<const model::Wizard *, model::LaneType>::iterator it = moveWizardLane.find(wizard);
	if (it == moveWizardLane.end())
	{
		return beginWizardLane[wizard];
	}
	
	return moveWizardLane[wizard];
}

model::LaneType LaneAdvisor::getSelfLane()
{
	return getLaneToMove(env->self);
}