
#include "PPTRestoreClassHead.h"
#include <numeric>
struct PPTRestore::Ximpl
{
	enum imageStyle { normal, leanToRight, leanToLeft };
	int cannyThreshold;
	Mat srcImage, dstImage;
	Mat midImage, edgeDetect;
	Mat afterEnhance;
	vector<Point> resultPointsByEdge;
	vector<Point> resultPointsByContours;
	vector<Vec4i> lines;

	vector<Point> axisSort(const vector<Vec4i>& lines);
	vector<Point> pointsFilter(const vector<Point>& candidates);//对各线段起始点过滤
	vector<Point> findCrossPoint(const vector<Vec4i>& lines);	//根据直线提取确定矩形4个顶点

	vector<Point> axisSort(const vector<vector<Point>>& contours);
	vector<Point> findCrossPoint(const vector<vector<Point>>& contours);//根据轮廓提取4个顶点

	void loadImage(const string& name);//加载图像
	void doEdgeDetect();//边缘提取
	void doFindcontours();//轮廓提取
	void doAffineTransform();//透视变换
	void dstImageEnhance();//图强增强
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

//通过(设置上限的)边缘提取得到四个顶点
vector<Point> PPTRestore::Ximpl::pointsFilter(const vector<Point>& candidate)
{
	vector<Point> candidates(candidate);
	vector<Point> filter(candidate);
	for (auto i = candidates.begin(); i != candidates.end();)
	for (auto j = filter.begin(); j != filter.end(); ++j)
	{
		if (abs((*i).x - (*j).x) < 5 && abs((*i).y - (*j).y) < 5 && abs((*i).x - (*j).x) > 0 && abs((*i).y - (*j).y) > 0)
			i = filter.erase(i);
		else
			++i;
	}
	return filter;
}
vector<Point> PPTRestore::Ximpl::axisSort(const vector<Vec4i>& lines)
{
	vector<Point> points(lines.size() * 2);//各个线段的起始点
	for (size_t i = 0; i < lines.size(); ++i)//将Vec4i转为point
	{
		points[i * 2].x = lines[i][0];
		points[i * 2].y = lines[i][1];
		points[i * 2 + 1].x = lines[i][2];
		points[i * 2 + 1].y = lines[i][3];
	}
	points = this->pointsFilter(points);//对自己过滤一次
	/*for (auto i : points)
	cout << i.x << " " << i.y << endl;*/
	sort(points.begin(), points.end(), CmpDistanceToZero());
	return points;
}
void PPTRestore::Ximpl::doEdgeDetect()
{
	this->cannyThreshold = 80;
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
	cout << "cannyThreshold1:" << this->cannyThreshold << endl;

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


	this->findCrossPoint(this->lines);
	/*for (size_t i = 0; i < lines.size(); ++i)
	cout << lines[i] << endl;*/
	imshow("【边缘提取效果图】", this->edgeDetect);
}
vector<Point> PPTRestore::Ximpl::findCrossPoint(const vector<Vec4i>& lines)//通过直线找点
{
	int rightTopFlag = 0;
	int leftDownFlag = 0;
	int diagLength = 0;//对角线长度
	vector<Point> temp = this->axisSort(lines);
	Point leftTop, rightDown;//左上和右下可以直接判断
	vector<Point> rightTop(temp.size());
	vector<Point> leftDown(temp.size());//左下和右上有多个点可能符合
	//对PPT照片而言，一定是左上角离原点最近，右下角离原点最远
	leftTop.x = temp[0].x;
	leftTop.y = temp[0].y;
	rightDown.x = temp[temp.size() - 1].x;
	rightDown.y = temp[temp.size() - 1].y;
	for (auto & i : temp)
	if (i.x > leftTop.x && i.y < rightDown.y)
		rightTop.push_back(i);
	for (auto & i : temp)
	if (i.y > leftTop.y && i.x < rightDown.x)
		leftDown.push_back(i);

	diagLength = (leftTop.x - rightDown.x) * (leftTop.x - rightDown.x) + (leftTop.y - rightDown.y) * (leftTop.y - rightDown.y);
	rightTop.erase(remove(rightTop.begin(), rightTop.end(), Point(0, 0)), rightTop.end());
	leftDown.erase(remove(leftDown.begin(), leftDown.end(), Point(0, 0)), leftDown.end());
	//删除因图像畸变对计算两个距离最长点对的影响的点
	for (auto i = rightTop.begin(); i != rightTop.end();)
	{
		if (((*i).x - leftTop.x) * ((*i).x - leftTop.x) + ((*i).y - leftTop.y) * ((*i).y - leftTop.y) < diagLength / 8)
			i = rightTop.erase(i);
		else
			++i;
	}

	/*for (auto i : rightTop)
	cout << "右上还有：" << i.x << " " << i.y << endl;*/

	int maxDistance = (rightTop[0].x - leftDown[0].x) * (rightTop[0].x - leftDown[0].x) + (rightTop[0].y - leftDown[0].y) * (rightTop[0].y - leftDown[0].y);

	for (size_t i = 0; i < rightTop.size(); ++i)
	for (size_t j = 0; j < leftDown.size(); ++j)
	if ((rightTop[i].x - leftDown[j].x) * (rightTop[i].x - leftDown[j].x) + (rightTop[i].y - leftDown[j].y) * (rightTop[i].y - leftDown[j].y) > maxDistance)
	{
		maxDistance = (rightTop[i].x - leftDown[j].x) * (rightTop[i].x - leftDown[j].x) + (rightTop[i].y - leftDown[j].y) * (rightTop[i].y - leftDown[j].y);
		rightTopFlag = i;
		leftDownFlag = j;
	}

	/*cout << rightTop[rightTopFlag].x << " " << rightTop[rightTopFlag].y << endl;
	cout << leftDown[leftDownFlag].x << " " << leftDown[leftDownFlag].y << endl;*/

	/*for (auto i : rightTop)
	cout << i.x << " " << i.y << endl;*/

	/*for (auto i : leftdown)
	cout << i.x << " " << i.y << endl;*/

	this->resultPointsByEdge.push_back(leftTop);
	this->resultPointsByEdge.push_back(rightTop[rightTopFlag]);
	this->resultPointsByEdge.push_back(leftDown[leftDownFlag]);
	this->resultPointsByEdge.push_back(rightDown);
	return this->resultPointsByEdge;
}


//通过轮廓提取得到四个顶点
vector<Point> PPTRestore::Ximpl::axisSort(const vector<vector<Point>>& contours)
{
	vector<Point> points(contours.size() * contours[0].size());
	for (auto i : contours)
	for (auto j : i)
		points.push_back(j);
	points = this->pointsFilter(points);//对自己过滤一次
	points.erase(remove(points.begin(), points.end(), Point(0, 0)), points.end());
	sort(points.begin(), points.end(), CmpDistanceToZero());
	return points;
}
void PPTRestore::Ximpl::doFindcontours()
{
	const float approachMaxThreshold = 20;
	Mat result(this->midImage.size(), CV_8U, Scalar(0));
	vector<vector<Point>> contours;
	findContours(this->midImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	sort(contours.begin(), contours.end(), CmpContoursSize());
	for (size_t i = 0; i < 5; ++i)//去除贴近图像边缘的轮廓
	{
		size_t j = contours[i].size();
		if (contours[i][j / 5].y - 0 < approachMaxThreshold || this->midImage.cols - contours[i][j / 2].x < approachMaxThreshold)
			contours.erase(remove(contours.begin(), contours.end(), contours[i]), contours.end());
	}
	vector<vector<Point>> biggestContours;
	for (size_t i = 0; i < 3; ++i)
		biggestContours.push_back(contours[i]);
	drawContours(result, biggestContours, -1, Scalar(255), 2);
	imshow("轮廓提取", result);
	this->findCrossPoint(biggestContours);//通过大轮廓找到交点
	/*this->candidatePoint.push_back(Point(250, 0));
	this->candidatePoint.push_back(Point(512, 159));
	this->candidatePoint.push_back(Point(186, 736));
	this->candidatePoint.push_back(Point(475, 637));*/
}
vector<Point> PPTRestore::Ximpl::findCrossPoint(const vector<vector<Point>>& contours)//通过轮廓找点
{
	int imageState;//图片如何倾斜
	vector<Point> temp = this->axisSort(contours);
	Point leftTop, trueRightTop, trueLeftDown, rightDown;//左上和右下可以直接判断
	vector<Point> rightTop(temp.size());
	vector<Point> leftDown(temp.size());//左下和右上有多个点可能符合
	//对PPT照片而言，一定是左上角离原点最近，右下角离原点最远
	leftTop.x = temp[0].x;
	leftTop.y = temp[0].y;
	rightDown.x = temp[temp.size() - 1].x;
	rightDown.y = temp[temp.size() - 1].y;

	for (auto & i : temp)
	if (i.x > leftTop.x && i.y < rightDown.y)
		rightTop.push_back(i);
	for (auto & i : temp)
	if (i.y > leftTop.y && i.x < rightDown.x)
		leftDown.push_back(i);

	if (rightTop.end() == find_if(rightTop.begin(), rightTop.end(), [leftTop, rightTop](Point p){return p.y < leftTop.y; }))
		imageState = imageStyle::leanToRight;//如果所有右上点的y值都 > 左上点的y值 ，说明图像向右倾斜
	else
		imageState = imageStyle::leanToLeft;

	if (imageState == imageStyle::leanToRight)//向右倾斜
	{
		sort(rightTop.begin(), rightTop.end(), [rightTop](Point p1, Point p2){return p1.x > p2.x; });//对所有右上点按X值排序，X最大的就是真正的右上点
		rightTop.erase(remove(rightTop.begin(), rightTop.end(), Point(0, 0)), rightTop.end());
		trueRightTop = rightTop[0];
		sort(leftDown.begin(), leftDown.end(), [leftDown](Point p1, Point p2){return p1.x < p2.x; });//对所有左下点按X值排序，X最小的就是真正的左下点
		leftDown.erase(remove(leftDown.begin(), leftDown.end(), Point(0, 0)), leftDown.end());
		trueLeftDown = leftDown[0];
	}
	else //向左倾斜
	{
		sort(rightTop.begin(), rightTop.end(), [rightTop](Point p1, Point p2){return p1.y < p2.y; });//对所有右上点按Y值排序，Y最小的就是真正的右上点
		rightTop.erase(remove(rightTop.begin(), rightTop.end(), Point(0, 0)), rightTop.end());
		trueRightTop = rightTop[0];
		sort(leftDown.begin(), leftDown.end(), [leftDown](Point p1, Point p2){return p1.y > p2.y; });//对所有左下点按Y值排序，Y最大的就是真正的左下点
		leftDown.erase(remove(leftDown.begin(), leftDown.end(), Point(0, 0)), leftDown.end());
		trueLeftDown = leftDown[0];
	}

	//ofstream fout("result.txt");
	//for (auto i : leftDown)
	//	fout << "左下还有：" << i.x << " " << i.y << endl;

	//cout << "右上点：" << trueRightTop << endl
	//	<< "左下点：" << trueLeftDown << endl;


	this->resultPointsByContours.push_back(leftTop);
	this->resultPointsByContours.push_back(trueRightTop);
	this->resultPointsByContours.push_back(trueLeftDown);
	this->resultPointsByContours.push_back(rightDown);
	return this->resultPointsByContours;
}

void PPTRestore::Ximpl::loadImage(const string& name)
{
	srcImage = imread(name, 1);
	if (!srcImage.data)
	{
		cout << "读取图片错误，请确定目录下是否有imread函数指定的图片存在" << endl;
	}
	imshow(WINDOW_NAME1, this->srcImage);
}
void PPTRestore::Ximpl::doAffineTransform()
{
	this->doEdgeDetect();//仿射变换前先边缘、直线提取
	//this->doFindcontours();//提取轮廓
	Point2f _srcTriangle[4];
	Point2f _dstTriangle[4];
	vector<Point2f>srcTriangle(_srcTriangle, _srcTriangle + 4);
	vector<Point2f>dstTriangle(_dstTriangle, _dstTriangle + 4);

	const int leftTopX = (this->resultPointsByEdge[0].x + this->resultPointsByEdge[0].x) / 2;
	const int leftTopY = (this->resultPointsByEdge[0].y + this->resultPointsByEdge[0].y) / 2;
	const int rightTopX = (this->resultPointsByEdge[1].x + this->resultPointsByEdge[1].x) / 2;
	const int rightTopY = (this->resultPointsByEdge[1].y + this->resultPointsByEdge[1].y) / 2;
	const int leftDownX = (this->resultPointsByEdge[2].x + this->resultPointsByEdge[2].x) / 2;
	const int leftDownY = (this->resultPointsByEdge[2].y + this->resultPointsByEdge[2].y) / 2;
	const int rightDownX = (this->resultPointsByEdge[3].x + this->resultPointsByEdge[3].x) / 2;
	const int rightDownY = (this->resultPointsByEdge[3].y + this->resultPointsByEdge[3].y) / 2;

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
	this->pImpl->doAffineTransform();
	this->pImpl->dstImageEnhance();
}