#include "DavCartographer.h"

#include <limits>
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
	nearMarkers[AB] = Point2D(600, 3400);
	nearMarkers[EB] = Point2D(3400, 600);
	nearMarkers[TAT1] = Point2D(200, 2700);
	nearMarkers[TAT2] = Point2D(200, 1700);
	nearMarkers[TET1] = Point2D(1400, 200);
	nearMarkers[TET2] = Point2D(2400, 200);
	nearMarkers[MAT1] = Point2D(1000, 3000);
	nearMarkers[MAT2] = Point2D(1800, 2200);
	nearMarkers[MET1] = Point2D(2200, 2000);
	nearMarkers[MET2] = Point2D(2600, 1400);
	nearMarkers[BAT1] = Point2D(1400, 3800);
	nearMarkers[BAT2] = Point2D(2400, 3800);
	nearMarkers[BET1] = Point2D(3800, 2600);
	nearMarkers[BET2] = Point2D(3800, 1600);

}

const Point2D &  Cartographer::getNearestCollectionPoint(const Point2D & forPoint) const
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
	int wizardId = (int)env->self->getId();
	double shift = 0;

	switch (wizardId) {
	case 1:	
	case 6:
		shift = -1.0;
		break;
	case 2:
	case 7:
		shift = 1.0;
		break;
	case 3:
	case 8:
		shift = (wizardId % 2) ? -1.0: 1.0;
		break;
	case 4:
	case 9:
		shift = -1.0;
		break;
	case 5:
	case 10:
		shift = 1.0;
		break;
	}

	shift *= 100.0;	
	
	std::vector<Point2D> & topLane = laneWaypoints[model::LANE_TOP];
	std::vector<Point2D> & middleLane = laneWaypoints[model::LANE_MIDDLE];
	std::vector<Point2D> & bottomLane = laneWaypoints[model::LANE_BOTTOM];

	topLane.push_back(Point2D(200 + shift, 3500));
	topLane.push_back(Point2D(200 + shift, 3350));
	topLane.push_back(Point2D(200 + shift, 3200));
	topLane.push_back(Point2D(200 + shift, 3000));
	topLane.push_back(Point2D(200 + shift, 2850));
	topLane.push_back(Point2D(200 + shift, 2700));
	topLane.push_back(Point2D(200 + shift, 2650));
	topLane.push_back(Point2D(200 + shift, 2400));
	topLane.push_back(Point2D(200 + shift, 2200));
	topLane.push_back(Point2D(200 + shift, 2000));
	topLane.push_back(Point2D(200 + shift, 1850));
	topLane.push_back(Point2D(200 + shift, 1700));
	topLane.push_back(Point2D(200 + shift, 1550));
	topLane.push_back(Point2D(200 + shift, 1300));
	topLane.push_back(Point2D(200 + shift, 1150));
	topLane.push_back(Point2D(200 + shift, 1000));
	topLane.push_back(Point2D(300 + shift, 700));
	topLane.push_back(Point2D(700 + shift, 200));
	topLane.push_back(Point2D(1000, 200 + shift));
	topLane.push_back(Point2D(1200, 200 + shift));
	topLane.push_back(Point2D(1400, 200 + shift));
	topLane.push_back(Point2D(1600, 200 + shift));
	topLane.push_back(Point2D(1800, 200 + shift));
	topLane.push_back(Point2D(2000, 200 + shift));
	topLane.push_back(Point2D(2200, 200 + shift));
	topLane.push_back(Point2D(2400, 200 + shift));
	topLane.push_back(Point2D(2550, 200 + shift));
	topLane.push_back(Point2D(2700, 200 + shift));
	topLane.push_back(Point2D(2850, 200 + shift));
	topLane.push_back(Point2D(3000, 200 + shift));
	topLane.push_back(Point2D(3200, 200 + shift));
	topLane.push_back(Point2D(3400, 200 + shift));

	middleLane.push_back(Point2D(200, 3770));
	middleLane.push_back(Point2D(300, 3770));
	middleLane.push_back(Point2D(400, 3770));
	middleLane.push_back(Point2D(500, 3770));
	middleLane.push_back(Point2D(700, 3770));
	middleLane.push_back(Point2D(700 , 3600));
	middleLane.push_back(Point2D(700, 3400));
	middleLane.push_back(Point2D(700, 3100));
	middleLane.push_back(Point2D(850, 3100));
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

	bottomLane.push_back(Point2D(400, 3800 + shift));
	bottomLane.push_back(Point2D(500, 3800 + shift));
	bottomLane.push_back(Point2D(600, 3800 + shift));
	bottomLane.push_back(Point2D(800, 3800 + shift));
	bottomLane.push_back(Point2D(1000, 3800 + shift));
	bottomLane.push_back(Point2D(1200, 3800 + shift));
	bottomLane.push_back(Point2D(1400, 3800 + shift));
	bottomLane.push_back(Point2D(1600, 3800 + shift));
	bottomLane.push_back(Point2D(1800, 3800 + shift));
	bottomLane.push_back(Point2D(2000, 3800 + shift));
	bottomLane.push_back(Point2D(2200, 3800 + shift));
	bottomLane.push_back(Point2D(2400, 3800 + shift));
	bottomLane.push_back(Point2D(2600, 3800 + shift));
	bottomLane.push_back(Point2D(2800, 3800 + shift));
	bottomLane.push_back(Point2D(3000, 3800 + shift));
	bottomLane.push_back(Point2D(3200, 3800 + shift));
	bottomLane.push_back(Point2D(3400, 3800 + shift));
	bottomLane.push_back(Point2D(3600, 3800 + shift));
	bottomLane.push_back(Point2D(3800, 3800 + shift));
	bottomLane.push_back(Point2D(3800 + shift, 3600 + shift));
	bottomLane.push_back(Point2D(3800 + shift, 3200 + shift));
	bottomLane.push_back(Point2D(3800 + shift, 3000));
	bottomLane.push_back(Point2D(3800 + shift, 2800));
	bottomLane.push_back(Point2D(3800 + shift, 2600));
	bottomLane.push_back(Point2D(3800 + shift, 2400));
	bottomLane.push_back(Point2D(3800 + shift, 2200));
	bottomLane.push_back(Point2D(3800 + shift, 2000));
	bottomLane.push_back(Point2D(3800 + shift, 1800));
	bottomLane.push_back(Point2D(3800 + shift, 1600));
	bottomLane.push_back(Point2D(3800 + shift, 1400));
	bottomLane.push_back(Point2D(3850 + shift, 1200));
	bottomLane.push_back(Point2D(3850 + shift, 1000));
	bottomLane.push_back(Point2D(3850 + shift, 800));
	bottomLane.push_back(Point2D(3850 + shift, 650));
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
#ifdef DEBUG_MAP
	bool isSucces = locMap.calcWay(desiredEndPoint, result);
	//if (env->self->isMaster())
	//{
		locMap.saveToFile();
	//}
	return isSucces;
#else
	return locMap.calcWay(desiredEndPoint, result);
#endif // DEBUG_MAP

}
const model::Building * Cartographer::getNearAlliedBuilding() const
{
	const model::Building * nearBuilding = nullptr;
	double nearDistance = std::numeric_limits<double>::max();

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
	model::LaneType lane = model::LANE_MIDDLE;
	double minDistance = std::numeric_limits<double>::max();;
	
	double distance[3];
	for (int i = 0; i < model::_LANE_COUNT_; ++i)
	{
		const std::vector<Point2D> & lanePoints = laneWaypoints[i];
		int indexNear = point.findNearestIndex(lanePoints);
		if (indexNear < 0)
		{
			continue;
		}
		
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