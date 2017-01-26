#include "MyStrategy.h"

#include "DavCartographer.h"
#include "DavTactics.h"

MyStrategy::MyStrategy() 
{ 
	env = new dav::GameEnvironment();
	advisor = 0;
}

void MyStrategy::move(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move) {

	env->self = &self;
	env->world = &world;
	env->game = &game;
	env->move = &move;

	if (advisor == NULL)
	{
		env->initializeRandom(game.getRandomSeed());		
		advisor = new dav::Advisor(env);
	}

	advisor->work();
}

MyStrategy::~MyStrategy()
{
	delete env;

	if (advisor)
	{
		delete advisor;
	}

}


