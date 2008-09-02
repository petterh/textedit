#include "precomp.h"
#include "..\fileUtils.h"
#include "CppUnitLite\CppUnitLite.h"

void test();

TEST(isPathSeparator, fileUtils) {

	CHECK(isPathSeparator(_T('\\')));
	CHECK(isPathSeparator(_T('/')));

	CHECK(!isPathSeparator(_T(' ')));
	CHECK(!isPathSeparator(_T('X')));
}

TEST(getDefaultExtension, fileUtils) {

	CHECK_EQUAL(_T(".txt"), getDefaultExtension());
}

TEST(areFileNamesEqual, fileUtils) {

	// Fails because files don't exist:
	CHECK(!areFileNamesEqual(_T("c:\\foo.txt"), _T("C:\\FOO.TXT")));
}
