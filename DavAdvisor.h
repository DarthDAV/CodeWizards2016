#pragma once

#ifndef _DAV_ADVISOR_H_
#define _DAV_ADVISOR_H_

#include <list>
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
				sgDestroyEnemy
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

		std::list<Stage> plan;
		std::list<Stage>::iterator stageIt;
						
		GameEnvironment * env;
		Cartographer * cg;

		MoveTactics * moveTactics;
		BattleTactics * battleTactics;

		Tactics * curTactics;


		model::SkillType skillByLevel[30];

		void prepare();

		bool preparePlan();
		void prepareTopLanePlan();
		void prepareMiddleLanePlan();
		void prepareBottomLanePlan();

		bool prepareTactics();

		void useStageTactics();

		bool isEnemyArea();

		int lastLevelUp;
		void skillLearn();
		model::SkillType getSkill(int level);

	public:
		Advisor(GameEnvironment * env);

		void work();

		~Advisor();
	};

}

#endif