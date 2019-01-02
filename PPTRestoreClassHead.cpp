
#include "PPTRestoreClassHead.h"

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

	template<class T, class U>
	friend ostream& operator<< (ostream& out, const pair<T, U>& p);
	friend ostream& operator<< (ostream& out, const Vec4i& v);

	void show_img(const string& windowName, const Mat& src, bool show = true)
	{
		if (PLATFORM == "WIN32")
		if (show) imshow(windowName, src);
	}
};

template<class T, class U>
ostream& operator<< (ostream& out, const pair<T, U>& p)
{
	out << "(" << p.first << "," << p.second << ")";
	return out;
}

ostream& operator<< (ostream& out, const Vec4i& v)
{
	out << "start point: (" << v[0] << "," << v[1] << ") " << "end point: (" << v[2] << "," << v[3] << ")";
	return out;
}

class Extreme_Img_Helper
{
public:
	Mat& cut(Mat&);
	vector<Point2f> deal(const vector<Point2f>& line_nodes, const vector<Point2f>& corner_nodes);
private:
	enum class state;
};



struct PPTRestore::Ximpl
{
	Mat srcImage;

	void loadImage(const string& name);//¼ÓÔØÍ¼Ïñ
	Mat preprocess_image(Mat&);
	vector<Point2f> corner_dectection(Mat&);
	vector<Vec4i> edge_detection(Mat&);

	map<double, Vec4i> find_cross_points_by_edges(const vector<Vec4i>& lines);	//¸ù¾ÝÖ±ÏßÌáÈ¡È·¶¨¾ØÐÎ4¸ö¶¥µã
	vector<Point2f> edge_corner_candidates(const map<double, Vec4i>&, const vector<Point2f>&); //compute nodes from table.
	vector<Point2f> cal_final_points(const vector<Point2f>& line_nodes, const vector<Point2f>& corner_nodes);
	vector<vector<Point2f>> divide_points_into_4_parts(const vector<Point2f>& nodes);
	Point2f& find_closest_points(const vector<Point2f>& line_nodes, const vector<Point2f>& corner_nodes);
	

	Mat perspective_transformation(const vector<Point2f>&, Mat&);//Í¸ÊÓ±ä»»
	Mat image_enhance(Mat&);//Í¼Ç¿ÔöÇ¿
	Debug* debug;
	Extreme_Img_Helper* helper;
};

PPTRestore::PPTRestore() : pImpl(new Ximpl()) {}

PPTRestore::PPTRestore(const PPTRestore& other) : pImpl(new Ximpl(*other.pImpl)) {}

PPTRestore& PPTRestore::operator=(PPTRestore other)
{
	std::swap(other.pImpl, this->pImpl);
	return *this;
}

PPTRestore::~PPTRestore()
{
	delete pImpl;
	pImpl = nullptr;
}


void PPTRestore::Ximpl::loadImage(const string& name)
{
	srcImage = imread(name, 1);
	if (!srcImage.data)
	{
		cout << "¶ÁÈ¡Í¼Æ¬´íÎó£¬ÇëÈ·¶¨Ä¿Â¼ÏÂÊÇ·ñÓÐimreadº¯ÊýÖ¸¶¨µÄÍ¼Æ¬´æÔÚ" << endl;
	}
	debug->show_img(WINDOW_NAME1, this->srcImage);
}

Mat PPTRestore::Ximpl::preprocess_image(Mat& src)
{
	Mat src_gray, after_gaus, res;
	string source_window = "¸ßË¹Ä£ºý-ÅòÕÍ";

	cvtColor(src, src_gray, CV_BGR2GRAY);
	GaussianBlur(src_gray, after_gaus, Size(9, 9), 0, 0);
	auto kernel = getStructuringElement(MORPH_RECT, Size(25, 25));
	dilate(after_gaus, res, kernel);
	debug->show_img(source_window, res);

	return res;
}

