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

	prepareMarkers();
	prepareDefaultWays();

	respawnDetected = false;

	alliesFaction = env->self->getFaction();
	enemiesFaction = alliesFaction == FACTION_ACADEMY ? FACTION_RENEGADES : FACTION_ACADEMY;
}

void Cartographer::prepareMarkers()
{
	//Союзные строения
	markers[AB].setPosition(400, 3600);
	markers[MAT1].setPosition(902, 2768);
	markers[MAT2].setPosition(1929, 2400);
	markers[TAT1].setPosition(50, 2693);
	markers[TAT2].setPosition(350, 1656);
	markers[BAT1].setPosition(1370, 3650);
	markers[BAT2].setPosition(2312, 3950);
		
	//Вражеские строения
	markers[EB].setPosition(3600, 400);
	markers[MET1].setPosition(2100, 1600);
	markers[MET2].setPosition(3100, 1200);
	markers[TET1].setPosition(1700, 50);
	markers[TET2].setPosition(2600, 350);
	markers[BET1].setPosition(3650, 2350);
	markers[BET2].setPosition(3950, 1300);

	//Свободное место рядом
	nearMarkers[AB] = markers[AB].getShift(250.0, 250.0);
	nearMarkers[EB] = markers[EB].getShift(-250.0, -250.0);
	nearMarkers[TAT1] = markers[TAT1].getShift(250.0, 250.0);
	nearMarkers[TAT2] = markers[TAT2].getShift(-250.0, 250.0);
	nearMarkers[TET1] = markers[TET1].getShift(-150.0, +150.0);
	nearMarkers[TET2] = markers[TET2].getShift(-150, -250.0);
	nearMarkers[MAT1] = markers[MAT1].getShift(0, 250.0);
	nearMarkers[MAT2] = markers[MAT2].getShift(0, -250.0);
	nearMarkers[MET1] = markers[MET1].getShift(0, +250.0);
	nearMarkers[MET2] = markers[MET2].getShift(-100.0, 0);
	nearMarkers[BAT1] = markers[BAT1].getShift(-100.0, +250.0);
	nearMarkers[BAT2] = markers[BAT2].getShift(-100, -250.0);
	nearMarkers[BET1] = markers[BET1].getShift(250.0, 100.0);
	nearMarkers[BET2] = markers[BET2].getShift(-100.0, 250.0);

}

const Point2D &  Cartographer::getNearestСollectionPoint(const Point2D & forPoint) const
{
	int index = AB;
	double minDistance = std::numeric_limits<double>::max();
	double distance;

	for (int i = 0; i < _PointMarker_Size; ++i)
	{
		const Point2D & point = nearMarkers[i];
		distance = forPoint.getDistanceTo(point);
		if (distance < minDistance)
		{
			minDistance = distance;
			index = i;
			continue;
		}
	}

	return nearMarkers[index];
}

void Cartographer::prepareDefaultWays()
{
	std::vector<Point2D> & topLane = laneWaypoints[model::LANE_TOP];
	std::vector<Point2D> & middleLane = laneWaypoints[model::LANE_MIDDLE];
	std::vector<Point2D> & bottomLane = laneWaypoints[model::LANE_BOTTOM];

	topLane.push_back(Point2D(200, 3400));
	topLane.push_back(Point2D(200, 3000));
	topLane.push_back(Point2D(200, 2610));
	topLane.push_back(Point2D(200, 2400));
	topLane.push_back(Point2D(200, 2000));
	topLane.push_back(Point2D(200, 1720));
	topLane.push_back(Point2D(200, 1320));
	topLane.push_back(Point2D(200, 1000));
	topLane.push_back(Point2D(300, 700));
	topLane.push_back(Point2D(700, 200));
	topLane.push_back(Point2D(1000, 250));
	topLane.push_back(Point2D(1200, 250));
	topLane.push_back(Point2D(1600, 250));
	topLane.push_back(Point2D(2000, 250));
	topLane.push_back(Point2D(2400, 150));
	topLane.push_back(Point2D(2700, 150));
	topLane.push_back(Point2D(3200, 150));
	topLane.push_back(Point2D(3400, 150));

	middleLane.push_back(Point2D(300, 3300));
	middleLane.push_back(Point2D(800, 3200));
	middleLane.push_back(Point2D(1000, 3000));
	middleLane.push_back(Point2D(1200, 2800));
	middleLane.push_back(Point2D(1400, 2600));
	middleLane.push_back(Point2D(1600, 2400));
	middleLane.push_back(Point2D(1800, 2200));
	middleLane.push_back(Point2D(2200, 2200));
	middleLane.push_back(Point2D(2200, 2000));
	middleLane.push_back(Point2D(2400, 1600));
	middleLane.push_back(Point2D(2600, 1400));
	middleLane.push_back(Point2D(2800, 1200));
	middleLane.push_back(Point2D(3000, 1000));
	middleLane.push_back(Point2D(3200, 800));
	middleLane.push_back(Point2D(3400, 600));

	bottomLane.push_back(Point2D(200, 3800));
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
	bottomLane.push_back(Point2D(3850, 1200));
	bottomLane.push_back(Point2D(3850, 800));
	bottomLane.push_back(Point2D(3850, 650));
}

