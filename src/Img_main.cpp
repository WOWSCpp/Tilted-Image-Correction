#include "Img_Process.h"
int main()
{
	const string testName = "test_sets/";
	vector<string> a{"1.jpg", "2.jpg","3.jpg","4.jpg","5.jpg","6.jpg","7.jpg" };
	ImageProcess ppt;
	for (auto e : a)
	{
		Mat src = imread(testName + e, 1);
		ppt.get_points(src);
		waitKey();
	}
	return 0;
}



