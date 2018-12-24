#include "PPTRestoreClassHead.h"


int main()
{
	const string testName = "5.jpg";
	PPTRestore ppt;
	ppt.imageRestoreAndEnhance(testName);
	waitKey();
	return 0;
}