void Cartographer::update()
{
	alliesNearBase.clear();
	enemiesNearBase.clear();
	respawnDetected = false;

	int tickCount = env->game->getTickCount();
	if (tickCount > 1200)
	{		
		respawnDetected = self.getCenter().getDistanceTo(*(env->self)) > 70.0;
	}
		
	self = Object2D(*(env->self));
	const Point2D & selfPos = self.getCenter();
		
	locMap.init(*(env->self));

	const std::vector<Tree>& trees = env->world->getTrees();
	for (int i = 0; i < trees.size(); ++i) {
		locMap.add(trees[i]);
	}

	const std::vector<Minion>& minions = env->world->getMinions();
	for (int i = 0; i < minions.size(); ++i) {
		const Minion & minion = minions[i];
		checkNearBase(minion);
		locMap.add(minion);
	}
	
	const std::vector<Wizard>& wizards = env->world->getWizards();
	for (int i = 0; i < wizards.size(); ++i) {
		const Wizard & wizard = wizards[i];
		if (wizard.isMe())
		{
			continue;
		}
		checkNearBase(wizard);
		locMap.add(wizard);
	}

	const std::vector<Building>& buildings = env->world->getBuildings();
	for (int i = 0; i < buildings.size(); ++i) {
		locMap.add(buildings[i]);
	}

	locMap.fixing();

#ifdef DEBUG_MAP
	if (env->self->isMaster())
	{
		locMap.saveToFile();//TODO
	}
#endif // DEBUG_MAP

}

bool Cartographer::checkNearBase(const model::LivingUnit & unit)
{
	const Point2D & basePos = markers[AB];

	if (basePos.getDistanceTo(unit) < NEAR_BASE)
	{
		if (unit.getFaction() == alliesFaction)
		{
			alliesNearBase.push_back(&unit);
		}
		else if (unit.getFaction() == enemiesFaction)
		{
			enemiesNearBase.push_back(&unit);
		}
		return true;
	}
	
	return false;
}

bool Cartographer::calcWayByLane(const Point2D & endPoint, std::vector<Point2D> & result) const
{
	const Point2D & beginPoint = self.getCenter();
	model::LaneType lane = getBestLane(beginPoint, endPoint);
	calcLaneFragment(beginPoint, endPoint, lane, result);	
	return result.size();
}

model::LaneType Cartographer::getBestLane(const Point2D & beginPoint, const Point2D & endPoint) const
{
	const int size = 3;
	double overheads[size];

	overheads[LANE_TOP] = calcOverheads(beginPoint, endPoint, model::LANE_TOP);
	overheads[LANE_MIDDLE] = calcOverheads(beginPoint, endPoint, model::LANE_MIDDLE);
	overheads[LANE_BOTTOM] = calcOverheads(beginPoint, endPoint, model::LANE_BOTTOM);

	model::LaneType bestLaneIndex = LANE_MIDDLE;
	double minOverheads = 8000;

	for (int i = 0; i < size; ++i)
	{
		if (overheads[i] < minOverheads)
		{
			minOverheads = overheads[i];
			bestLaneIndex = (model::LaneType) i;
		}
	}

	return bestLaneIndex;
}