vector<Point2f> PPTRestore::Ximpl::corner_dectection(Mat& src)
{
	//³õÊ¼»¯ Shi-Tomasi algorithmµÄÒ»Ð©²ÎÊý
	vector<Point2f> corners;
	double qualityLevel = 0.01;
	double minDistance = 10;
	int blockSize = 3;
	bool useHarrisDetector = false;
	double k = 0.04;

	int maxCorners = 23;
	int maxTrackbar = 100;
	RNG rng(12345);
	string source_window = "½Çµã¼ì²é";
	// ½Çµã¼ì²â
	goodFeaturesToTrack(src, corners, maxCorners, qualityLevel, minDistance, Mat(), blockSize, useHarrisDetector, k);

	//»­³ö¼ì²âµ½µÄ½Çµã

	Mat tmp = src.clone();
	int r = 4;
	for (int i = 0; i < corners.size(); i++)
	{
		circle(tmp, corners[i], r, Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
			rng.uniform(0, 255)), -1, 8, 0);
	}

	debug->show_img(source_window, tmp);

	return corners;
}

vector<Vec4i> PPTRestore::Ximpl::edge_detection(Mat& src)
{
	vector<Vec4i> lines;
	int cannyThreshold = 180;
	float factor = 2.5;
	const int maxLinesNum = 10;//×î¶à¼ì²â³öµÄÖ±ÏßÌõÊý
	Mat mid, edgeDetect;
	Canny(src, mid, cannyThreshold, cannyThreshold * factor);
	threshold(mid, mid, 128, 255, THRESH_BINARY);
	cvtColor(mid, edgeDetect, CV_GRAY2RGB);
	HoughLinesP(mid, lines, 1, CV_PI / 180, 50, 100, 100);

	while (lines.size() >= 30)
	{
		cannyThreshold += 2;
		Canny(src, mid, cannyThreshold, cannyThreshold * factor);
		threshold(mid, mid, 128, 255, THRESH_BINARY);
		cvtColor(mid, edgeDetect, CV_GRAY2RGB);
		HoughLinesP(mid, lines, 1, CV_PI / 180, 50, 100, 100);
	}


	Canny(src, mid, cannyThreshold, cannyThreshold * factor);
	threshold(mid, mid, 128, 255, THRESH_BINARY);
	cvtColor(mid, edgeDetect, CV_GRAY2RGB);
	HoughLinesP(mid, lines, 1, CV_PI / 180, 50, 100, 100);

	const int imageRow = mid.rows;
	const int imageCol = mid.cols;

	struct IsCloseToEdge
	{
		bool operator()(Vec4i line)
		{
			return abs(line[0] - line[2]) < 10 || abs(line[1] - line[3]) < 10;
		}
	};
	lines.erase(remove_if(lines.begin(), lines.end(), IsCloseToEdge()), lines.end());

	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(edgeDetect, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(186, 88, 255), 1, CV_AA);
	}

	debug->show_img("¡¾±ßÔµÌáÈ¡Ð§¹ûÍ¼¡¿", edgeDetect);
	return lines;
}

map<double, Vec4i> PPTRestore::Ximpl::find_cross_points_by_edges(const vector<Vec4i>& lines)//Í¨¹ýÖ±ÏßÕÒµã
{
	Point2f left_top, right_top, left_down, right_down;
	map<double, Vec4i> table_vec;
	vector<double> ratios;
	for (auto line : lines)
	{
		int start_x = line[0];
		int start_y = line[1];
		int end_x = line[2];
		int end_y = line[3];
		double ratio = (double)(end_y - start_y) / (double)(end_x - start_x);

		table_vec[ratio] = line;
	}

	return table_vec;
}

