
#include "Img_Process.h"

struct ImageProcess::Ximpl
{
	Mat srcImage;
	Mat afterCanny;
	vector<Point2f> hullPoints;
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
	Mat perspective_transformation(const vector<Point2f>&, Mat&);
	Mat image_enhance(Mat&);
	Debug* debug;
	Utility* utility;
	PreProcess *pre;
};

ImageProcess::ImageProcess() : pImpl(new Ximpl()) {}

ImageProcess::ImageProcess(const ImageProcess& other) : pImpl(new Ximpl(*other.pImpl)) {}

ImageProcess& ImageProcess::operator=(ImageProcess other)
{
	std::swap(other.pImpl, this->pImpl);
	return *this;
}

ImageProcess::~ImageProcess()
{
	delete pImpl;
	pImpl = nullptr;
}


Mat ImageProcess::Ximpl::preprocess_image(Mat& img)
{
	Size size(5, 5);
	GaussianBlur(img, img, size, 0);
	auto contours = pre->m_find_contours(img);
	if (contours.empty()) return img;
	auto biggestContours = pre->m_find_biggest_contours(contours);
	if (biggestContours.empty()) return img;

	Mat tmp = img.clone();
	pre->render_to_white(tmp);
	auto minRect = pre->m_find_minRect(biggestContours);
	Mat drawing = Mat::zeros(tmp.size(), CV_8UC3);
	pre->show_biggest_contours_and_center(biggestContours, minRect, drawing);

	auto centerRectContours = pre->m_find_centerRect_contours(biggestContours, minRect, tmp);
	pre->render_center_rect_contours(tmp, centerRectContours);
	//imshow("tmp", tmp);

	hullPoints = pre->get_hull_points(contours, img);
	if (hullPoints.empty()) return img;
	return tmp;
}

vector<Point2f> ImageProcess::Ximpl::corner_dectection(Mat& src)
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

	//debug->show_img(source_window, tmp);
	return corners;
}



vector<Vec4f> ImageProcess::Ximpl::edge_detection(Mat& src)
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
	vector<Vec4f> lines;
	src = dst;
	Mat gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	pair<double, double> p = pre->auto_canny(gray);
	double lower = p.first, upper = p.second;

	Mat mid, edgeDetect;
	Canny(src, mid, lower, upper, 3);
	//imshow("bbbbbb", mid);
	//threshold(mid, mid, 128, 255, THRESH_BINARY);
	cvtColor(mid, edgeDetect, CV_GRAY2RGB);

	int min_line_length = 50;
	int max_line_gap = 100;
	HoughLinesP(mid,
		lines,
		1,
		CV_PI / 180,
		10,
		min_line_length,
		max_line_gap
	);
	cout << "lines.size()" << lines.size() << endl;

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
	if (lines.empty()) return{};

	std::vector<int> labels;
	int numberOfLines = cv::partition(lines, labels, Utility::is_similar_line);

	vector<Vec4f> final_lines;
	int max_size = *max_element(labels.begin(), labels.end());
	for (int i = 0; i < max_size + 1; ++i)
		final_lines.emplace_back(lines[find(labels.begin(), labels.end(), i) - labels.begin()]);

	sort(final_lines.begin(), final_lines.end(), [&](Vec4f v1, Vec4f v2) {
		return utility->pointDistance(v1) < utility->pointDistance(v2); });
	final_lines.erase(remove_if(final_lines.begin(), final_lines.end(), [&](Vec4f v1)
	{
		auto v2 = final_lines.back();
		return utility->pointDistance(v1) < 0.2 * utility->pointDistance(v2);
	}), final_lines.end());


	for (size_t i = 0; i < final_lines.size(); i++)
	{
		Vec4i l = final_lines[i];
		line(edgeDetect, Point2i(l[0], l[1]), Point2i(l[2], l[3]), Scalar(0, 0, 255), 1, CV_AA);
	}

	//debug->show_img("edgeDetect", edgeDetect);
	return final_lines;
}


