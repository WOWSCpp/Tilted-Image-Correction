#ifndef __PROCESS_H
#define __PROCESS_H


#include "Img_PreProcess.h"

class ImageProcess
{
public:
	ImageProcess();
	ImageProcess(const ImageProcess&);
	ImageProcess(ImageProcess&&);
	ImageProcess& operator=(ImageProcess other);
	~ImageProcess();
	void public_api(const string& name);
	vector<Point2f> get_points(Mat&);
	Mat get_image(Mat& image, vector<Point2f>& points);
private:
	struct Ximpl;
	Ximpl* pImpl;
};

#endif


