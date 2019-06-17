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
#include <tuple>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/ml/ml.hpp> 
#define PLATFORM  "WIN32"
using namespace std;

template <unsigned int N = 0, class F, class... T>
typename enable_if<N == sizeof...(T), void>::type for_each(tuple<T...>&, F&& f) {}

template <unsigned int N = 0, class F, class...T>
typename enable_if<N < sizeof...(T), void>::type for_each(tuple<T...>& t, F&& f)
{
	f(get<N>(t));
	for_each<N + 1, F, T...>(t, std::forward<F>(f));
}


template <unsigned int N = 0, class F, class...T>
typename enable_if<N == sizeof...(T), void>::type for_each(const tuple<T...>&, F&& f) {}

template <unsigned int N = 0, class F, class...T>
typename enable_if<N < sizeof...(T), void>::type for_each(const tuple<T...>& t, F&& f)
{
	f(get<N>(t));
	for_each<N + 1, F, T...>(t, std::forward<F>(f));
}


template <unsigned int N = 0, class F, class... T>
typename enable_if<N == sizeof...(T), void>::type for_each(tuple<T...>&&, F&& f) {}

template <unsigned int N = 0, class F, class... T>
typename enable_if<N < sizeof...(T), void>::type for_each(tuple<T...>&& t, F&& f)
{
	f(get<N>(t));
	for_each<N + 1, F, T...>(move(t), std::forward<F>(f));
}

template <unsigned int N, class F, class T>
struct Unpack
{
	template <class... Args>
	static decltype(auto) unpack(F&& f, T&& t, Args&&... args)
	{
		return Unpack<N - 1, F, T>::unpack(
			forward<F>(f),
			forward<T>(t),
			get<N - 1>(forward<T>(t)),
			forward<Args>(args)...);
	}
};

template <class F, class T>
struct Unpack<0, F, T>
{
	template <class... Args>
	static decltype(auto) unpack(F&& f, T&& t, Args&&... args)
	{
		return f(forward<Args>(args)...);
	}
};

template <class F, class... Args>
decltype(auto) wrapper(F&& f, tuple<Args...>& t)
{
	return Unpack<sizeof...(Args), F, const tuple<Args...>&>::unpack(forward<F>(f), t);
}

template <class F, class... Args>
decltype(auto) wrapper(F&& f, const tuple<Args...>& t)
{
	return Unpack<sizeof...(Args), F, tuple<Args...>&>::unpack(forward<F>(f), t);
}

template <class F, class... Args>
decltype(auto) wrapper(F&& f, tuple<Args...>&& t)
{
	return Unpack<sizeof...(Args), F, tuple<Args...>&&>::unpack(forward<F>(f), move(t));
}

void foo(int i, const std::string& s)
{
	cout << "int para: " << i << "\n"
		<< "str para: " << s << "\n";
}

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

