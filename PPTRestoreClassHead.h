#ifndef __PPTRESTORE_H
#define __PPTRESTORE_H
#include <cmath>
#include <iostream>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/ml/ml.hpp>  
#include <memory>
using namespace cv;
using namespace std;
#define WINDOW_NAME1 "【原始图窗口】"			 
#define WINDOW_NAME2 "【经过Warp后的图像】"        
#define WINDOW_NAME3 "【再经过增强后的图像】"        
struct IsCloseToEdge
{
	bool operator()(Vec4i line)
	{
		return abs(line[0] - line[2]) < 10 || abs(line[1] - line[3]) < 10;
	}
};
struct CmpContoursSize//轮廓大小排序
{
	bool operator()(const vector<Point>& lhs, const vector<Point>& rhs) const
	{
		return lhs.size() > rhs.size();
	}
};
struct CmpDistanceToZero//各个点到原点距离
{
	bool operator()(const Point& lhs, const Point& rhs) const
	{
		return lhs.x + lhs.y < rhs.x + rhs.y;
	}
};
class PPTRestore
{
public:
	PPTRestore();
	PPTRestore(const PPTRestore&);
	PPTRestore(PPTRestore&&);
	PPTRestore& operator=(PPTRestore other);
	~PPTRestore();
	void imageRestoreAndEnhance(const string name);//图像还原和增强
private:
	struct Ximpl;
	Ximpl* pImpl;
};

#endif