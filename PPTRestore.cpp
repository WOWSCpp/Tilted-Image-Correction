#include "PPTRestoreClassHead.h"
int main()
{
	const string testName = "1.jpg";
	PPTRestore ppt;
	//ppt.public_api(testName);
	Mat src = imread(testName, 1);
	ppt.get_points(src);
	waitKey();
	return 0;
}