#pragma once

#ifndef _DAV_ADVISOR_H_
#define _DAV_ADVISOR_H_

#include <list>
#include <map>
#include "DavGameEnvironment.h"
#include "DavMap.h"
#include "DavCartographer.h"
#include "DavTactics.h"
#include "DavGeometry.h"

namespace dav
{

	class Advisor
	{
	private:

		class Stage
		{
		public:
			enum Goal
			{
				sgReachPoint = 0,
				sgDestroyEnemy,
				sgRetreatToPoint
			};

		private:
			Point2D point;
			Goal goal;
		
		public:

			Stage(const Point2D & _point, Goal _goal): point(_point), goal(_goal)
			{

			}

			Goal getGoal() const
			{
				return goal;
			}

			const Point2D & getPoint() const
			{
				return point;
			}

		};

		const double LOW_HEALTH = 0.23;
		const int ORDER_COUNTDOWN = 600;
		int orderCountdown;
				
		GameEnvironment * env;
		Cartographer * cg;

		model::LaneType curLane;
		std::list<Stage> plan;
		std::list<Stage>::iterator stageIt;

		model::LaneType orderLane;
		model::SkillType orderSkill;
		
		MoveTactics * moveTactics;
		BattleTactics * battleTactics;
		Tactics * curTactics;

		model::SkillType skillByLevel[30];
		int lastLevelUp;
		void skillLearn();
		model::SkillType getSkill(int level);

		void prepare();
		void preparePlan();
		void useLane(model::LaneType lane);
		void prepareTopLanePlan();
		void prepareMiddleLanePlan();
		void prepareBottomLanePlan();
		void prepareTactics();
		void prepareSkills();
		
		void giveOrders();
		void executeOrders();
		void readMessages();
		
		void onRespawn();

		bool isBaseInDanger();		
		void retreatToBase();
		
		bool isLowHealth();		
		void retreatToNearAlliedBuilding();
		
		bool ifMetEnemy();	
		bool isEnemyArea();
		void joinBattle();

		void useStageTactics();
		void nextStage();

		bool isValidLane(model::LaneType lane) const
		{
			return lane == model::LANE_TOP || lane == model::LANE_MIDDLE || lane == model::LANE_BOTTOM;
		}


	public:
		Advisor(GameEnvironment * env);

		void work();

		~Advisor();
	};

	class LaneAdvisor
	{
	private:
						
		GameEnvironment * env;
		Cartographer * cg;

		std::map<const model::Wizard *, model::LaneType> beginWizardLane;
		std::map<const model::Wizard *, model::LaneType> moveWizardLane;
		
		std::list<const model::Wizard *> allies, enemies;
		
		std::vector<const model::Wizard *> wizardsByLane[model::_LANE_COUNT_][2];
		int beginBalance[model::_LANE_COUNT_];
		int beginReserve[model::_LANE_COUNT_];

		int resultBalance[model::_LANE_COUNT_];
		std::list<const model::Wizard *> wizardsReserve;		
		
		void calcWizardsByLane();
		void calcWizardsReserve();		
		model::LaneType getLaneToMove(const model::Wizard * wizard);

	public:

		LaneAdvisor(GameEnvironment * _env, Cartographer * _cg) : env(_env), cg(_cg)
		{

		}

		void analyzeSituation();
		void balance();
		void getRecommendedOrders(std::vector<model::Message> & result);
		model::LaneType getSelfLane();
		model::LaneType getWorstLane();
	};
		
}

#endif