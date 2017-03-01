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

		const double GROUP_RADIUS = 350.0;
		const double NEAR_BASE = 950.0;
		const double LOW_HEALTH = 0.40;
		const double NORM_HEALTH = 0.65;
		const int ORDER_COUNTDOWN = 400;
		int orderCountdown;
				
		GameEnvironment * env;
		Cartographer * cg;

		Point2D baseRetreatPoint;
		
		model::LaneType curLane;
		std::list<Stage> plan;
		std::list<Stage>::iterator stageIt;

		model::LaneType orderLane;
		model::SkillType orderSkill;
		
		MoveTactics * moveTactics;
		BattleTactics * battleTactics;
		RetreatTactics * retreatTactics;
		Tactics * curTactics;

		model::SkillType skillByLevel[30];
		int lastLevelUp;
		void skillLearn();
		model::SkillType getSkill(int level);

		void prepare();
		void preparePlan();
		model::LaneType getDefaultLane() const;
		void useLane(model::LaneType lane);
		void prepareTopLanePlan();
		void prepareMiddleLanePlan();
		void prepareBottomLanePlan();
		void prepareTactics();
		void prepareSkills();
		
		void giveOrders();
		void changeLaneIfNeed(model::LaneType toLane);
		void executeOrders();
		void readMessages();
		
		void onRespawn();

		bool isFight();
		bool isBaseInDanger();
		bool isRetreat();
		bool isRetreatToBase();
		bool isRetreatNotToBase();
		bool isNearBase();
		bool isOnBase();		
		void retreatToBase();
		void protectBase();
		void setUrgentStage(Stage & stage);
		void dropUrgentStage();
		void skipUrgentStageIfNeed();

		bool isLowHealth();	
		bool isNormHealth();
		void retreatToNearAlliedBuilding();
		void retreatToNearAlly();
		
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

		LaneAdvisor(GameEnvironment * _env, Cartographer * _cg); 

		void clear();
		void analyzeSituation();
		void balance();
		void getRecommendedOrders(std::vector<model::Message> & result);
		model::LaneType getSelfLane();
		model::LaneType getWorstLane();
	};
		
}

#endif