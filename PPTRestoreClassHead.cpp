
#include "PPTRestoreClassHead.h"
template<class T, class U>
ostream& operator<< (ostream& out, const pair<T, U>& p){	out << "(" << p.first << "," << p.second << ")";	return out;}

ostream& operator<< (ostream& out, const Vec4i& v){	out << "start point: (" << v[0] << "," << v[1] << ") " << "end point: (" << v[2] << "," << v[3] << ")";	return out;}

struct PPTRestore::Ximpl
{
	int cannyThreshold;
	Mat srcImage, dstImage;
	Mat midImage, edgeDetect;
	Mat afterEnhance; 
	vector<Point2f> resultPointsByEdge;
	vector<Vec4i> lines;
	map<double, Vec4i> findCrossPoint(const vector<Vec4i>& lines);	//根据直线提取确定矩形4个顶点
	void edge_corner_candidates(const map<double, Vec4i>&, const vector<Point2f>&); //compute nodes from table.
	vector<Point2f> calculate_final_nodes(vector<Point2f>& line_nodes, vector<Point2f>& show_corner_Mat);
	vector<vector<Point2f>> divide_into_4_parts(vector<Point2f>& line_nodes);
	Point2f mid_process_final_points(vector<Point2f>& line_nodes, vector<Point2f>& corner_nodes);
	Mat preprocess_image();
	vector<Point2f> corner_dectection(Mat);

	void loadImage(const string& name);//加载图像
	void doEdgeDetect(vector<Point2f>&);//边缘提取
	void doAffineTransform(vector<Point2f>&);//透视变换
	void dstImageEnhance();//图强增强
	Debug* debug;
};

PPTRestore::PPTRestore() : pImpl(new Ximpl()){}
PPTRestore::PPTRestore(const PPTRestore& other) : pImpl(new Ximpl(*other.pImpl)){}
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

Mat PPTRestore::Ximpl::preprocess_image()
{
	Mat src = srcImage, src_gray, after_gaus, res;
	string source_window = "高斯模糊-膨胀";

	cvtColor(src, src_gray, CV_BGR2GRAY);
	GaussianBlur(src_gray, after_gaus, Size(9, 9), 0, 0);
	auto kernel = getStructuringElement(MORPH_RECT, Size(25, 25));
	dilate(after_gaus, res, kernel);
	imshow(source_window, res);

	return res;
}

vector<Point2f> PPTRestore::Ximpl::corner_dectection(Mat src)
{
	//初始化 Shi-Tomasi algorithm的一些参数
	vector<Point2f> corners;
	double qualityLevel = 0.01;
	double minDistance = 10;
	int blockSize = 3;
	bool useHarrisDetector = false;
	double k = 0.04;

	//给原图做一次备份
	Mat copy;
	copy = src.clone();

	int maxCorners = 23;
	int maxTrackbar = 100;
	RNG rng(12345);
	string source_window = "角点检查";
	// 角点检测
	goodFeaturesToTrack(src, corners, maxCorners, qualityLevel, minDistance, Mat(), blockSize, useHarrisDetector, k);

	//画出检测到的角点
	cout << "** Number of corners detected: " << corners.size() << endl;
	
	int r = 4;
	for (int i = 0; i < corners.size(); i++)
	{
		circle(src, corners[i], r, Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
			rng.uniform(0, 255)), -1, 8, 0);
	}

	imshow(source_window, src);

	return corners;
}

