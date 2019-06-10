#ifndef __UTILITY_H
#define __UTILITY_H
#include <cmath>
#include <iostream>
#include <fstream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <numeric>
#include <map>
#define PLATFORM  "WIN32"
using namespace cv;
using namespace std;

template<class T>
class Types
{
public:
	using paii = pair<Point2f, T>;
	struct AscendingCmp
	{
		bool operator()(paii p1, paii p2)
		{
			return p1.second > p2.second;
		}
	};
	using ascend_distance = priority_queue<paii, vector<paii>, AscendingCmp>;
};

class Utility
{
public:
	double angle(Point2f pt1, Point2f pt2, Point2f pt0);
	double pointDistance(const Point2f& p1, const Point2f& p2);
	double pointDistance(const Vec4f& line);
	static bool is_similar_line(const Vec4i& _l1, const Vec4i& _l2);
};

template<class T, class U>
ostream& operator<< (ostream& out, const pair<T, U>& p);
ostream& operator<< (ostream& out, const Vec4f& v);

class Debug
{
public:
	template<template<class, class...> class ContainerType, class ValueType, class... Args>
	void print(const ContainerType<ValueType, Args...>& c)
	{
		if (PLATFORM == "WIN32")
			for (const auto& v : c)
			{
				cout << v << endl;
			}
	}

	template<template<class, class, class> class ContainerType, class ValueType, class Cmp>
	void print(ContainerType<ValueType, vector<ValueType>, Cmp> q)
	{
		if (PLATFORM == "WIN32")
			while (!q.empty())
			{
				auto p = q.top();
				cout << p << endl;
				q.pop();
			}
	}

	void show_img(const string& windowName, const Mat& src, bool show = true)
	{
		if (PLATFORM == "WIN32")
			if (show) imshow(windowName, src);
	}
private:
	template<class T, class U>
	friend ostream& operator<< (ostream& out, const pair<T, U>& p);
	friend ostream& operator<< (ostream& out, const Vec4f& v);
};


template<class T, class U>
inline ostream& operator<< (ostream& out, const pair<T, U>& p)
{
	out << "(" << p.first << "," << p.second << ")";
	return out;
}

inline ostream& operator<< (ostream& out , const Vec4f& v)
{
	out << "start Point2f: (" << v[0] << "," << v[1] << ") " << "end Point2f: (" << v[2] << "," << v[3] << ")";
	return out;
}

#endif // !__UTILITY_H

