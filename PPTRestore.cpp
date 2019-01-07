#include "PPTRestoreClassHead.h"
int main()
{
	//const string testName = "ppt2.jpg";
	const string testName = "test_sets/g51.jpg";
	PPTRestore ppt;
	Mat src = imread(testName, 1);
	ppt.get_points(src);
	waitKey();
	return 0;
}