double Cartographer::calcOverheads(const Point2D & beginPoint, const Point2D & endPoint, model::LaneType lane) const
{
	const std::vector<Point2D> & points = laneWaypoints[lane];
	int indexNearBegin = beginPoint.findNearestIndex(points);
	int indexNearEnd = endPoint.findNearestIndex(points);

	return points[indexNearBegin].getDistanceTo(beginPoint) + points[indexNearEnd].getDistanceTo(endPoint);
}

void Cartographer::calcLaneFragment(const Point2D & beginPoint, const Point2D & endPoint, model::LaneType lane, std::vector<Point2D> & result) const
{
	result.clear();

	const std::vector<Point2D> & points = laneWaypoints[lane];
	int indexBegin = beginPoint.findNearestIndex(points);
	int indexEnd = endPoint.findNearestIndex(points);

	double wayLen = calcWayLen(beginPoint, endPoint, indexBegin, indexEnd, lane);

	if (std::abs(indexEnd - indexBegin) > 3)
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

		double wayLenAB = calcWayLen(beginPoint, endPoint, alternativeBeginIndex, indexEnd, lane);
		double wayLenAE = calcWayLen(beginPoint, endPoint, indexBegin, alternativeEndIndex, lane);

		if (wayLenAB < wayLen)
		{
			indexBegin = alternativeBeginIndex;
		}

		if (wayLenAE < wayLen)
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

double Cartographer::calcWayLen(const Point2D & beginPoint, const Point2D & endPoint, int laneBegin, int laneEnd, model::LaneType lane) const
{
	double len = 0;
	const Point2D * point = &beginPoint;
	
	const std::vector<Point2D> & points = laneWaypoints[lane];
	if (laneBegin < laneEnd)
	{		
		for (int i = laneBegin; i <= laneEnd; ++i)
		{
			len += point->getDistanceTo(points[i]);
			point = &(points[i]);
		}
		len += point->getDistanceTo(endPoint);
	}
	else
	{		
		for (int i = laneBegin; i >= laneEnd; --i)
		{
			len += point->getDistanceTo(points[i]);
			point = &(points[i]);
		}
		len += point->getDistanceTo(endPoint);
	}

	return len;
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

const model::Building * Cartographer::getNearAlliedBuilding() const
{
	const model::Building * nearBuilding = nullptr;
	double nearDistance = 4000.0;

	const std::vector<Building>& buildings = env->world->getBuildings();
	for (int i = 0; i < buildings.size(); ++i) {
		const Building & building = buildings[i];
		if (building.getFaction() != alliesFaction)
		{
			continue;
		}

		double distance = self.getCenter().getDistanceTo(building);
		if (distance < nearDistance)
		{
			nearDistance = distance;
			nearBuilding = &building;
		}
	}

	return nearBuilding;
}

model::LaneType Cartographer::whatLane(const Point2D & point) const
{
	model::LaneType lane = model::_LANE_UNKNOWN_;
	double minDistance = 4000.0;
	
	double distance[3];
	for (int i = 0; i < 3; ++i)
	{
		const std::vector<Point2D> & lanePoints = laneWaypoints[i];
		int indexNear = point.findNearestIndex(lanePoints);
		
		distance[i] = lanePoints[indexNear].getDistanceTo(point);

		if (distance[i] < minDistance)
		{
			minDistance = distance[i];
			lane = model::LaneType(i);
		}
	}
	
	return lane;
}

bool compare(const Wizard *p1, const Wizard *p2) 
{ 
	return p1->getId() < p2->getId();
}

void Cartographer::getWizards(model::Faction faction, std::list<const model::Wizard *> & result) const
{
	result.clear();

	const std::vector<model::Wizard>& wizards = env->world->getWizards();
	for (int i = 0; i < wizards.size(); ++i) {
		const model::Wizard & wizard = wizards[i];
		
		if (wizard.isMe())
		{
			continue;
		}		
		else if (wizard.getFaction() != faction)
		{
			continue;
		}
		
		result.push_back(&wizard);
	}

	result.sort(compare);
}

Cartographer::~Cartographer()
{

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
