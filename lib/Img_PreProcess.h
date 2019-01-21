#ifndef __PREPROCESS_H
#define __PREPROCESS_H
#include "Img_Utility.h"

class PreProcess
{
public:
	pair<double, double> auto_canny(Mat Input);
	vector<vector<Point2i>> m_find_contours(Mat img);
	vector<vector<Point2i>> m_find_biggest_contours(vector<vector<Point2i>>& c);
	vector<RotatedRect> m_find_minRect(vector<vector<Point2i>>& c);
	vector<vector<Point2i>> m_find_centerRect_contours(vector<vector<Point2i>>& r, vector<RotatedRect>&, Mat&);
	vector<Point2f> get_hull_points(vector<vector<Point2i>>& contours, Mat img);
	void render_to_white(Mat& img);
	void render_center_rect_contours(Mat& img, const vector<vector<Point2i>>& crc);
	void show_biggest_contours_and_center(vector<vector<Point2i>>& c, vector<RotatedRect>& r, Mat& drawing);
};

#endif // ! __PREPROCESS_H
