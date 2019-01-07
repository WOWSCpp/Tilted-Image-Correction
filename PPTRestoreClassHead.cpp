
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
	friend ostream& operator<< (ostream& out, const Vec4f& v);

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

ostream& operator<< (ostream& out, const Vec4f& v)
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

unordered_map<string, Mat> PPTRestore::tempImg;

struct PPTRestore::Ximpl
{
	Mat srcImage;
	Mat afterCanny;

	Mat preprocess_image(Mat&);
	vector<Point2f> corner_dectection(Mat&);
	vector<Vec4f> edge_detection(Mat&);

	map<float, Vec4f> find_cross_points_by_edges(const vector<Vec4f>& lines);	
	vector<Point2f> edge_corner_candidates(const map<float, Vec4f>&, const vector<Point2f>&); 
	vector<Point2f> cal_final_points(const vector<Point2f>& line_nodes, const vector<Point2f>& corner_nodes);
	vector<vector<Point2f>> divide_points_into_4_parts(const vector<Point2f>& nodes);
	Point2f& find_closest_points(const vector<Point2f>& line_nodes, const vector<Point2f>& corner_nodes);
	vector<Point2f> cal_points_with_lines(const vector<Vec4f>&);
	Point2f line_intersection(const Point2f& o1, const Point2f& p1, const Point2f& o2, const Point2f& p2);
	void test(Mat);
	Mat perspective_transformation(const vector<Point2f>&, Mat&);
	Mat image_enhance(Mat&);
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


Mat PPTRestore::Ximpl::preprocess_image(Mat& src)
{
	Mat src_gray, after_gaus, res;
	string source_window = "pre_process";

	cvtColor(src, src_gray, CV_BGR2GRAY);
	GaussianBlur(src_gray, after_gaus, Size(9, 9), 0, 0);
	auto kernel = getStructuringElement(MORPH_RECT, Size(25, 25));
	dilate(after_gaus, res, kernel);
	debug->show_img(source_window, res);
	PPTRestore::tempImg["pre_process"] = res;
	return res;
}

vector<Point2f> PPTRestore::Ximpl::corner_dectection(Mat& src)
{
	vector<Point2f> corners;
	float qualityLevel = 0.01;
	float minDistance = 10;
	int blockSize = 3;
	bool useHarrisDetector = false;
	float k = 0.04;

	int maxCorners = 23;
	int maxTrackbar = 100;
	RNG rng(12345);
	string source_window = "corner";
	goodFeaturesToTrack(src, corners, maxCorners, qualityLevel, minDistance, Mat(), blockSize, useHarrisDetector, k);

	Mat tmp = src.clone();
	int r = 4;
	for (int i = 0; i < corners.size(); i++)
	{
		circle(tmp, corners[i], r, Scalar(255, 0, 0), -1, 8, 0);
	}

	debug->show_img(source_window, tmp);
	PPTRestore::tempImg["corner"] = tmp;
	return corners;
}

void PPTRestore::Ximpl::test(Mat src)
{
	Mat dst = afterCanny;
	const float approachMaxThreshold = 20;
	vector<vector<Point>> contours;
	findContours(dst, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	Mat result = Mat::zeros(dst.size(), CV_8UC1);
	if (!contours.empty())
	{
		//result:存放轮廓，contours：找到的轮廓，-1：将所有轮廓画出，Scalar(255)：白色画笔，2：为画笔粗细
		drawContours(result, contours, -1, Scalar(255), 2);
		imshow("处理图", result);
	}

	cout << contours.size() << endl;
	cout << contours[0].size() << endl;
	
}

bool is_similar_line(const Vec4i& _l1, const Vec4i& _l2)
{
	Vec4i l1(_l1), l2(_l2);

	float length1 = sqrtf((l1[2] - l1[0])*(l1[2] - l1[0]) + (l1[3] - l1[1])*(l1[3] - l1[1]));
	float length2 = sqrtf((l2[2] - l2[0])*(l2[2] - l2[0]) + (l2[3] - l2[1])*(l2[3] - l2[1]));

	float product = (l1[2] - l1[0])*(l2[2] - l2[0]) + (l1[3] - l1[1])*(l2[3] - l2[1]);

	if (fabs(product / (length1 * length2)) < cos(CV_PI / 30))
		return false;

	float mx1 = (l1[0] + l1[2]) * 0.5f;
	float mx2 = (l2[0] + l2[2]) * 0.5f;

	float my1 = (l1[1] + l1[3]) * 0.5f;
	float my2 = (l2[1] + l2[3]) * 0.5f;
	float dist = sqrtf((mx1 - mx2)*(mx1 - mx2) + (my1 - my2)*(my1 - my2));

	if (dist > std::max(length1, length2) * 0.3f)
		return false;

	return true;
}


vector<Vec4f> PPTRestore::Ximpl::edge_detection(Mat& src)
{
	// to do
	float alpha = 2.0; /*< Simple contrast control */
	int beta = 1;       /*< Simple brightness control */
	Mat dst = src;
	Mat new_image = Mat::zeros(dst.size(), dst.type());
	for (int y = 0; y < dst.rows; y++) {
		for (int x = 0; x < dst.cols; x++) {
			for (int c = 0; c < dst.channels(); c++) {
				new_image.at<Vec3b>(y, x)[c] =
					saturate_cast<uchar>(alpha*dst.at<Vec3b>(y, x)[c] + beta);
			}
		}
	}
	src = dst;
	vector<Vec4f> lines;
	int cannyThreshold = 180;
	float factor = 2.5;
	const int maxLinesNum = 10;
	int min_line_length = 20;
	int max_line_gap = 200;
	Mat mid, edgeDetect;
	Canny(src, mid, cannyThreshold, cannyThreshold * factor);
	threshold(mid, mid, 128, 255, THRESH_BINARY);
	cvtColor(mid, edgeDetect, CV_GRAY2RGB);
	HoughLinesP(mid, lines, 1, CV_PI / 180, 80, min_line_length, max_line_gap);

	while (lines.size() >= 30)
	{
		cannyThreshold += 2;
		Canny(src, mid, cannyThreshold, cannyThreshold * factor);
		threshold(mid, mid, 128, 255, THRESH_BINARY);
		cvtColor(mid, edgeDetect, CV_GRAY2RGB);
		HoughLinesP(mid, lines, 1, CV_PI / 180, 80, min_line_length, max_line_gap);
	}


	Canny(src, mid, cannyThreshold, cannyThreshold * factor);
	threshold(mid, mid, 128, 255, THRESH_BINARY);
	cvtColor(mid, edgeDetect, CV_GRAY2RGB);
	HoughLinesP(mid, lines, 1, CV_PI / 180, 80, min_line_length, max_line_gap);
	afterCanny = mid;

	// refine lines
	struct IsCloseToEdge
	{
		bool operator()(Vec4f line)
		{
			return abs(line[0] - line[2]) < 10 || abs(line[1] - line[3]) < 10;
		}
	};
	lines.erase(remove_if(lines.begin(), lines.end(), IsCloseToEdge()), lines.end());

	std::vector<int> labels;
	int numberOfLines = cv::partition(lines, labels, is_similar_line);
	
	vector<Vec4f> final_lines;
	int max_size = *max_element(labels.begin(), labels.end());
	for (int i = 0; i < max_size + 1; ++i)
		final_lines.emplace_back(lines[find(labels.begin(), labels.end(), i) - labels.begin()]);

	for (size_t i = 0; i < final_lines.size(); i++)
	{
		Vec4i l = final_lines[i];
		line(edgeDetect, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 1, CV_AA);
	}

	debug->show_img("【边缘提取效果图】", edgeDetect);
	return final_lines;
}


Point2f PPTRestore::Ximpl::line_intersection(const Point2f& o1, const Point2f& p1, const Point2f& o2, const Point2f& p2)
{
	Point2f x = o2 - o1;
	Point2f d1 = p1 - o1;
	Point2f d2 = p2 - o2;

	float cross = d1.x*d2.y - d1.y*d2.x;
	double t1 = (x.x * d2.y - x.y * d2.x) / cross;
	return o1 + d1 * t1;
}

vector<Point2f> PPTRestore::Ximpl::cal_points_with_lines(const vector<Vec4f>& lines)
{
	cout << "size is : " << lines.size() << endl;
	//left, right, up, down
	vector<float> ratio;
	vector<float> intersects;
	for (int i = 0; i < lines.size(); ++i)
	{
		ratio.emplace_back((lines[i][3] - lines[i][1]) / (lines[i][2] - lines[i][0]));
		intersects.emplace_back(lines[i][1] - ratio[i] * lines[i][0]);
	}
	vector<Point2f> intersect_points;
	const float height = srcImage.rows; // height
	const float width = srcImage.cols; // width

	Point2f center(width / 2, height / 2);

	cout << height << endl;
	cout << width << endl;
	
	vector<Point2f> cross_points;
	int min_padding = 50;
	for (int i = 0; i < lines.size(); ++i)
	{
		for (int j = i; j < lines.size(); ++j)
		{
			if (j == i) continue;
			Point2f c = line_intersection(Point2f(lines[i][0], lines[i][1]), Point2f(lines[i][2], lines[i][3]),
				Point2f(lines[j][0], lines[j][1]), Point2f(lines[j][2], lines[j][3]));
			if (c.x < -min_padding || c.x > width + min_padding || c.y < -min_padding || c.y > height + min_padding) continue;
			cross_points.emplace_back(c);
		}
	}

	// add all line's edge point into cross_points
	for (auto line : lines)
	{
		cross_points.emplace_back(Point2f(line[0], line[1]));
		cross_points.emplace_back(Point2f(line[2], line[3]));
	}


	Mat t = srcImage.clone();
	int r = 4;
	for (int i = 0; i < cross_points.size(); i++)
	{
		circle(t, cross_points[i], r, Scalar(0, 255, 0), -1, 8, 0);
	}
	imshow("aaaaaaaaaaa", t);

	vector<vector<Point2f>> _4_parts = divide_points_into_4_parts(cross_points);
	vector<Point2f> a = _4_parts[0];
	vector<Point2f> b = _4_parts[1];
	vector<Point2f> c = _4_parts[2];
	vector<Point2f> d = _4_parts[3];

	sort(a.begin(), a.end(), [&](Point2f p1, Point2f p2) {
		return pow(center.y - p1.y, 2) + pow(center.x - p1.x, 2) < pow(center.y - p2.y, 2) + pow(center.x - p2.x, 2); });
	sort(b.begin(), b.end(), [&](Point2f p1, Point2f p2) {
		return pow(center.y - p1.y, 2) + pow(center.x - p1.x, 2) < pow(center.y - p2.y, 2) + pow(center.x - p2.x, 2); });
	sort(c.begin(), c.end(), [&](Point2f p1, Point2f p2) {
		return pow(center.y - p1.y, 2) + pow(center.x - p1.x, 2) < pow(center.y - p2.y, 2) + pow(center.x - p2.x, 2); });
	sort(d.begin(), d.end(), [&](Point2f p1, Point2f p2) {
		return pow(center.y - p1.y, 2) + pow(center.x - p1.x, 2) < pow(center.y - p2.y, 2) + pow(center.x - p2.x, 2); });
	intersect_points.emplace_back(a.back());
	intersect_points.emplace_back(c.back());
	intersect_points.emplace_back(b.back());
	intersect_points.emplace_back(d.back());
	return intersect_points;
}


map<float, Vec4f> PPTRestore::Ximpl::find_cross_points_by_edges(const vector<Vec4f>& lines)
{
	Point2f left_top, right_top, left_down, right_down;
	map<float, Vec4f> table_vec;
	vector<float> ratios;
	for (auto line : lines)
	{
		int start_x = line[0];
		int start_y = line[1];
		int end_x = line[2];
		int end_y = line[3];
		float ratio = (float)(end_y - start_y) / (float)(end_x - start_x);

		table_vec[ratio] = line;
	}

	return table_vec;
}

vector<Point2f> PPTRestore::Ximpl::edge_corner_candidates(const map<float, Vec4f>& ratio_2points, const vector<Point2f>& corners)
{
	vector<float> intercepts;
	vector<float> ratios;
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
		float min_dis = INT_MAX;
		for (int j = 0; j < ratio_2points.size(); ++j)
		{
			float dis = (float)abs(ratios[j] * corners[i].x - corners[i].y + intercepts[j]) / (float)sqrt(pow(ratios[j], 2) + 1);
			if (dis < min_dis)
				min_dis = dis;
		}
		nodes_to_lines.push({ corners[i], min_dis });
	}

	int num = 100;

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
		circle(show_corner_Mat, corners[i], r, Scalar(0, 255, 0), -1, 8, 0);
	}
	PPTRestore::tempImg["together"] = show_corner_Mat;
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
	float min_dis = DBL_MAX;
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
		if (node.x < height * 1.1)
		{
			if (node.y < width * 1.1)
				left_top_line_nodes.emplace_back(node);
			if (node.y > width * 0.9)
				left_down_line_nodes.emplace_back(node);
		}
		if (node.x > height * 0.9)
		{
			if (node.y < width * 1.1)
				right_top_line_nodes.emplace_back(node);
			if (node.y > width * 0.9)
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
	PPTRestore::tempImg["final"] = output;
	return output;
}

vector<Point2f> PPTRestore::get_points(Mat& image)
{
	PPTRestore::tempImg["raw"] = image;
	this->pImpl->srcImage = image;
	auto after_preprocess = this->pImpl->preprocess_image(this->pImpl->srcImage);
	// auto corners = this->pImpl->corner_dectection(after_preprocess);
	auto lines = this->pImpl->edge_detection(this->pImpl->srcImage);
	auto final_points_new = this->pImpl->cal_points_with_lines(lines);
	auto points_with_ratio = this->pImpl->find_cross_points_by_edges(lines);
	// auto final_points = this->pImpl->edge_corner_candidates(points_with_ratio, corners);
	get_image(image, final_points_new);
	return final_points_new;
}

Mat PPTRestore::get_image(Mat& image, const vector<Point2f>& points)
{
	auto after_transform = this->pImpl->perspective_transformation(points, image);
	auto final_mat = this->pImpl->image_enhance(after_transform);
	return final_mat;
}

