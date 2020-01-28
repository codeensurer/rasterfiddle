// WKT2WKB.cpp : Defines the entry point for the console application.
//

#include <stdafx.h>
#include <WkbTester.h>

void test()
{
	WkbTester * tester = new WkbTester();
	tester->runTests();
}

int main()
{
	test();
	return 0;
}

