
#ifdef LAUNCH_GTEST

#include <stdio.h>

#include "gtest/gtest.h"


GTEST_API_ int main(int argc, char **argv) {
	printf("Running main() from gtest_main.cc\n");
	testing::InitGoogleTest(&argc, argv);
	int n= RUN_ALL_TESTS();
	system("pause");
	return n;
}

#endif