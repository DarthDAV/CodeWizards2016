#include "DavCartographer.h"

#include "model/Game.h"
#include "model/Move.h"
#include "model/World.h"

using namespace std;
using namespace model;
using namespace dav;

Cartographer::Cartographer(GameEnvironment * env) : self(*(env->self))
{
	this->env = env;

	double worldSize = env->game->getMapSize();
	map = new Map(worldSize, worldSize);

	prepareMarkers();
	prepareDefaultWays();

	respawnDetected = true;

	enemiesFaction = env->self->getFaction() == FACTION_ACADEMY ? FACTION_RENEGADES : FACTION_ACADEMY;	
}

void Cartographer::prepareMarkers()
{
	//Союзные строения
	Markers[AB].setPosition(400, 3600);
	Markers[MAT1].setPosition(902, 2768);
	Markers[MAT2].setPosition(1929, 2400);
	Markers[TAT1].setPosition(50, 2693);
	Markers[TAT2].setPosition(350, 1656);
	Markers[BAT1].setPosition(1370, 3650);
	Markers[BAT2].setPosition(2312, 3950);
		
	//Вражеские строения
	Markers[EB].setPosition(3600, 400);
	Markers[MET1].setPosition(2100, 1600);
	Markers[MET2].setPosition(3100, 1200);
	Markers[TET1].setPosition(1700, 50);
	Markers[TET2].setPosition(2600, 350);
	Markers[BET1].setPosition(3650, 2350);
	Markers[BET2].setPosition(3950, 1300);
}

void Cartographer::prepareDefaultWays()
{
	std::vector<Point2D> & topLane = laneWaypoints[model::LANE_TOP];
	std::vector<Point2D> & middleLane = laneWaypoints[model::LANE_MIDDLE];
	std::vector<Point2D> & bottomLane = laneWaypoints[model::LANE_BOTTOM];

	topLane.push_back(Point2D(100, 3400));
	topLane.push_back(Point2D(200, 3000));
	topLane.push_back(Point2D(200, 2610));
	topLane.push_back(Point2D(200, 2400));
	topLane.push_back(Point2D(200, 2000));
	topLane.push_back(Point2D(100, 1720));
	topLane.push_back(Point2D(100, 1320));
	topLane.push_back(Point2D(100, 1000));
	topLane.push_back(Point2D(300, 700));
	topLane.push_back(Point2D(700, 200));
	topLane.push_back(Point2D(1000, 200));
	topLane.push_back(Point2D(1200, 300));
	topLane.push_back(Point2D(1600, 300));
	topLane.push_back(Point2D(2000, 300));
	topLane.push_back(Point2D(2400, 100));
	topLane.push_back(Point2D(2700, 100));
	topLane.push_back(Point2D(3200, 100));
	topLane.push_back(Point2D(3400, 100));

	middleLane.push_back(Point2D(200, 3400));
	middleLane.push_back(Point2D(600, 3400));
	middleLane.push_back(Point2D(800, 3000));
	middleLane.push_back(Point2D(1020, 3000));
	middleLane.push_back(Point2D(1200, 2800));
	middleLane.push_back(Point2D(1400, 2600));
	middleLane.push_back(Point2D(1600, 2600));
	middleLane.push_back(Point2D(1830, 2450));
	middleLane.push_back(Point2D(1830, 2200));
	middleLane.push_back(Point2D(2100, 2300));
	middleLane.push_back(Point2D(2300, 2300));
	middleLane.push_back(Point2D(2300, 2000));
	middleLane.push_back(Point2D(2500, 1600));
	middleLane.push_back(Point2D(2600, 1400));
	middleLane.push_back(Point2D(3000, 1000));
	middleLane.push_back(Point2D(3200, 1000));
	middleLane.push_back(Point2D(3400, 800));
	middleLane.push_back(Point2D(3500, 600));

	bottomLane.push_back(Point2D(100, 3900));
	bottomLane.push_back(Point2D(400, 3800));
	bottomLane.push_back(Point2D(800, 3800));
	bottomLane.push_back(Point2D(1280, 3800));
	bottomLane.push_back(Point2D(1600, 3800));
	bottomLane.push_back(Point2D(2000, 3800));
	bottomLane.push_back(Point2D(2400, 3800));
	bottomLane.push_back(Point2D(2800, 3800));
	bottomLane.push_back(Point2D(3200, 3800));
	bottomLane.push_back(Point2D(3400, 3800));
	bottomLane.push_back(Point2D(3800, 3800));
	bottomLane.push_back(Point2D(3800, 3600));
	bottomLane.push_back(Point2D(3800, 3200));
	bottomLane.push_back(Point2D(3800, 3000));
	bottomLane.push_back(Point2D(3800, 2600));
	bottomLane.push_back(Point2D(3800, 2200));
	bottomLane.push_back(Point2D(3800, 1800));
	bottomLane.push_back(Point2D(3800, 1400));
	bottomLane.push_back(Point2D(3900, 1200));
	bottomLane.push_back(Point2D(3900, 800));
	bottomLane.push_back(Point2D(3900, 650));
}

