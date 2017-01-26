#pragma once

#ifndef _MY_STRATEGY_H_
#define _MY_STRATEGY_H_

#include "Strategy.h"
#include "DavAdvisor.h"
#include "DavGameEnvironment.h"

class MyStrategy : public Strategy {

private:

	dav::GameEnvironment * env;
	dav::Advisor * advisor;

public:
    MyStrategy();

    void move(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move) override;

	~MyStrategy();
};

#endif
