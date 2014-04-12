#include <limits>
#include <iostream>
#include <cmath>
#include <vector>
#include <stack>
#include <algorithm>
#include <map>
#include <queue>

namespace geometry {

    struct Point {
        long double x;
        long double y;
        long double z;
        Point() {
            x = y = z = std::numeric_limits< long double >::quiet_NaN();
        }
        Point(long double x, long double y, long double z): x(x), y(y), z(z) {}

        Point operator+(const Point &point) const {
            return Point(x + point.x, y + point.y, z + point.z);
        }
        Point operator-(const Point &point) const {
            return Point(x - point.x, y - point.y, z - point.z);
        }
        long double operator*(const Point &point) const {
            return x * point.x + y * point.y + z * point.z;
        }
        Point operator^(const Point &point) const {
            return Point(y * point.z - z * point.y, z * point.x - x * point.z, x * point.y - y * point.x);
        }

        Point operator*(long double value) const {
            return Point(x * value, y * value, z * value);
        }
        Point operator/(long double value) const {
            return Point(x / value, y / value, z / value);
        }
        Point operator+(long double value) const {
            return Point(x + value, y + value, z + value);
        }
        Point operator-(long double value) const {
            return Point(x - value, y - value, z - value);
        }

        bool operator==(const Point &point) const {
            return x == point.x && y == point.y && z == point.z;
        }
        bool operator!=(const Point &point) const {
            return !(point == *this);
        }
        bool operator<(const Point &point) const {
            return x < point.x || (x == point.x && y < point.y) 
                || (x == point.x && y == point.y && z < point.z);
        }

        long double sqrLength() {
            return (*this) * (*this);
        }
        long double length() {
            return sqrtl(sqrLength());
        }
        
        friend std::istream &operator>>(std::istream &in, Point &point);
        friend std::ostream &operator<<(std::ostream &out, const Point &point);
    };

    std::istream &operator>>(std::istream &in, Point &point) {
        return in >> point.x >> point.y >> point.z;
    }
    std::ostream &operator<<(std::ostream &out, const Point &point) {
        return out << point.x << ' ' << point.y << ' ' << point.z;
    }

    
    struct Segment {
        Point a;
        Point b;
        Segment(const Point &a, const Point &b): a(a), b(b) {}
        long double sqrLength() {
            return (a - b).sqrLength();
        }
        long double length() {
            return (a - b).length();
        }
    };

    struct Line {
        Point point;
        Point direction;
        Line(const Point &first, const Point &second) {
            point = first;
            direction = (second - first) / Segment(first, second).length();
        }
    };

    // ��������������� ���������. ������� ������� ����������� �� ������� ���������
    struct Plane {
        Point point;
        Point firstDirection;
        Point secondDirection;
        // ��� ��� �������� ������� ���������
        Plane(const Point &a, const Point &b, const Point &c) {
            point = a;
            firstDirection = b - a;
            secondDirection = c - a;
        }
        Point normal() const {
            return firstDirection ^ secondDirection;
        }
        //���������� � ������ ������� �������
        long double distanse(const Point &p) {
            Point norm = normal();
            return (norm * (p - point)) / (norm * norm); 
        }
        Point projection(const Point &p) {
            return p - normal() * distanse(p);
        }
    };

    struct SetOfPoints {
        std::vector< Point > points;
    private:
        //������� ������������ ������� (first, second) � �������� normal ����������� � �������� �������� � ������ � ���� ���������
        //����������������� ������� ����� ���� �������� � ������� opened ��� ���������� ������ ������� ���������
        void addSegment(std::map< std::pair< size_t, size_t >, Point > &segments,
                        std::queue< std::pair< size_t, size_t > > &opened,
                        size_t first, size_t second, const Point &normal) {
            segments[std::make_pair(first, second)] = Point(0, 0, 0);
            std::pair < size_t, size_t > segment(second, first);
            if (segments.find(segment) == segments.end()) {
                segments[segment] = normal;
                opened.push(segment);
            }
        }

