#pragma once

#ifndef _DAV_GAME_ENVIRONMENT_H_
#define _DAV_GAME_ENVIRONMENT_H_

#include "model/Game.h"
#include "model/Move.h"
#include "model/World.h"

namespace dav
{

	class GameEnvironment
	{
	private:
		bool learnedSkills[model::_SKILL_COUNT_];

	public:
		GameEnvironment();

		const model::Wizard * self;
		const model::World * world;
		const model::Game * game;
		model::Move * move;
		
		void setSkillLeanded(model::SkillType skill, bool learned)
		{
			learnedSkills[skill] = learned;
		}

		bool isSkillLeanded(model::SkillType skill) const;

		void initializeRandom(long int seed) ;
		bool randomBool() const;
		
		double randomDouble(double a, double b) const;

		int randomInt(int a, int b) const;

		double getMaxForwardSpeed() const;
		double getMaxBackwardSpeed() const;
		double getMaxStrafeSpeed() const;

		bool getActionRechargeTime(model::ActionType actionType) const;

		~GameEnvironment();
	};

}

#endif