Point2f ImageProcess::Ximpl::line_intersection(const Point2f& o1, const Point2f& p1, const Point2f& o2, const Point2f& p2)
{
	Point2f x = o2 - o1;
	Point2f d1 = p1 - o1;
	Point2f d2 = p2 - o2;

	float cross = d1.x*d2.y - d1.y*d2.x;
	double t1 = (x.x * d2.y - x.y * d2.x) / cross;
	return o1 + d1 * t1;
}

vector<Point2f> ImageProcess::Ximpl::cal_points_with_lines(const vector<Vec4f>& lines)
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

	vector<Point2f> cross_points(hullPoints);
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

	// add all line's edge Point2i into cross_points
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
	//imshow("aaaaaaaaaaa", t);

	vector<vector<Point2f>> _4_parts = divide_points_into_4_parts(cross_points);
	vector<Point2f> a = _4_parts[0];
	vector<Point2f> b = _4_parts[1];
	vector<Point2f> c = _4_parts[2];
	vector<Point2f> d = _4_parts[3];

	//sort(a.begin(), a.end(), [&](Point2f p1, Point2f p2) {
	//	return pow(center.y * 0 - p1.y, 2) + pow(center.x * 0 - p1.x, 2) < pow(center.y * 0 - p2.y, 2) + pow(center.x * 0 - p2.x, 2); });
	//sort(b.begin(), b.end(), [&](Point2f p1, Point2f p2) {
	//	return pow(center.y * 0 - p1.y, 2) + pow(center.x * 2 - p1.x, 2) < pow(center.y * 0 - p2.y, 2) + pow(center.x * 2 - p2.x, 2); });
	//sort(c.begin(), c.end(), [&](Point2f p1, Point2f p2) {
	//	return pow(center.y * 2 - p1.y, 2) + pow(center.x * 0 - p1.x, 2) < pow(center.y * 2 - p2.y, 2) + pow(center.x * 0 - p2.x, 2); });
	//sort(d.begin(), d.end(), [&](Point2f p1, Point2f p2)  {
	//	return pow(center.y * 2 - p1.y, 2) + pow(center.x * 2 - p1.x, 2) < pow(center.y * 2 - p2.y, 2) + pow(center.x * 2 - p2.x, 2); });

	sort(a.begin(), a.end(), [&](Point2f p1, Point2f p2) {
		return utility->pointDistance(center, p1) < utility->pointDistance(center, p2);
	});

	sort(b.begin(), b.end(), [&](Point2f p1, Point2f p2) {
		return utility->pointDistance(center, p1) < utility->pointDistance(center, p2);
	});
	sort(c.begin(), c.end(), [&](Point2f p1, Point2f p2) {
		return utility->pointDistance(center, p1) < utility->pointDistance(center, p2);
	});
	sort(d.begin(), d.end(), [&](Point2f p1, Point2f p2) {
		return utility->pointDistance(center, p1) < utility->pointDistance(center, p2);
	});


	intersect_points.emplace_back(a.empty() ? Point2f(20, 20) : a.back());
	intersect_points.emplace_back(c.empty() ? Point2f(srcImage.cols - 20, 20) : c.back());
	intersect_points.emplace_back(b.empty() ? Point2f(20, srcImage.rows - 20) : b.back());
	intersect_points.emplace_back(d.empty() ? Point2f(srcImage.cols - 20, srcImage.rows - 20) : d.back());
	return intersect_points;
}


map<float, Vec4f> ImageProcess::Ximpl::find_cross_points_by_edges(const vector<Vec4f>& lines)
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

