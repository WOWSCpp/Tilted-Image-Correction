#include "Img_PreProcess.h"

pair<double, double> PreProcess::auto_canny(Mat Input)
{
	//Input = Input.reshape(0, 1); // spread Input Mat to single row
	//vector<double> vecFromMat;
	//Input.copyTo(vecFromMat); // Copy Input Mat to vector vecFromMat
	//nth_element(vecFromMat.begin(), vecFromMat.begin() + vecFromMat.size() / 2, vecFromMat.end());
	//double v = vecFromMat[vecFromMat.size() / 2];
	//double sigma = 0.333;
	//int lower = int(max(0, int((1.0 - sigma) * v)));
	//int upper = int(min(255, int((1.0 + sigma) * v)));
	//return{ lower, upper };
	return{ 10, 200 };
}

vector<vector<Point2i>> PreProcess::m_find_contours(Mat img)
{
	Mat gray, edges;
	cvtColor(img, gray, COLOR_BGR2GRAY);
	pair<double, double> p = auto_canny(gray);
	double lower = p.first, upper = p.second;
	cout << lower << " " << upper << endl;
	Canny(gray, edges, lower, upper);
	vector<vector<Point2i>> contours;
	findContours(edges.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	contours.erase(remove_if(contours.begin(), contours.end(), [](vector<Point2i> p) {return p.size() < 1000; }), contours.end());
	return contours;
}

vector<vector<Point2i>> PreProcess::m_find_biggest_contours(vector<vector<Point2i>>& contours)
{
	vector<vector<Point2i>> biggest_contours;
	sort(contours.begin(), contours.end(), [](vector<Point2i> c1, vector<Point2i> c2) {return c1.size() > c2.size(); });

	if (contours.size() >= 1) biggest_contours.emplace_back(contours[0]);
	if (contours.size() >= 2) biggest_contours.emplace_back(contours[1]);
	return biggest_contours;
}

vector<RotatedRect> PreProcess::m_find_minRect(vector<vector<Point2i>>& biggest_contours)
{
	vector<RotatedRect> minRect(biggest_contours.size());
	for (int i = 0; i < biggest_contours.size(); i++)
		minRect[i] = minAreaRect(Mat(biggest_contours[i]));
	return minRect;
}

vector<vector<Point2i>> PreProcess::m_find_centerRect_contours(vector<vector<Point2i>>& biggest_contours, vector<RotatedRect>& minRect, Mat& img)
{
	unordered_map<string, int> table;
	for (int i = 0; i < biggest_contours.size(); ++i)
		table[to_string(int(minRect[i].center.x)) + to_string(int(minRect[i].center.y))] = i;
	sort(minRect.begin(), minRect.end(), [&](RotatedRect r1, RotatedRect r2) {
		return abs(r1.center.x - img.cols / 2) + abs(r1.center.y - img.rows / 2) <
			abs(r2.center.x - img.cols / 2) + abs(r2.center.y - img.rows / 2);
	});
	auto centerRect = minRect[0];
	vector<vector<Point2i>> centerRectContours;
	centerRectContours.emplace_back(biggest_contours[table[to_string(int(centerRect.center.x)) + to_string(int(centerRect.center.y))]]);
	return centerRectContours;
}

vector<Point2f> PreProcess::get_hull_points(vector<vector<Point2i>>& contours, Mat img)
{
	vector<Point2f> hullPoints;
	vector<vector<Point2i>> polyContours(contours.size());
	int maxArea = 0;
	for (int index = 0; index < contours.size(); index++)
	{
		if (contourArea(contours[index]) > contourArea(contours[maxArea]))
			maxArea = index;
		approxPolyDP(contours[index], polyContours[index], 10, true);
	}
	if (polyContours.empty()) return {};
	Mat polyPic = Mat::zeros(img.size(), CV_8UC3);
	drawContours(polyPic, polyContours, maxArea, Scalar(0, 0, 255), 2);

	vector<int> hull;
	convexHull(polyContours[maxArea], hull, false);    //¼ì²â¸ÃÂÖÀªµÄÍ¹°ü

	for (int i = 0; i < hull.size(); ++i)
	{
		circle(polyPic, polyContours[maxArea][i], 10, Scalar(255, 0, 0), 3);
		hullPoints.emplace_back(polyContours[maxArea][i]);
	}
	return hullPoints;
}

void PreProcess::render_to_white(Mat& src)
{
	for (int i = 0; i < src.rows; ++i)
	{
		for (int j = 0; j < src.cols; ++j)
		{
			src.at<Vec3b>(i, j)[0] = 255;
			src.at<Vec3b>(i, j)[1] = 255;
			src.at<Vec3b>(i, j)[2] = 255;
		}
	}
}

void PreProcess::render_center_rect_contours(Mat& img, const vector<vector<Point2i>>& centerRectContours)
{
	for (int i = 0; i < centerRectContours.size(); ++i)
	{
		for (int j = 0; j < centerRectContours[i].size(); ++j)
		{
			img.at<Vec3b>(centerRectContours[i][j].y, centerRectContours[i][j].x)[0] = 0;
			img.at<Vec3b>(centerRectContours[i][j].y, centerRectContours[i][j].x)[1] = 0;
			img.at<Vec3b>(centerRectContours[i][j].y, centerRectContours[i][j].x)[2] = 0;
		}
	}
}

void PreProcess::show_biggest_contours_and_center(vector<vector<Point2i>>& biggest_contours, vector<RotatedRect>& minRect, Mat& drawing)
{
	for (int i = 0; i < biggest_contours.size(); i++)
	{
		Scalar color = Scalar(0, 255, 0);
		drawContours(drawing, biggest_contours, i, color, 1, 8, vector<Vec4i>(), 0, Point2i());
		Point2f rect_points[4]; minRect[i].points(rect_points);
		for (int j = 0; j < 4; j++)
			line(drawing, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
	}

	for (int i = 0; i < biggest_contours.size(); i++)
	{
		Scalar color = Scalar(255, 0, 0);
		drawContours(drawing, biggest_contours, -1, Scalar(255), 2);
		circle(drawing, minRect[i].center, 4, color, -1, 8, 0);
	}
	//imshow("Contours", drawing);
}