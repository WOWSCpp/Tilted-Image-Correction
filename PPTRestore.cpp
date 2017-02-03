#include "PPTRestoreClassHead.h"
int main()
{
	const string testName = "ppt1.jpg";
	PPTRestore ppt;
	ppt.imageRestoreAndEnhance(testName);
	waitKey();
	return 0;
}