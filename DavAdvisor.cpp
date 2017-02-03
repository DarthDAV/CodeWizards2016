#include "DavAdvisor.h"

#include "DavGeometry.h"

using namespace dav;

Advisor::Advisor(GameEnvironment * env) : curTactics(nullptr), curLane(model::_LANE_UNKNOWN_), orderLane(model::_LANE_UNKNOWN_), orderSkill(model::_SKILL_UNKNOWN_), moveTactics(nullptr), battleTactics(nullptr), lastLevelUp(0), orderCountdown(ORDER_COUNTDOWN)
{
	this->env = env;
	cg = new Cartographer(env);

	dav::Tactics::env = env;
	dav::Tactics::cg = cg;

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
	if (env->self->isMaster())
	{
		lane = model::LANE_MIDDLE;
	}
	else
	{
		readMessages();
		if (isValidLane(orderLane))
		{
			lane = orderLane;
		}
	}
		
	if (lane < 0)
	{
		lane = (model::LaneType) env->randomInt(0, 2);
	}

	useLane(lane);
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

	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::TAT1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::TAT2), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::TET1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::TET2), Stage::sgReachPoint));
	
	Point2D nearEB(cg->getPoint(Cartographer::EB).getShift(-200.0, -250.0));
	plan.push_back(Stage(nearEB, Stage::sgReachPoint));

	stageIt = plan.begin();
}
	
void Advisor::prepareMiddleLanePlan()
{
	curLane = model::LANE_MIDDLE;
	
	plan.clear();
	
	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::MAT1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::MAT2), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::MET1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::MET2), Stage::sgReachPoint));
	
	Point2D nearEB(cg->getPoint(Cartographer::EB).getShift(-200.0, -200.0));
	plan.push_back(Stage(nearEB, Stage::sgReachPoint));
	
	stageIt = plan.begin();
}

void Advisor::prepareBottomLanePlan()
{
	curLane = model::LANE_BOTTOM;
	
	plan.clear();

	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::BAT1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::BAT2), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::BET1), Stage::sgReachPoint));
	plan.push_back(Stage(cg->getÑollectionPoint(Cartographer::BET2), Stage::sgReachPoint));

	Point2D nearEB(cg->getPoint(Cartographer::EB).getShift(+300.0, +200.0));
	plan.push_back(Stage(nearEB, Stage::sgReachPoint));

	stageIt = plan.begin();
}

void Advisor::prepareTactics()
{
	moveTactics = new MoveTactics();
	battleTactics = new BattleTactics();
}

void Advisor::prepareSkills()
{
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

	if (ifMetEnemy())
	{
		joinBattle();
	}
	else if (isBaseInDanger())
	{
		retreatToBase();
	}
	else if (isLowHealth())
	{
		retreatToNearAlliedBuilding();
	}

	curTactics->work();

	if (curTactics->getStatus() == Tactics::tsCompleted)
	{
		nextStage();
		useStageTactics();
	}
	else if (curTactics->getStatus() != Tactics::tsInProgress)
	{
		retreatToBase();//TODO Åñëè ñáîé òàêòèêè
	}
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

	//Ïğèêàç äëÿ ñåáÿ
	orderLane = laneAdvisor.getSelfLane();
	if (curLane != orderLane && isValidLane(orderLane))
	{
		useLane(orderLane);
		retreatToBase();
	}
	
}

void Advisor::executeOrders()
{
	model::LaneType prevOrderLane = orderLane;
	model::SkillType prevOrderSkill = orderSkill;//TODO Èçó÷åíèå óìåíèé
	readMessages();

	if (curLane != orderLane && isValidLane(orderLane))
	{
		//Ñìåíà äîğîæêè c ïåğåõîäàìè
		useLane(orderLane);
		retreatToBase();
	}
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

	if (curLevel <= lastLevelUp)
	{
		return;
	}

	lastLevelUp = curLevel;

	model::SkillType skill = getSkill(curLevel);
	env->move->setSkillToLearn(skill);
	env->setSkillLeanded(skill, true);
}

model::SkillType Advisor::getSkill(int level)
{
	return skillByLevel[level];
}

void Advisor::onRespawn()
{
	LaneAdvisor laneAdvisor(env, cg);
	laneAdvisor.analyzeSituation();
	curLane = laneAdvisor.getWorstLane();
	useLane(curLane);

	if (env->self->isMaster())
	{
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

	auto  alliesNearBase = cg->getAlliesNearBase();
	if (alliesNearBase.size() < enemiesNearBase.size())
	{
		return true;
	}

	return false;
}

void Advisor::retreatToBase()
{
	stageIt = plan.insert(stageIt, Stage(cg->getÑollectionPoint(Cartographer::AB), Stage::sgRetreatToPoint));
	useStageTactics();
}

bool Advisor::isLowHealth()
{
	return double(env->self->getLife()) / double(env->self->getMaxLife()) < LOW_HEALTH;
}

void Advisor::retreatToNearAlliedBuilding()
{
	const model::Building * building = cg->getNearAlliedBuilding();
	if (building == nullptr)
	{
		retreatToBase();
		return;
	}

	const Point2D & colPoint = cg->getNearestÑollectionPoint(Point2D(*building));
	stageIt = plan.insert(stageIt, Stage(colPoint, Stage::sgRetreatToPoint));
	useStageTactics();	
}

bool Advisor::ifMetEnemy()
{
	return isEnemyArea() && stageIt->getGoal() != Stage::sgDestroyEnemy;
}

bool Advisor::isEnemyArea()
{
	double distance = cg->getNearEnemyDistance();
	return distance <= env->game->getWizardCastRange();
}

void Advisor::joinBattle()
{
	stageIt = plan.insert(stageIt, Stage(cg->getSelf().getCenter(), Stage::sgDestroyEnemy));
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
		stageIt = plan.begin();//TODO Ïëàí çàêîí÷èëñÿ
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
		moveTactics->calcWay(curStag.getPoint());
		curTactics = moveTactics;
	}

}

void LaneAdvisor::analyzeSituation()
{		
	cg->getWizards(cg->getAlliesFaction(), allies);
	cg->getWizards(cg->getEnemiesFaction(), enemies);
	
	calcWizardsByLane();

	for (int i = 0; i < model::_LANE_COUNT_; ++i)
	{
		beginBalance[i] = wizardsByLane[i][0].size() - wizardsByLane[i][1].size();

		//TODO Ó÷åñòü áàøíè

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
		int worstBalance = resultBalance[worstLane];
				
		if (worstBalance >= 0)
		{
			break;//TODO
		}
		
		int needCount = worstBalance == 0 ? 1 : std::abs(worstBalance);
		int reinforcementCount = needCount > wizardsReserve.size() ? wizardsReserve.size() : needCount;

		while(reinforcementCount > 0)
		{
			const model::Wizard * wizard = wizardsReserve.front();	
			wizardsReserve.pop_front();
			--reinforcementCount;

			model::LaneType reserveLane = cg->whatLane(Point2D(*wizard));
			--(resultBalance[reserveLane]);

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
	int worstBalance = 5000;
	model::LaneType worstLane = model::_LANE_UNKNOWN_;

	for (int i = 0; i < model::_LANE_COUNT_; ++i)
	{
		if (resultBalance[i] < worstBalance)
		{
			worstLane = (model::LaneType) i;
			worstBalance = resultBalance[i];
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