void Cartographer::update()
{
	int tickCount = env->game->getTickCount();
	if (tickCount == 0)
	{
		respawnDetected = true;
	}	
	else if (tickCount > 1200)
	{		
		respawnDetected = self.getCenter().getDistanceTo(*(env->self)) > 70.0;
	}
		
	self = Object2D(*(env->self));
	
	//TODO map->clear();
	locMap.init(*(env->self));

	const std::vector<Building>& buildings = env->world->getBuildings();
	const std::vector<Wizard>& wizards = env->world->getWizards();
	const std::vector<Minion>& minions = env->world->getMinions();
	const std::vector<Tree>& trees = env->world->getTrees();

	for (int i = 0; i < trees.size(); ++i) {
		const Tree & tree = trees[i];
		//TODO map->add(tree);
		locMap.add(tree);
	}

	for (int i = 0; i < buildings.size(); ++i) {
		const Building & building = buildings[i];
		//TODO map->add(building);
		locMap.add(building);
	}

	for (int i = 0; i < minions.size(); ++i) {
		const Minion & minion = minions[i];
		//TODO map->add(minion);
		locMap.add(minion);
	}

	for (int i = 0; i < wizards.size(); ++i) {
		const Wizard & wizard = wizards[i];
		if (wizard.isMe())
		{
			continue;
		}
		
		//TODO map->add(wizard);
		locMap.add(wizard);
	}

	locMap.fixing();
}

bool Cartographer::calcWayByLane(const Point2D & endPoint, std::vector<Point2D> & result) const
{
	const Point2D & beginPoint = self.getCenter();
	int laneIndex = getBestLane(beginPoint, endPoint);
	calcLaneFragment(beginPoint, endPoint, laneIndex, result);	
	return result.size();
}

int Cartographer::getBestLane(const Point2D & beginPoint, const Point2D & endPoint) const
{
	const int size = 3;
	double overheads[size];

	overheads[LANE_TOP] = calcOverheads(beginPoint, endPoint, model::LANE_TOP);
	overheads[LANE_MIDDLE] = calcOverheads(beginPoint, endPoint, model::LANE_MIDDLE);
	overheads[LANE_BOTTOM] = calcOverheads(beginPoint, endPoint, model::LANE_BOTTOM);

	int bestLaneIndex = LANE_MIDDLE;
	double minOverheads = 8000;

	for (int i = 0; i < size; ++i)
	{
		if (overheads[i] < minOverheads)
		{
			minOverheads = overheads[i];
			bestLaneIndex = i;
		}
	}

	return bestLaneIndex;
}

double Cartographer::calcOverheads(const Point2D & beginPoint, const Point2D & endPoint, int lane) const
{
	const std::vector<Point2D> & points = laneWaypoints[lane];
	int indexNearBegin = beginPoint.findNearestIndex(points);
	int indexNearEnd = endPoint.findNearestIndex(points);

	return points[indexNearBegin].getDistanceTo(beginPoint) + points[indexNearEnd].getDistanceTo(endPoint);
}