vector<Point2f> ImageProcess::Ximpl::edge_corner_candidates(const map<float, Vec4f>& ratio_2points, const vector<Point2f>& corners)
{
	vector<float> intercepts;
	vector<float> ratios;
	vector<Point2i> points;
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

vector<Point2f> ImageProcess::Ximpl::cal_final_points(const vector<Point2f>& line_nodes, const vector<Point2f>& corner_nodes)
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

Point2f& ImageProcess::Ximpl::find_closest_points(const vector<Point2f>& line_nodes, const vector<Point2f>& corner_nodes)
{

	if (line_nodes.size() == 1) return const_cast<Point2f&>(line_nodes[0]);
	float min_dis = DBL_MAX;
	int position = 0;
	for (int i = 0; i < line_nodes.size(); ++i)
	{
		for (int j = 0; j < corner_nodes.size(); ++j)
		{
			if (utility->pointDistance(line_nodes[i], corner_nodes[j]) < min_dis)
			{
				min_dis = utility->pointDistance(line_nodes[i], corner_nodes[j]);
				position = i;
			}
		}
	}
	return const_cast<Point2f&>(line_nodes[position]);
}

vector<vector<Point2f>> ImageProcess::Ximpl::divide_points_into_4_parts(const vector<Point2f>& line_nodes)
{
	vector<Point2f> left_top_line_nodes, left_down_line_nodes, right_top_line_nodes, right_down_line_nodes;
	vector<vector<Point2f>> res;
	int height = this->srcImage.cols / 2, width = this->srcImage.rows / 2;
	float p1 = 1, p2 = 1; //tiao can
	for (auto node : line_nodes)
	{
		if (node.x < height * p1)
		{
			if (node.y < width * p1)
				left_top_line_nodes.emplace_back(node);
			if (node.y > width * p2)
				left_down_line_nodes.emplace_back(node);
		}
		if (node.x > height * p2)
		{
			if (node.y < width * p1)
				right_top_line_nodes.emplace_back(node);
			if (node.y > width * p2)
				right_down_line_nodes.emplace_back(node);
		}
	}
	res.emplace_back(left_top_line_nodes);
	res.emplace_back(left_down_line_nodes);
	res.emplace_back(right_top_line_nodes);
	res.emplace_back(right_down_line_nodes);
	return res;
}

Mat ImageProcess::Ximpl::perspective_transformation(const vector<Point2f>& final_points, Mat& src)
{
	//debug->print(final_points);
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
	cout << newWidth << " " << newHeight << endl;
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
	//debug->show_img("after_transform", after_transform);
	return after_transform;
}

Mat ImageProcess::Ximpl::image_enhance(Mat& input)
{
	Mat output;
	Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
	filter2D(input, output, this->srcImage.depth(), kernel);
	return output;
}

vector<Point2f> ImageProcess::get_points(Mat& image)
{

	if (!pImpl->hullPoints.empty())
		pImpl->hullPoints.clear();
	imshow("raw image", image);
	//resize(image, image, Size(image.cols / 2, image.rows / 2));
	pImpl->srcImage = image.clone();


	auto after_preprocess = pImpl->preprocess_image(image.clone());

	//auto corners = this->pImpl->corner_dectection(after_preprocess);

	auto lines = pImpl->edge_detection(after_preprocess);

	if (lines.empty())return{ Point2f(20, 20), Point2f(image.cols - 20 , 20), Point2f(20, image.rows - 20), Point2f(image.cols - 20, image.rows - 20) };

	auto final_points_new = pImpl->cal_points_with_lines(lines);
	if (PLATFORM != "WIN32")
		for (auto& p : final_points_new) { p.x *= 2; p.y *= 2; }
	// auto points_with_ratio = pImpl->find_cross_points_by_edges(lines);
	pImpl->debug->print(final_points_new);
	// auto final_points = this->pImpl->edge_corner_candidates(points_with_ratio, corners);
	get_image(image, final_points_new);
	return final_points_new;
}

Mat ImageProcess::get_image(Mat& image, vector<Point2f>& points)
{
	sort(points.begin(), points.end(), [](Point2f p1, Point2f p2) {return p1.y < p2.y; });
	sort(points.begin(), points.begin() + 2, [](Point2f p1, Point2f p2) {return p1.x < p2.x; });
	sort(points.begin() + 2, points.begin() + 4, [](Point2f p1, Point2f p2) {return p1.x < p2.x; });
	auto after_transform = pImpl->perspective_transformation(points, image);
	auto final_mat = pImpl->image_enhance(after_transform);
	imshow("get_from_trans", final_mat);
	return final_mat;
}