        //������� ��������������� ��������� (first, scond, bottom) � ������
        //������� �������������� ������ ������� ��� �����������
        std::vector< size_t > addPlane(std::map< std::pair< size_t, size_t >, Point > &segments,
                                       std::queue< std::pair< size_t, size_t > > &opened,
                                       size_t bottom, size_t first, size_t second) {
            Point normal = Plane(points[bottom], points[first], points[second]).normal();
            addSegment(segments, opened, bottom, first, normal);
            addSegment(segments, opened, first, second, normal);
            addSegment(segments, opened, second, bottom, normal);
            std::vector< size_t > plane;
            plane.push_back(bottom);
            plane.push_back(first);
            plane.push_back(second);
            return plane;
        }

        //������ �����-������ ��������� � �������� ��������
        std::vector< size_t > initConvexHull(std::map< std::pair< size_t, size_t >, Point > &segments,
                                             std::queue< std::pair< size_t, size_t > > &opened) {
            //������ ����� ������ � �������� ��������
            size_t bottom = 0;
            for (size_t i = 1; i < points.size(); i++)
                if (points[i].z < points[bottom].z)
                    bottom = i;
            //��������� ����� � ������������ ����� � Oz
            size_t second = bottom;
            long double maxSqrSin = 0;
            for (size_t i = 0; i < points.size(); i++) {
                if (i != bottom) {
                    Point vect = points[i] - points[bottom];
                    //����� ��������� sin^2 ��� �����. ���� ������ ���� �� 0 �� pi/2
                    long double sqrSin = (vect ^ Point(0, 0, 1)).sqrLength() / vect.sqrLength();
                    if (sqrSin > maxSqrSin) {
                        second = i;
                        maxSqrSin = sqrSin;
                    }
                }
            }
            size_t first;
            //����� �� n*n ������� ���������� 3-� �����
            for (size_t i = 0; i < points.size(); i++) 
                if (i != bottom && i != second) {
                    Plane plane(points[bottom], points[i], points[second]);
                    bool onHull = true;
                    for (size_t j = 0; j < points.size() && onHull; j++)
                        //�����, ����� ����� �� ������ ���������� �� 1 ������� �� ���������, �� � ���������� ������� ��������
                        if (i != j && second != j && bottom != j && plane.distanse(points[j]) > 0)
                            onHull = false;
                    if (onHull) {
                        first = i;
                        break;
                    }
                }
            return addPlane(segments, opened, bottom, first, second);
        }
    public:
        void convexHull(std::vector< std::vector< size_t > > &planes) {
            planes.clear();
            if (points.size() < 3)
                return;
            // ��� ������� ������� �� �������� �������� ������ ������ ������� ��� ������ � ��� ���������
            // ���������� ������ 1 ������ ��������� �� ���� ����������������� � ���������, � �������
            // ���� ��� ������� ��������� ���������, ������ ������� ������
            std::map< std::pair< size_t, size_t >, Point > segments;
            // ������� � ��������� �� �������� ��������, ��� ������� ��������� �� ������� ���� 1 ������� ���������
            std::queue< std::pair< size_t, size_t > > opened;
            planes.push_back(initConvexHull(segments, opened));
            while (!opened.empty()) {
                std::pair< size_t, size_t > segment = opened.front();
                opened.pop();
                Point normal = segments[segment];
                if (normal == Point(0, 0, 0))
                    continue;
                //����� ��� ���� �� 0 �� pi � ����� ����� ������� � ���������� �����
                //�����, �� �������� �����, ��������� �������� cos * fabs(cos)
                long double maxSqrCos = -2;
                size_t newPoint;
                for (size_t i = 0; i < points.size(); i++)
                    if (i != segment.first && i != segment.second) {
                        Point nextNormal = Plane(points[segment.first], points[segment.second], points[i]).normal();
                        long double sqrCos = (normal * nextNormal) * fabs(normal * nextNormal) 
                            / normal.sqrLength() / nextNormal.sqrLength();
                        if (sqrCos > maxSqrCos) {
                            maxSqrCos = sqrCos;
                            newPoint = i;
                        }
                    }
                planes.push_back(addPlane(segments, opened, segment.first, segment.second, newPoint));
            }
        }
    };
};