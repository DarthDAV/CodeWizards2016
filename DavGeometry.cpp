#include "DavGeometry.h"

#include <cstdlib>
#include <limits>

using namespace dav;

int Point2D::findNearestIndex(const std::vector<Point2D> & wherePoints) const
{
	int index = -1;
	double minDistance = std::numeric_limits<double>::max();
	double distance;

	for (int i = 0; i < wherePoints.size(); ++i)
	{
		const Point2D & point = wherePoints[i];
		distance = getDistanceTo(point);
		if (distance < minDistance)
		{
			minDistance = distance;
			index = i;
			continue;
		}
	}

	return index;
}