vector<Point2f> PPTRestore::Ximpl::edge_corner_candidates(const map<double, Vec4i>& ratio_2points, const vector<Point2f>& corners)
{
	debug->print(ratio_2points);
	vector<double> intercepts;
	vector<double> ratios;
	vector<Point> points;
	vector<Point2f> corner_nodes;

	Types<float>::ascend_distance nodes_to_lines;
	for (auto p : ratio_2points)
	{
		ratios.push_back(p.first);
		intercepts.push_back((p.second)[1] - (p.first * (p.second)[0]));
	}

	for (int i = 0; i < corners.size(); ++i)
	{
		double min_dis = INT_MAX;
		for (int j = 0; j < ratio_2points.size(); ++j)
		{
			double dis = (double)abs(ratios[j] * corners[i].x - corners[i].y + intercepts[j]) / (double)sqrt(pow(ratios[j], 2) + 1);
			if (dis < min_dis)
				min_dis = dis;
		}
		nodes_to_lines.push({ corners[i], min_dis });
	}

	int num = 100;
	debug->print(nodes_to_lines);

	while (!nodes_to_lines.empty() && num > 0)
	{
		auto p = nodes_to_lines.top().first;
		corner_nodes.emplace_back(Point2f(p.x, p.y));
		nodes_to_lines.pop();
		num--;
	}

	Mat to_show = srcImage;
	Mat show_corner_Mat = to_show.clone();
	RNG rng(12345);
	int r = 4;
	for (int i = 0; i < corner_nodes.size(); i++)
	{
		circle(show_corner_Mat, corners[i], r, Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
			rng.uniform(0, 255)), -1, 8, 0);
	}
	string win = "4_nodes";
	debug->show_img(win, show_corner_Mat);

	//transform the nodes in the ratio_2points int vector<Point2f>
	vector<Point2f> line_nodes;
	for (auto p : ratio_2points)
	{
		line_nodes.emplace_back(Point2f((float)p.second[0], (float)p.second[1]));
		line_nodes.emplace_back(Point2f((float)p.second[2], (float)p.second[3]));
	}


	for (int i = 0; i < line_nodes.size(); i++)
	{
		circle(show_corner_Mat, line_nodes[i], r, Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
			rng.uniform(0, 255)), -1, 8, 0);
	}
	debug->show_img(win, show_corner_Mat);
	return cal_final_points(line_nodes, corner_nodes);
}

vector<Point2f> PPTRestore::Ximpl::cal_final_points(const vector<Point2f>& line_nodes, const vector<Point2f>& corner_nodes)
{
	vector<vector<Point2f>> line_temp = divide_points_into_4_parts(line_nodes);
	vector<vector<Point2f>> corner_temp = divide_points_into_4_parts(corner_nodes);

	vector<Point2f> left_top_line_nodes = line_temp[0], left_down_line_nodes = line_temp[1], right_top_line_nodes = line_temp[2], right_down_line_nodes = line_temp[3], res;
	vector<Point2f> left_top_corner_nodes = corner_temp[0], left_down_corner_nodes = corner_temp[1], right_top_corner_nodes = corner_temp[2], right_down_corner_nodes = corner_temp[3];

	res.emplace_back(find_closest_points(left_top_line_nodes, left_top_corner_nodes));
	res.emplace_back(find_closest_points(right_top_line_nodes, right_top_corner_nodes));
	res.emplace_back(find_closest_points(left_down_line_nodes, left_down_corner_nodes));
	res.emplace_back(find_closest_points(right_down_line_nodes, right_down_corner_nodes));

	return res;
}

Point2f& PPTRestore::Ximpl::find_closest_points(const vector<Point2f>& line_nodes, const vector<Point2f>& corner_nodes)
{
	if (line_nodes.size() == 1) return const_cast<Point2f&>(line_nodes[0]);
	double min_dis = DBL_MAX;
	int position = 0;
	for (int i = 0; i < line_nodes.size(); ++i)
	{
		for (int j = 0; j < corner_nodes.size(); ++j)
		{
			if (pow(line_nodes[i].x - corner_nodes[j].x, 2) + pow(line_nodes[i].y - corner_nodes[j].y, 2) < min_dis)
			{
				min_dis = pow(line_nodes[i].x - corner_nodes[j].x, 2) + pow(line_nodes[i].y - corner_nodes[j].y, 2);
				position = i;
			}
		}
	}
	return const_cast<Point2f&>(line_nodes[position]);
}

