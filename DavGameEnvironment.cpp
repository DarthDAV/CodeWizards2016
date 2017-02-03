#include "DavGameEnvironment.h"

#include <cstdlib>
#include <algorithm>
#include <cstring>

using namespace dav;

GameEnvironment::GameEnvironment()
{
	self = 0;
	world = 0;
	game = 0;
	move = 0;

	std::memset(learnedSkills, 0, model::_SKILL_COUNT_*sizeof(bool));
}

void GameEnvironment::initializeRandom(long int seed)
{
	std::srand(seed);
}

bool GameEnvironment::randomBool() const
{	
	return rand() % 2;
}

double GameEnvironment::randomDouble(double a, double b) const
{
	double r = double(rand()) / double(RAND_MAX);

	return a + (b - a)*r;
}

int GameEnvironment::randomInt(int a, int b) const
{
	//TODO -1 1
	return a + ( rand() % (1 + b - a) );
}

double GameEnvironment::getMaxForwardSpeed() const
{
	return 100.0;//game->getWizardForwardSpeed();//TODO

}

double GameEnvironment::getMaxBackwardSpeed() const
{
	return 100.0;//game->getWizardBackwardSpeed();
}

double GameEnvironment::getMaxStrafeSpeed() const
{
	return 100.0;//game->getWizardStrafeSpeed();
}

bool GameEnvironment::getActionRechargeTime(model::ActionType actionType) const
{
	return std::max(self->getRemainingActionCooldownTicks(), (self->getRemainingCooldownTicksByAction()[actionType]) );
}

bool GameEnvironment::isSkillLeanded(model::SkillType skill) const
{
	const std::vector<model::SkillType> & selfSkills = self->getSkills();
	for (int i = 0; i < selfSkills.size(); ++i)
	{
		if (selfSkills[i] == skill)
		{
			return true;
		}
	}
	
	return false;
	//TODO return learnedSkills[skill];
}

GameEnvironment::~GameEnvironment()
{
}