void PPTRestore::Ximpl::doEdgeDetect(vector<Point2f>& corners)
{
	this->cannyThreshold = 180;
	float factor = 2.5;
	const int maxLinesNum = 10;//最多检测出的直线条数
	Canny(this->srcImage, this->midImage, this->cannyThreshold, this->cannyThreshold * factor);
	threshold(this->midImage, this->midImage, 128, 255, THRESH_BINARY);
	cvtColor(this->midImage, this->edgeDetect, CV_GRAY2RGB);
	HoughLinesP(this->midImage, this->lines, 1, CV_PI / 180, 50, 100, 100);

	while (this->lines.size() >= 30)
	{
		this->cannyThreshold += 2;
		Canny(this->srcImage, this->midImage, this->cannyThreshold, this->cannyThreshold * factor);
		threshold(this->midImage, this->midImage, 128, 255, THRESH_BINARY);
		cvtColor(this->midImage, this->edgeDetect, CV_GRAY2RGB);
		HoughLinesP(this->midImage, this->lines, 1, CV_PI / 180, 50, 100, 100);
	}


	Canny(this->srcImage, this->midImage, this->cannyThreshold, this->cannyThreshold * factor);
	threshold(this->midImage, this->midImage, 128, 255, THRESH_BINARY);
	cvtColor(this->midImage, this->edgeDetect, CV_GRAY2RGB);
	HoughLinesP(this->midImage, this->lines, 1, CV_PI / 180, 50, 100, 100);

	const int imageRow = this->midImage.rows;
	const int imageCol = this->midImage.cols;


	lines.erase(remove_if(lines.begin(), lines.end(), IsCloseToEdge()), lines.end());

	for (size_t i = 0; i < this->lines.size(); i++)
	{
		Vec4i l = this->lines[i];
		line(this->edgeDetect, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(186, 88, 255), 1, CV_AA);
	}


	auto points_with_ratio = this->findCrossPoint(this->lines);
	this->edge_corner_candidates(points_with_ratio, corners);
	imshow("【边缘提取效果图】", this->edgeDetect);
}
map<double, Vec4i> PPTRestore::Ximpl::findCrossPoint(const vector<Vec4i>& lines)//通过直线找点
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
		double ratio = (double) (end_y - start_y) / (double) (end_x - start_x);
		
		table_vec[ratio] = line;
	}

	return table_vec;
}
void PPTRestore::Ximpl::edge_corner_candidates(const map<double, Vec4i>& ratio_2points, const vector<Point2f>& corners)
{
	debug->print(ratio_2points);
	vector<double> intercepts;
	vector<double> ratios;
	vector<Point> points;
	using paii = pair<Point2f, double>;
	struct cmp
	{
		bool operator()(paii p1, paii p2)
		{
			return p1.second > p2.second;
		}
	};
	priority_queue<paii, vector<paii>, cmp> nodes_to_lines;
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
			double dis = (double)abs(ratios[j] * corners[i].x - corners[i].y + intercepts[j]) / (double) sqrt(pow(ratios[j], 2) + 1);
			if (dis < min_dis)
				min_dis = dis;
		}
		nodes_to_lines.push({ corners[i], min_dis });
	}
	int num = 100;
	

	auto ttt = nodes_to_lines;
	while (!ttt.empty())
	{
		auto p = ttt.top().first;
		cout << p.x << "  " << p.y << " " << ttt.top().second << endl;
		ttt.pop();
	}

	vector<Point2f> corner_nodes;

	while (!nodes_to_lines.empty() && num > 0)
	{
		auto p = nodes_to_lines.top().first;
		corner_nodes.emplace_back(Point2f(p.x, p.y ));
		nodes_to_lines.pop();
		num--;
	}
	cout << "uuu ssss : " << corner_nodes.size() << endl;


	Mat show_corner_Mat = this->edgeDetect;
	RNG rng(12345);
	int r = 4;
	for (int i = 0; i < corner_nodes.size(); i++)
	{
		circle(show_corner_Mat, corners[i], r, Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
			rng.uniform(0, 255)), -1, 8, 0);
	}
	string win = "4_nodes";
	//imshow(win, show_corner_Mat);
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
	imshow(win, show_corner_Mat);
	this->resultPointsByEdge = calculate_final_nodes(line_nodes, corner_nodes);
}

vector<Point2f> PPTRestore::Ximpl::calculate_final_nodes(vector<Point2f>& line_nodes, vector<Point2f>& corner_nodes)
{
	vector<vector<Point2f>> line_temp = divide_into_4_parts(line_nodes);
	vector<vector<Point2f>> corner_temp = divide_into_4_parts(corner_nodes);
	cout << "line_temp.size()" << line_temp.size() << endl;
	cout << "corner_temp.size()" << corner_temp.size() << endl;
	vector<Point2f> left_top_line_nodes = line_temp[0], left_down_line_nodes = line_temp[1], right_top_line_nodes = line_temp[2], right_down_line_nodes = line_temp[3], res;
	vector<Point2f> left_top_corner_nodes = corner_temp[0], left_down_corner_nodes = corner_temp[1], right_top_corner_nodes = corner_temp[2], right_down_corner_nodes = corner_temp[3];
	
	res.emplace_back(mid_process_final_points(left_top_line_nodes, left_top_corner_nodes));
	res.emplace_back(mid_process_final_points(right_top_line_nodes, right_top_corner_nodes));
	res.emplace_back(mid_process_final_points(left_down_line_nodes, left_down_corner_nodes));
	res.emplace_back(mid_process_final_points(right_down_line_nodes, right_down_corner_nodes));

	return res;
}