vector<vector<Point2f>> PPTRestore::Ximpl::divide_points_into_4_parts(const vector<Point2f>& line_nodes)
{
	vector<Point2f> left_top_line_nodes, left_down_line_nodes, right_top_line_nodes, right_down_line_nodes;
	vector<vector<Point2f>> res;
	int height = this->srcImage.cols / 2, width = this->srcImage.rows / 2;
	for (auto node : line_nodes)
	{
		if (node.x < height)
		{
			if (node.y < width)
				left_top_line_nodes.emplace_back(node);
			else
				left_down_line_nodes.emplace_back(node);
		}
		else
		{
			if (node.y < width)
				right_top_line_nodes.emplace_back(node);
			else
				right_down_line_nodes.emplace_back(node);
		}
	}
	res.emplace_back(left_top_line_nodes);
	res.emplace_back(left_down_line_nodes);
	res.emplace_back(right_top_line_nodes);
	res.emplace_back(right_down_line_nodes);
	return res;
}

Mat PPTRestore::Ximpl::perspective_transformation(const vector<Point2f>& final_points, Mat& src)
{
	debug->print(final_points);
	Point2f _srcTriangle[4];
	Point2f _dstTriangle[4];
	vector<Point2f>srcTriangle(_srcTriangle, _srcTriangle + 4);
	vector<Point2f>dstTriangle(_dstTriangle, _dstTriangle + 4);
	Mat after_transform;

	const int leftTopX = final_points[0].x;
	const int leftTopY = final_points[0].y;
	const int rightTopX = final_points[1].x;
	const int rightTopY = final_points[1].y;
	const int leftDownX = final_points[2].x;
	const int leftDownY = final_points[2].y;
	const int rightDownX = final_points[3].x;
	const int rightDownY = final_points[3].y;

	int newWidth = 0;
	int newHeight = 0;

	newWidth = sqrt((leftTopX - rightTopX) * (leftTopX - rightTopX) + (leftTopY - rightTopY) * (leftTopY - rightTopY));
	newHeight = sqrt((leftTopX - leftDownX) * (leftTopX - leftDownX) + (leftTopY - leftDownY) * (leftTopY - leftDownY));

	after_transform = Mat::zeros(newHeight, newWidth, src.type());

	srcTriangle[0] = Point2f(leftTopX, leftTopY);
	srcTriangle[1] = Point2f(rightTopX, rightTopY);
	srcTriangle[2] = Point2f(leftDownX, leftDownY);
	srcTriangle[3] = Point2f(rightDownX, rightDownY);

	dstTriangle[0] = Point2f(0, 0);
	dstTriangle[1] = Point2f(newWidth, 0);
	dstTriangle[2] = Point2f(0, newHeight);
	dstTriangle[3] = Point2f(newWidth, newHeight);


	Mat m1 = Mat(srcTriangle);
	Mat m2 = Mat(dstTriangle);
	Mat status;
	Mat h = findHomography(m1, m2, status, 0, 3);
	perspectiveTransform(srcTriangle, dstTriangle, h);
	warpPerspective(src, after_transform, h, after_transform.size());
	debug->show_img(WINDOW_NAME2, after_transform);
	return after_transform;
}

Mat PPTRestore::Ximpl::image_enhance(Mat& input)
{
	Mat output;
	Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
	filter2D(input, output, this->srcImage.depth(), kernel);
	return output;
}

vector<Point2f> PPTRestore::get_points(Mat& image)
{
	this->pImpl->srcImage = image;
	auto after_preprocess = this->pImpl->preprocess_image(this->pImpl->srcImage);
	auto corners = this->pImpl->corner_dectection(after_preprocess);
	auto lines = this->pImpl->edge_detection(this->pImpl->srcImage);
	auto points_with_ratio = this->pImpl->find_cross_points_by_edges(lines);
	auto final_points = this->pImpl->edge_corner_candidates(points_with_ratio, corners);
	get_image(image, final_points);
	return final_points;
}

Mat PPTRestore::get_image(Mat& image, const vector<Point2f>& points)
{
	auto after_transform = this->pImpl->perspective_transformation(points, image);
	auto final_mat = this->pImpl->image_enhance(after_transform);
	return final_mat;
}

