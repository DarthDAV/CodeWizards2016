#include "DavAdvisor.h"

#include "DavGeometry.h"

using namespace dav;

Advisor::Advisor(GameEnvironment * env) : curTactics(nullptr)
{
	this->env = env;
	cg = new Cartographer(env);

	dav::Tactics::env = env;
	dav::Tactics::cg = cg;

	lastLevelUp = 0;

	prepare();	
}

void Advisor::prepare()
{	
	prepareTactics();
	preparePlan();

	skillByLevel[1] = model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1;
	skillByLevel[2] = model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1;
	skillByLevel[3] = model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2;
	skillByLevel[4] = model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2;
	skillByLevel[5] = model::SKILL_SHIELD;

	skillByLevel[6] = model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1;
	skillByLevel[7] = model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1;
	skillByLevel[8] = model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2;
	skillByLevel[9] = model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2;
	skillByLevel[10] = model::SKILL_HASTE;

	skillByLevel[11] = model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1;
	skillByLevel[12] = model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1;
	skillByLevel[13] = model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2;
	skillByLevel[14] = model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2;
	skillByLevel[15] = model::SKILL_FROST_BOLT;
	
		/*
		SKILL_RANGE_BONUS_PASSIVE_1 = 0,
		SKILL_RANGE_BONUS_AURA_1 = 1,
		SKILL_RANGE_BONUS_PASSIVE_2 = 2,
		SKILL_RANGE_BONUS_AURA_2 = 3,
		SKILL_ADVANCED_MAGIC_MISSILE = 4,

		SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1 = 5,
		SKILL_MAGICAL_DAMAGE_BONUS_AURA_1 = 6,
		SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2 = 7,
		SKILL_MAGICAL_DAMAGE_BONUS_AURA_2 = 8,
		SKILL_FROST_BOLT = 9,

		SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1 = 10,
		SKILL_STAFF_DAMAGE_BONUS_AURA_1 = 11,
		SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2 = 12,
		SKILL_STAFF_DAMAGE_BONUS_AURA_2 = 13,
		SKILL_FIREBALL = 14,

		SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1 = 15,
		SKILL_MOVEMENT_BONUS_FACTOR_AURA_1 = 16,
		SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2 = 17,
		SKILL_MOVEMENT_BONUS_FACTOR_AURA_2 = 18,
		SKILL_HASTE = 19,

		SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1 = 20,
		SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1 = 21,
		SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2 = 22,
		SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2 = 23,
		SKILL_SHIELD = 24,
		*/

}

bool Advisor::preparePlan()
{
	curTactics = nullptr;
	plan.clear();
	stageIt = plan.end();	
	
	int lane = -1;

	const std::vector<model::Message> & messages = env->self->getMessages();
	for (int i = 0; i < messages.size(); ++i)
	{
		lane = messages[i].getLane();
	}

	if (lane < 0)
	{
		lane = env->randomInt(0, 2);
	}

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

	return true;
}

void Advisor::prepareTopLanePlan()
{
	plan.clear();

	Point2D nearTAT1(cg->getPoint(Cartographer::TAT1).getShift(250.0, 250.0));
	Point2D nearTAT2(cg->getPoint(Cartographer::TAT2).getShift(-250.0, 250.0));
	Point2D nearTET1(cg->getPoint(Cartographer::TET1).getShift(-150.0, +150.0));
	Point2D nearTET2(cg->getPoint(Cartographer::TET2).getShift(-150, -250.0));
	Point2D nearEB(cg->getPoint(Cartographer::EB).getShift(-200.0, -250.0));

	plan.push_back(Stage(nearTAT1, Stage::sgReachPoint));
	plan.push_back(Stage(nearTAT2, Stage::sgReachPoint));
	plan.push_back(Stage(nearTET1, Stage::sgReachPoint));
	plan.push_back(Stage(nearTET2, Stage::sgReachPoint));
	plan.push_back(Stage(nearEB, Stage::sgReachPoint));

	stageIt = plan.begin();
}
	
void Advisor::prepareMiddleLanePlan()
{
	plan.clear();
	
	Point2D nearMAT1(cg->getPoint(Cartographer::MAT1).getShift(0, 250.0));
	Point2D nearMAT2(cg->getPoint(Cartographer::MAT2).getShift(0, -250.0));
	Point2D nearMET1(cg->getPoint(Cartographer::MET1).getShift(0, +250.0));
	Point2D nearMET2(cg->getPoint(Cartographer::MET2).getShift(-100.0, 0));
	Point2D nearEB(cg->getPoint(Cartographer::EB).getShift(-200.0, -200.0));

	plan.push_back(Stage(nearMAT1, Stage::sgReachPoint));
	plan.push_back(Stage(nearMAT2, Stage::sgReachPoint));
	plan.push_back(Stage(nearMET1, Stage::sgReachPoint));
	plan.push_back(Stage(nearMET2, Stage::sgReachPoint));
	plan.push_back(Stage(nearEB, Stage::sgReachPoint));
	
	stageIt = plan.begin();
}

void Advisor::prepareBottomLanePlan()
{
	plan.clear();

	Point2D nearBAT1(cg->getPoint(Cartographer::BAT1).getShift(-100.0, +250.0));
	Point2D nearBAT2(cg->getPoint(Cartographer::BAT2).getShift(-100, -250.0));
	Point2D nearBET1(cg->getPoint(Cartographer::BET1).getShift(250.0, 100.0));
	Point2D nearBET2(cg->getPoint(Cartographer::BET2).getShift(-100.0, 250.0));
	Point2D nearEB(cg->getPoint(Cartographer::EB).getShift(+300.0, +200.0));

	plan.push_back(Stage(nearBAT1, Stage::sgReachPoint));
	plan.push_back(Stage(nearBAT2, Stage::sgReachPoint));
	plan.push_back(Stage(nearBET1, Stage::sgReachPoint));
	plan.push_back(Stage(nearBET2, Stage::sgReachPoint));
	plan.push_back(Stage(nearEB, Stage::sgReachPoint));

	stageIt = plan.begin();
}

bool Advisor::prepareTactics()
{
	moveTactics = new MoveTactics();
	battleTactics = new BattleTactics();
	return true;
}

Advisor::~Advisor()
{
	delete moveTactics;
	delete battleTactics;
}

void Advisor::work()
{
	skillLearn();
		
	cg->update();
	
	if (cg->isRespawn())
	{
		preparePlan();
	}

	if (curTactics == nullptr)
	{
		useStageTactics();
	}

	if (isEnemyArea() && stageIt->getGoal() != Stage::sgDestroyEnemy)
	{		
		stageIt = plan.insert(stageIt, Stage(cg->getSelf().getCenter(), Stage::sgDestroyEnemy));
		useStageTactics();
	}
	
	curTactics->work();

	if (curTactics->getStatus() == Tactics::tsCompleted)
	{
		if (stageIt->getGoal() == Stage::sgDestroyEnemy)
		{
			stageIt = plan.erase(stageIt);
		}
		else
		{
			++stageIt;
		}
		
		if (stageIt == plan.end())
		{
			stageIt = plan.begin();//TODO
		}

		useStageTactics();
	}
	else if (curTactics->getStatus() != Tactics::tsInProgress)
	{
		//TODO
		useStageTactics();
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

}

bool Advisor::isEnemyArea()
{
	double distance = cg->getNearEnemyDistance();
	return distance <= env->game->getWizardCastRange();
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