Point2f PPTRestore::Ximpl::mid_process_final_points(vector<Point2f>& line_nodes, vector<Point2f>& corner_nodes)
{
	if (line_nodes.size() == 1) return line_nodes[0];
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
	return line_nodes[position];
}

vector<vector<Point2f>> PPTRestore::Ximpl::divide_into_4_parts(vector<Point2f>& line_nodes)
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


//通过轮廓提取得到四个顶点
void PPTRestore::Ximpl::loadImage(const string& name)
{
	srcImage = imread(name, 1);
	if (!srcImage.data)
	{
		cout << "读取图片错误，请确定目录下是否有imread函数指定的图片存在" << endl;
	}
	imshow(WINDOW_NAME1, this->srcImage);
}
void PPTRestore::Ximpl::doAffineTransform(vector<Point2f>& corners)
{
	this->doEdgeDetect(corners);//仿射变换前先边缘、直线提取

	Point2f _srcTriangle[4];
	Point2f _dstTriangle[4];
	vector<Point2f>srcTriangle(_srcTriangle, _srcTriangle + 4);
	vector<Point2f>dstTriangle(_dstTriangle, _dstTriangle + 4);
	
	auto four_points = this->resultPointsByEdge;
	const int leftTopX = four_points[0].x;
	const int leftTopY = four_points[0].y;
	const int rightTopX = four_points[1].x;
	const int rightTopY = four_points[1].y;
	const int leftDownX = four_points[2].x;
	const int leftDownY = four_points[2].y;
	const int rightDownX = four_points[3].x;
	const int rightDownY = four_points[3].y;

	cout << "1111111111111111111" << endl;
	cout << leftTopX << " " << leftTopY << endl;
	cout << rightTopX << " " << rightTopY << endl;
	cout << leftDownX << " " << leftDownY << endl;
	cout << rightDownX << " " << rightDownY << endl;


	int newWidth = 0;
	int newHeight = 0;

	newWidth = sqrt((leftTopX - rightTopX) * (leftTopX - rightTopX) + (leftTopY - rightTopY) * (leftTopY - rightTopY));
	newHeight = sqrt((leftTopX - leftDownX) * (leftTopX - leftDownX) + (leftTopY - leftDownY) * (leftTopY - leftDownY));

	this->dstImage = Mat::zeros(newHeight, newWidth, this->srcImage.type());

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
	warpPerspective(this->srcImage, this->dstImage, h, this->dstImage.size());
	imshow(WINDOW_NAME2, this->dstImage);
}
void PPTRestore::Ximpl::dstImageEnhance()
{
	Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
	//Mat kernel = (Mat_<float>(5, 5) << 0, -1, 0, -1,0,-1,0,-1,0,-1,0,-1,14,-1,0, -1, 0, -1, 0,-1,0,-1,0,-1,0);
	/*Mat kernel(3, 3, CV_32F, Scalar(-1)); 
	kernel.at<float>(1, 1) =9;*/
	//filter2D(this->dstImage, this->afterEnhance, this->dstImage.depth(), kernel);
	filter2D(this->srcImage, this->afterEnhance, this->srcImage.depth(), kernel);
	//imshow(WINDOW_NAME3, this->afterEnhance);
	imwrite("result.jpg", afterEnhance);
}
void PPTRestore::imageRestoreAndEnhance(const string name)
{
	//执行顺序：加载原图、修正、增强
	this->pImpl->loadImage(name);
	auto after_preprocess = this->pImpl->preprocess_image();
	auto corners = this->pImpl->corner_dectection(after_preprocess);
	this->pImpl->doAffineTransform(corners);
	this->pImpl->dstImageEnhance();
}