void Cartographer::calcLaneFragment(const Point2D & beginPoint, const Point2D & endPoint, int lane, std::vector<Point2D> & result) const
{
	result.clear();

	const std::vector<Point2D> & points = laneWaypoints[lane];
	int indexBegin = beginPoint.findNearestIndex(points);
	int indexEnd = endPoint.findNearestIndex(points);

	if (std::abs(indexEnd - indexBegin) > 2)
	{
		int alternativeBeginIndex = 0;
		int alternativeEndIndex = 0;

		if (indexBegin < indexEnd)
		{
			alternativeBeginIndex = indexBegin + 1;
			alternativeEndIndex = indexEnd - 1;
		}
		else
		{
			alternativeBeginIndex = indexBegin - 1;
			alternativeEndIndex = indexEnd + 1;
		}

		double ditanceBGW = points[indexBegin].getDistanceTo(points[alternativeBeginIndex]);
		double ditanceBLW = beginPoint.getDistanceTo(points[alternativeBeginIndex]);

		if (ditanceBGW > ditanceBLW)
		{
			indexBegin = alternativeBeginIndex;
		}

		double ditanceEGW = points[indexEnd].getDistanceTo(points[alternativeEndIndex]);
		double ditanceELW = endPoint.getDistanceTo(points[alternativeEndIndex]);

		if (ditanceEGW > ditanceELW)
		{
			indexEnd = alternativeEndIndex;
		}
	}

	result.push_back(beginPoint);

	if (indexBegin < indexEnd)
	{
		for (int i = indexBegin; i <= indexEnd; ++i)
		{
			result.push_back(points[i]);
		}
	}
	else
	{
		for (int i = indexBegin; i >= indexEnd; --i)
		{
			result.push_back(points[i]);
		}

	}

	result.push_back(endPoint);
}

bool Cartographer::calcWay(const Point2D & desiredEndPoint, std::vector<Point2D> & result) const
{
	return locMap.calcWay(desiredEndPoint, result);
}

/*
void Cartographer::getEnemies(std::vector<Object2D> & result) const
{
	result.clear();

	const std::vector<Building>& buildings = env->world->getBuildings();
	const std::vector<Wizard>& wizards = env->world->getWizards();
	const std::vector<Minion>& minions = env->world->getMinions();

	for (int i = 0; i < buildings.size(); ++i) {
		const Building & building = buildings[i];
		if (building.getFaction() != enemiesFaction)
		{
			continue;
		}
		else if (position.getDistanceTo(building) > scanRadius)
		{
			continue;
		}

		result.push_back(Object2D(building));
	}

	for (int i = 0; i < wizards.size(); ++i) {
		const Wizard & wizard = wizards[i];
		if (wizard.isMe())
		{
			continue;
		}
		else if (wizard.getFaction() != enemiesFaction)
		{
			continue;
		}
		else if (position.getDistanceTo(wizard) > scanRadius)
		{
			continue;
		}

		result.push_back(Object2D(wizard));
	}

	for (int i = 0; i < minions.size(); ++i) {
		const Minion & minion = minions[i];
		if (minion.getFaction() != enemiesFaction)
		{
			continue;
		}
		else if (position.getDistanceTo(minion) > scanRadius)
		{
			continue;
		}

		result.push_back(Object2D(minion));
	}

}*/

Cartographer::~Cartographer()
{
	delete map;
}

/*void Cartographer::getNearObjects(const Object2D & who, std::vector<Object2D> & result)
{
result.clear();

const int cellsSize = 9;
Map::Cell * cells[cellsSize];

Map::Cell * cell = map->findCell(who);

for (int i = 0; i < cell->buildings.size(); ++i) {
Object2D object(*(cell->buildings[i]));
result.push_back(object);
}

for (int i = 0; i < cell->wizards.size(); ++i) {
Object2D object(*(cell->wizards[i]));
result.push_back(object);
}

for (int i = 0; i < cell->minions.size(); ++i) {
Object2D object(*(cell->minions[i]));
result.push_back(object);
}

for (int i = 0; i < cell->trees.size(); ++i) {
Object2D object(*(cell->trees[i]));
result.push_back(object);
}
}

*/
