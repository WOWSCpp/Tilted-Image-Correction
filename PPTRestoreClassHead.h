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
#include <queue>
#include <unordered_map>
#include <numeric>
#include <map>
using namespace cv;
using namespace std;
#define WINDOW_NAME1 "raw"			 
#define WINDOW_NAME2 "after wrap"             
#define PLATFORM  "WIN32"



class PPTRestore
{
public:
	PPTRestore();
	PPTRestore(const PPTRestore&);
	PPTRestore(PPTRestore&&);
	PPTRestore& operator=(PPTRestore other);
	~PPTRestore();
	void public_api(const string& name);
	vector<Point2f> get_points(Mat&);
	Mat get_image(Mat& image, const vector<Point2f>& points);
	unordered_map<string, Mat> get_all_images() const { return tempImg; }

private:
	struct Ximpl;
	Ximpl* pImpl;
	static unordered_map<string, Mat> tempImg;
};


#endif


