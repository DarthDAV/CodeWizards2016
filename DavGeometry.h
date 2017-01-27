#pragma once

#ifndef _DAV_GEOMETRY_H_
#define _DAV_GEOMETRY_H_

//#define PI 3.14159265358979323846
//#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#include "DavGameEnvironment.h"

namespace dav
{
	
	//Вспомогательный класс для хранения позиций на карте.
	class Point2D {
	private:
		double x;
		double y;

	public:

		Point2D()
		{
			this->x = -1;
			this->y = -1;
		}

		Point2D(double x, double y) {
			this->x = x;
			this->y = y;
		}

		Point2D(const model::Unit & unit) {
			this->x = unit.getX();
			this->y = unit.getY();
		}

		double getX() const  {
			return x;
		} 

		double getY() const {
			return y;
		} 

		void setPosition(double _x, double _y)
		{
			x = _x;
			y = _y;
		}
		
		void setPosition(const Point2D & point)
		{
			x = point.x;
			y = point.y;
		}

		bool operator ==(const Point2D &other) const
		{
			return x == other.x && y == other.y;
		}

		double getDistanceTo(double x, double y) const  {
			return std::hypot(this->x - x, this->y - y);
		} 

		double getDistanceTo(const Point2D & point) const  {
			return getDistanceTo(point.x, point.y);
		} 

		double getDistanceTo(const model::Unit & unit) const {
			return getDistanceTo(unit.getX(), unit.getY());
		} 

		static double getCentersDistance(const model::CircularUnit & some, const model::CircularUnit & other)
		{
			return std::hypot(some.getX() - other.getX(), some.getY() - other.getY());
		}

		bool isCloserThan(const Point2D & point, double distance) const
		{
			return getDistanceTo(point.x, point.y) <= distance;
		} 

		bool isCloserThan(const model::Unit & unit, double distance) const
		{
			return getDistanceTo(unit.getX(), unit.getY()) <= distance;
		} 

		int findNearestIndex(const std::vector<Point2D> & wherePoints) const;

		Point2D getShift(double xShift, double yShift) const
		{
			return Point2D(x + xShift, y + yShift);
		}
	};

	class Object2D
	{
	private:
		Point2D center;
		double radius;

	public:

		Object2D(const model::CircularUnit & circularUnit) : center(circularUnit)
		{
			this->radius = circularUnit.getRadius();
		}

		Object2D(const Point2D & _center, double _radius) : center(_center), radius(_radius)
		{

		}

		Object2D(double x, double y, double _radius) : center(x, y), radius(_radius)
		{

		}

		const Point2D & getCenter() const
		{
			return center;
		}

		double getRadius() const
		{
			return radius;
		}

		void setCenter(double x, double y)
		{
			center.setPosition(x, y);
		}

		void setCenter(const Point2D & point)
		{
			center.setPosition(point);
		}


		bool operator ==(const Object2D &other) const
		{
			return center == other.center && radius == other.radius;
		}

		bool isCollision(const Object2D & other) const
		{
			double distance = center.getDistanceTo(other.center);
			return distance <= (radius + other.radius);
		}

		bool isCollision(const model::CircularUnit & other) const
		{
			double distance = center.getDistanceTo(other);
			return distance <= (radius + other.getRadius());
		}

		static bool isCollision(const model::CircularUnit & some, const model::CircularUnit & other)
		{
			double distance = Point2D::getCentersDistance(some, other);
			return distance <= (some.getRadius() + other.getRadius());
		}

	};
	

}

#endif
