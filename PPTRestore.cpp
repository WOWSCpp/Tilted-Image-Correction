#include "PPTRestoreClassHead.h"
int main()
{
	const string testName = "1.jpg";
	PPTRestore ppt;
	cout << INT_MIN * -1 << endl;
	//ppt.public_api(testName);
	Mat src = imread(testName, 1);
	ppt.get_points(src);
	auto r = ppt.get_all_images();
	for (auto p : r) cout << p.first << endl;
	waitKey();
	return 0;
}