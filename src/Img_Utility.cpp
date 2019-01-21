#include "Img_Utility.h"

double Utility::angle(Point2f pt1, Point2f pt2, Point2f pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

double Utility::pointDistance(const Point2f& p1, const Point2f& p2)
{
	return pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2);
}

double Utility::pointDistance(const Vec4f& line)
{
	return pow(line[0] - line[2], 2) + pow(line[1] - line[3], 2);
}


bool Utility::is_similar_line(const Vec4i& _l1, const Vec4i& _l2)
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